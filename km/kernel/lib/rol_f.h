#ifndef ROLF
#define ROLF

#include "rol.h"
#include "configuration.h"
#include "util_device.h"
#include "hash.h"
#include "atomic.h"
#include "reduce.h"

#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
//#pragma OPENCL EXTENSION cl_amd_printf : enable

void linit(__local Reduction_Object_S *object)
{
	const unsigned int local_id = get_local_id(0);
	const unsigned int local_size = get_local_size(0);
	for(short index = local_id; index < NUM_BUCKETS_S; index += local_size)
	{
		(object->buckets)[index].x = 0;//x,y initialization is necessary?
		(object->buckets)[index].y = 0;
		(object->locks)[index] = 0;
	}

	if(local_id == 0)
	{
		object->num_buckets = NUM_BUCKETS_S;
		object->memory_offset = 0;
	}
}

unsigned short lmalloc(__local Reduction_Object_S *object, unsigned short size)
{
	unsigned short align_size = (align(size))>>2;//align(size)/ALIGN_SIZE;
	unsigned short offset        = atomic_add(&(object->memory_offset), align_size);
//	if(offset + size > MAX_POOL_S)
//		return -1;
//	else
           return offset;
}

__local void * lget_address(__local Reduction_Object_S *object, unsigned memory_index)
{
	return object->memory_pool + memory_index;
}

__local void * lget_key_address(__local Reduction_Object_S *object, unsigned bucket_index)
{
	unsigned short key_index = ((object->buckets)[bucket_index]).x;
	__local char *size_address = lget_address(object, key_index);
	return size_address+4;   //original 2
}

unsigned short lget_key_size(__local Reduction_Object_S *object, unsigned bucket_index)
{
	unsigned short key_index = ((object->buckets)[bucket_index]).x;
	__local char *size_address = lget_address(object, key_index);
	return *size_address;    
}

__local void * lget_value_address(__local Reduction_Object_S *object, unsigned bucket_index)
{
	unsigned short value_index =  ((object->buckets)[bucket_index]).y;
	return lget_address(object, value_index);
}

unsigned short lget_value_size(__local Reduction_Object_S *object, unsigned bucket_index)
{
	unsigned short key_index = ((object->buckets)[bucket_index]).x;
	__local char *size_address = lget_address(object, key_index);
	return *(size_address + 1);   
}

bool linsert(__local Reduction_Object_S *object, void *key, unsigned short key_size, void *value, unsigned short value_size)
{
	unsigned int h = hash(key, key_size);
	unsigned int index = h%NUM_BUCKETS_G;
	unsigned int finish = 0;

	int DoWork = true;
	bool ret = true;
	int stride = 1;

	while(!finish)
	{
		if(object->num_buckets==0)
			return false;

		DoWork = 1;

		while(DoWork)
		{
			//printf("Doing work...\n");
			if(local_getlock(&((object->locks)[index])))
			{
				if(((object->buckets)[index]).x==0 && ((object->buckets)[index]).y==0)
				{
					int k = lmalloc(object, 2 + key_size);
					if(k == -1)
						ret = false;

					int v = lmalloc(object, value_size);
					if(v == -1)
						ret = false;

					if(ret == false)
					{
						local_releaselock(&(object->locks)[index]);
						finish  = 1;
						break;
					}

					((object->buckets)[index]).x = k;
					((object->buckets)[index]).y = v;

					__local char *key_size_address = (__local char *)lget_address(object, k);
					__local char *value_size_address = key_size_address + 1;
					*key_size_address = key_size;
					*value_size_address = value_size;

					//store the key data and value data into the ro
					__local void *key_data_start = key_size_address + 2;
					__local void *value_data_start = lget_address(object, v);

					copyVal_private_to_local(key_data_start, key, key_size);
					copyVal_private_to_local(value_data_start, value, value_size);
					//(object->buckets)[index] = kvn;
					atomic_add(&(object->num_buckets), -1);

					local_releaselock(&((object->locks)[index]));
					finish = 1;
					DoWork = false;
				}

				/*compare them*/
				else
				{
					//printf("no **********************\n");
					unsigned short size = lget_key_size(object, index);
					__local void *key_data = lget_key_address(object, index);

					/*if equal, conduct reduce*/
					if(equal_private_and_local(key, key_size, key_data, size))
					{
						unsigned short value_ro_size = lget_value_size(object, index);
						__local void *value_ro_data = lget_value_address(object, index);
						reduce_private_to_local(value_ro_data, value_ro_size, value, value_size);
						DoWork = false;
						finish = 1;
						ret = true;
						local_releaselock(&((object->locks)[index]));
					}

					/*else, compute a new index and continue*/
					else
					{
						DoWork = false;
						//finish = true;
						local_releaselock(&((object->locks)[index]));
						index = (index + stride)%NUM_BUCKETS_G;
					}
				}
			}
		}
	}
	return ret;
}

bool linsert_from_gp(__local Reduction_Object_S *object, __global void *key, unsigned char key_size, void *value, unsigned char value_size)
{
	unsigned int h = hash_global(key, key_size);
	unsigned int index = h&(NUM_BUCKETS_S-1);
	unsigned int finish = 0;
	//unsigned int kvn = 0;
	unsigned int index_base;
	
	bool DoWork = true;
	bool ret = true;
	int stride = 1;
	bool lock_status = 0;
/*
	while(!finish)
	{
		if(object->num_buckets<=NUM_BUCKETS_S/10)
			return false;

		DoWork = 1;
*/
     while(DoWork)
     {
         lock_status = 0;
         while (lock_status == 0)
          { 
            lock_status = local_getlock(&((object->locks)[index]));
          }
          index_base = index;
		  
        if(  *(__local int *) ( &(object->buckets)[index] ) ==0 ) // if(((object->buckets)[index]).x==0 && ((object->buckets)[index]).y==0)
           {
/*              unsigned short k = lmalloc(object, 2 + key_size);
					//if(k == -1)
					//	ret = false;

               unsigned short v = lmalloc(object, value_size); */
					//if(v == -1)
					//	ret = false;
/*
					if(ret == false)
					{
						local_releaselock(&(object->locks)[index]);
						finish  = 1;
						break;
					}
*/
					short k_align = ( align(key_size + 2 ) );
					short v_align = ( align(value_size) );	
                    short 	k       =  lmalloc(object, k_align + v_align);
                    short       v       =  k + (k_align>>2);	

              ((object->buckets)[index]).x = k;
              ((object->buckets)[index]).y = v;
  /*     *(__local int *) ( &(object->buckets)[index] ) =( (k<<16) | v);
 */
              __local char *key_size_address    = (__local char *)lget_address(object, k);
              __local char *value_size_address = key_size_address + 1;
                                       *key_size_address = key_size;
                                     *value_size_address = value_size;

					//store the key data and value data into the ro
                __local void *key_data_start       = key_size_address + 2;
                __local void *value_data_start    = lget_address(object, v);

                copyVal_global_to_local(key_data_start, key, key_size);
              *(__local int *) value_data_start = *(int *)value;//copyVal_private_to_local(value_data_start, value, value_size);
					//(object->buckets)[index] = kvn;
//                atomic_add(&(object->num_buckets), -1);
/*
					local_releaselock(&((object->locks)[index]));
					finish = 1;*/
                DoWork = false;
					//printf("inserting...\n");
              }

				/*compare them*/
            else
             {
					//printf("no **********************\n");
               unsigned short size      = lget_key_size(object, index);
              __local void *key_data = lget_key_address(object, index);

					/*if equal, conduct reduce*/
                   if(equal_global_and_local(key, key_size, key_data, size))
                     {
                       unsigned short value_ro_size  = lget_value_size(object, index);
                        __local void *value_ro_data   = lget_value_address(object, index);
						//printf("value size is: %d\n", value_size);
                        reduce_private_to_local(value_ro_data, value_ro_size, value, value_size);
                        DoWork = false;
                     }

					/*else, compute a new index and continue*/
                    else
                     {
                         DoWork = true;

                         index = (index + stride) &(NUM_BUCKETS_S-1);
                      }
                 }

             local_releaselock(&((object->locks)[index]));			

	 }

	return ret;
}

bool linsert_from_private(__local Reduction_Object_S *object,  void *key, unsigned char key_size, void *value, unsigned char value_size)
{
	unsigned int h = hash(key, key_size);//hash_global
	unsigned int index = h&(NUM_BUCKETS_S-1);
	unsigned int finish = 0;
	unsigned int index_base;
	//unsigned int kvn = 0;

	bool DoWork = true;
	bool ret = true;
	int stride = 1;
	bool lock_status = 0;

     while(DoWork)
     {
         lock_status = 0;
         while (lock_status == 0)
          { 
            lock_status = local_getlock(&((object->locks)[index]));
          }
          index_base = index;
		  
        if(  *(__local int *) ( &(object->buckets)[index] ) ==0 ) // if(((object->buckets)[index]).x==0 && ((object->buckets)[index]).y==0)
           {

					short k_align = ( align(key_size + 4 ) );
					short v_align = ( align(value_size) );	
                    short 	k       =  lmalloc(object, k_align + v_align);
                    short       v       =  k + (k_align>>2);	

              ((object->buckets)[index]).x = k;
              ((object->buckets)[index]).y = v;
  /*     *(__local int *) ( &(object->buckets)[index] ) =( (k<<16) | v);
 */
              __local char *key_size_address    = (__local char *)lget_address(object, k);
              __local char *value_size_address = key_size_address + 1;
                                       *key_size_address = key_size;
                                     *value_size_address = value_size;

					//store the key data and value data into the ro
                __local void *key_data_start       = key_size_address + 4;    
                __local void *value_data_start    = lget_address(object, v);

                *(__local int *) key_data_start = *(int *)key; //copyVal_private_to_local(key_data_start, key, key_size); //copyVal_global_to_local
                copyVal_private_to_local(value_data_start, value, value_size); //*(__local int *) value_data_start = *(int *)value;
					//(object->buckets)[index] = kvn;
//                atomic_add(&(object->num_buckets), -1);
/*
					local_releaselock(&((object->locks)[index]));
					finish = 1;*/
                DoWork = false;
					//printf("inserting...\n");
              }

				/*compare them*/
            else
             {
					//printf("no **********************\n");
               unsigned short size      = lget_key_size(object, index);
              __local void *key_data = lget_key_address(object, index);

					/*if equal, conduct reduce*/
                   if(equal_private_and_local(key, key_size, key_data, size)) //equal_global_and_local
                     {
                       unsigned short value_ro_size  = lget_value_size(object, index);
                        __local void *value_ro_data   = lget_value_address(object, index);
						//printf("value size is: %d\n", value_size);
                        reduce_private_to_local(value_ro_data, value_ro_size, value, value_size);
                        DoWork = false;
                     }

					/*else, compute a new index and continue*/
                    else
                     {
                         DoWork = true;

                         index = (index + stride) &(NUM_BUCKETS_S-1);
                      }
                 }

             local_releaselock(&((object->locks)[index_base ])); //index			

	 }

	return ret;
}

#endif
