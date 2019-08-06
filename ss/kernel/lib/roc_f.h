/*functions used in roc*/
#ifndef ROCF
#define ROCF
#include "roc.h"
#include "util_device.h"
#include "hash.h"

//#ifndef MAPREDUCE
#include "reduce.h"
//#endif
#include "atomic.h"

#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
//#pragma OPENCL EXTENSION cl_amd_printf : enable

/*initialize the memory offsets for the CPU reduction object*/
void cinit(__global Reduction_Object_C *roc)
{
//	int global_id     = get_global_id(0);
	int block_id      = get_group_id(0);
	int local_id       = get_local_id(0);
	
       int group_size =get_num_groups (0);
//       int local_size   =get_local_size(0);
	   
	if(local_id == 0)
	{
		(roc->memory_offset)[block_id] = (block_id* MAX_POOL_C)/group_size;
	}	
}

int cmalloc(__global Reduction_Object_C *roc, unsigned short size)
{
	//size = align(size)/ALIGN_SIZE;
	unsigned int align_size =(unsigned int) ( align(size) )>>2;
//	int global_id = get_global_id(0);
//	int local_id = get_local_id(0);
//	int block_id = global_id/CPU_LOCAL_THREADS;
	int block_id      = get_group_id(0);

	unsigned int offset = atomic_add(&(roc->memory_offset)[block_id], align_size ); //size
	//printf("Allocated in %d\n", offset);
	/*unsigned int offset = (roc->memory_offset)[block_id];
	(roc->memory_offset)[block_id]++;*/
	return offset;
}

__global void * cget_address(__global Reduction_Object_C *object, int memory_index)
{
	return object->memory_pool + memory_index;
}
#if 0
__global void * cget_key_address(__global Reduction_Object_C *object, int bucket_index)
{
	unsigned int key_index = (object->buckets)[bucket_index].x;
	__global void *key_address = cget_address(object, key_index);
	return key_address;
}

unsigned int cget_key_size(__global Reduction_Object_C *object, int bucket_index)
{
	return (object->key_size_per_bucket)[bucket_index];
}

__global void * cget_value_address(__global Reduction_Object_C *object, int bucket_index)
{
	unsigned int value_index = (object->buckets)[bucket_index].y;
	return cget_address(object, value_index);
}

unsigned int cget_value_size(__global Reduction_Object_C *object, int bucket_index)
{
	return (object->value_size_per_bucket)[bucket_index];
}

/*insert from private to global directly*/
bool cinsert_from_private(__global Reduction_Object_C *object, void *key, 
unsigned short key_size, void *value, unsigned short value_size)
{
	unsigned int h = hash(key, key_size);
	unsigned int index = h%NUM_BUCKETS_C;
	unsigned int finish = 0;

	int DoWork = true;
	bool ret = true;
	int stride = 1;

	while(!finish)
	{
		while(DoWork)
		{
			DoWork = 1;
			//printf("locked\n");
			if(global_getlock(&((object->locks)[index])))
			{
				if((object->pairs_per_bucket)[index]==0)
				{	
					int k = cmalloc(object, key_size);
					if(k == -1)
						ret = false;

					int v = cmalloc(object, value_size);
					if(v == -1)
						ret = false;

					if(ret == false)
					{
						global_releaselock(&(object->locks)[index]);
						finish = 1;
						break;
					}

					((object->buckets)[index]).x = k;
					((object->buckets)[index]).y = v;

				
					(object->key_size_per_bucket)[index] = key_size;
					(object->value_size_per_bucket)[index] = value_size;
					(object->pairs_per_bucket)[index] = 1;

					__global char *key_size_address = (global char *)cget_address(object, k);

					__global void *key_data_start = key_size_address;
					__global void *value_data_start = cget_address(object, v);

					copyVal_private_to_global(key_data_start, key, key_size);
					copyVal_private_to_global(value_data_start, value, value_size);

					global_releaselock(&((object->locks)[index]));

					finish = 1;
					DoWork = false;
				}

				/*compare them*/
				else
				{
					//printf("******not zero**********\n");
					unsigned int size = cget_key_size(object, index);
					__global void *key_data = cget_key_address(object, index);

					size = ((object->key_size_per_bucket)[index]);

					/*if equal, conduct reduce*/
					if(equal_private_and_cpu(key, key_size, key_data, size))
					{
						//printf("I am thread %d and I am reducing at bucket %d\n", get_global_id(0), index);
						unsigned short value_ro_size = cget_value_size(object, index);
						__global void *value_ro_data = cget_value_address(object, index);

						reduce_private_to_cpu(value_ro_data, value_ro_size, value, value_size);
						DoWork = false;
						finish = 1;
						ret = true;
						global_releaselock(&((object->locks)[index]));
						//printf("I am thread %d and I release the lock at bucket %d\n", get_global_id(0), index);
					}

					/*else, compute a new index and continue*/
					else
					{
						DoWork = false;
						//finish = true;
						global_releaselock(&((object->locks)[index]));
						index = (index + stride)%NUM_BUCKETS_C;
					}
				}
			}
		}
		if(finish)
			return ret;
	}
	return ret;
}

/*insert from private to global directly*/
bool cinsert_from_global(__global Reduction_Object_C *object, __global void *key, 
unsigned short key_size, __global void *value, unsigned short value_size)
{
	unsigned int h = hash_global(key, key_size);
	unsigned int index = h%NUM_BUCKETS_C;
	unsigned int finish = 0;

	int DoWork = true;
	bool ret = true;
	int stride = 1;

	while(!finish)
	{
		DoWork = 1;

		while(DoWork)
		{
			//printf("locked\n");
			if(global_getlock(&((object->locks)[index])))
			{
				if((object->pairs_per_bucket)[index]==0)
				{	
					int k = cmalloc(object, key_size);
					if(k == -1)
						ret = false;

					int v = cmalloc(object, value_size);
					if(v == -1)
						ret = false;

					if(ret == false)
					{
						global_releaselock(&(object->locks)[index]);
						finish = 1;
						break;
					}

					((object->buckets)[index]).x = k;
					((object->buckets)[index]).y = v;

				
					(object->key_size_per_bucket)[index] = key_size;
					(object->value_size_per_bucket)[index] = value_size;
					(object->pairs_per_bucket)[index] = 1;

					__global char *key_size_address = (global char *)cget_address(object, k);

					__global void *key_data_start = key_size_address;
					__global void *value_data_start = cget_address(object, v);

					copyVal_global_to_global(key_data_start, key, key_size);
					copyVal_global_to_global(value_data_start, value, value_size);

					global_releaselock(&((object->locks)[index]));

					finish = 1;
					DoWork = false;
				}

				/*compare them*/
				else
				{
					unsigned int size = cget_key_size(object, index);
					__global void *key_data = cget_key_address(object, index);

					//size = ((object->key_size_per_bucket)[index]);

					/*if equal, conduct reduce*/
					if(equal_global_and_global(key, key_size, key_data, size))
					{
						//printf("I am thread %d and I am reducing at bucket %d\n", get_global_id(0), index);
						unsigned int value_ro_size = cget_value_size(object, index);
						__global void *value_ro_data = cget_value_address(object, index);

						reduce_global_to_cpu(value_ro_data, value_ro_size, value, value_size);
						DoWork = false;
						finish = 1;
						ret = true;
						global_releaselock(&((object->locks)[index]));
						//printf("I am thread %d and I release the lock at bucket %d\n", get_global_id(0), index);
					}

					/*else, compute a new index and continue*/
					else
					{
						DoWork = false;
						//finish = true;
						global_releaselock(&((object->locks)[index]));
						index = (index + stride)%NUM_BUCKETS_C;
					}
				}
			}
		}
		if(finish)
			return ret;
	}
	return ret;
}

#endif
/*insert from global(k) and private(v) to global directly*/
bool cinsert_from_gp(__global Reduction_Object_C *object, __global void *key, 
unsigned short key_size, void *value, unsigned short value_size)
{
	unsigned int h = hash_global(key, key_size);
	unsigned int index = h & (NUM_BUCKETS_C-1);
	unsigned int finish = 0;
	unsigned int index_base;

	int DoWork = true;
	bool ret = true;
	int stride = 1;
	int lock_status;

       while(DoWork)
       { 
         lock_status = 0;
         while (lock_status == 0)
          { 
            lock_status = global_getlock(&((object->locks)[index]));
          }
		  index_base = index;

         uint4 key_data_unit4 =*(__global uint4*)(& (object->align_bucket)[index]); //.bucket_data_int4; /*load the unt4 data for the merge*/

         if (key_data_unit4.y == 0)//if((object->pairs_per_bucket)[index]==0)
          {	
             unsigned short k_align =  align(key_size  );
             unsigned short v_align =  align(value_size);	
             unsigned     int k           =  cmalloc(object, k_align + v_align);
             unsigned     int v           =  k + (k_align>>2);					

             key_data_unit4.xyzw = (uint4) ( (value_size<<16)|(key_size&(0xffff)), 1, k, v ); //(uint4) ( (key_size<<16)|(value_size&(0xffff)), 1, k, v );
			 
            *(__global uint4*)(& (object->align_bucket)[index]) = key_data_unit4; //( (object->align_bucket)[index]).bucket_data_int4 =  key_data_unit4; 			 
/*					((object->buckets)[index]).x = k;
					((object->buckets)[index]).y = v;
				
					(object->key_size_per_bucket)[index] = key_size;
					(object->value_size_per_bucket)[index] = value_size;
					(object->pairs_per_bucket)[index] = 1;
*/
              __global char *key_data_start = (__global char *)cget_address(object, k);
              __global void *value_data_start = cget_address(object, v);

             copyVal_global_to_global(key_data_start, key, key_size);
             *((__global int *)value_data_start) = *( ( int *)value );  /*copyVal_private_to_global(value_data_start, value, value_size);*/

               DoWork = false;
             }
				/*compare them*/
           else
            {
                unsigned int size              = (key_data_unit4.x) &(0xffff);//(key_data_unit4.x)>>16; //cget_key_size(object, index);
                 __global void *key_data = cget_address(object, (key_data_unit4.z) )  ;//cget_key_address(object, index);

					/*if equal, conduct reduce*/
               if(equal_global_and_global(key, key_size, key_data, size))
               {
                    unsigned short value_ro_size = 4; //(key_data_unit4.x) >>16; cget_value_size(object, index);
                     __global void *value_ro_data = cget_address(object, (key_data_unit4.w) )  ;//cget_value_address(object, index);
						//int value_ = *(int*)value_ro_data;							
                    reduce_private_to_global(value_ro_data, value_ro_size, value, value_size);
                    DoWork = false;
                }
               else
                {
                   DoWork = true; //false; ^_^
                   index = (index + stride) &(NUM_BUCKETS_C-1);
                 }
             }

           global_releaselock(&(object->locks)[index_base]);//index
			
        }
//		if(finish)
//			return ret;
//	}
	return ret;
}

bool cinsert_from_private(__global Reduction_Object_C *object, void *key, 
unsigned short key_size, void *value, unsigned short value_size)
{
	unsigned int h = hash(key, key_size);
	unsigned int index = h; //&(NUM_BUCKETS_C-1);
	unsigned int finish = 0;
    unsigned int index_base;

	int DoWork = true;
	bool ret = true;
	int stride = 1;
	int lock_status;

       while(DoWork)
       { 
       
         lock_status = 0;
         while (lock_status == 0)
          { 
            lock_status = global_getlock(&((object->locks)[index]));
          }
          index_base = index;
		  
         uint4 key_data_unit4 =*(__global uint4*)(& (object->align_bucket)[index]); //.bucket_data_int4; /*load the unt4 data for the merge*/

         if (key_data_unit4.y == 0)//if((object->pairs_per_bucket)[index]==0)
          {	
             unsigned short k_align =  align(key_size  );
             unsigned short v_align =  align(value_size);	
             unsigned     int k           =  cmalloc(object, k_align + v_align);
             unsigned     int v           =  k + (k_align>>2);					

             key_data_unit4.xyzw = (uint4) ( (value_size<<16)|(key_size&(0xffff)), 1, k, v );//(uint4) ( (key_size<<16)|(value_size&(0xffff)), 1, k, v );
			 
            *(__global uint4*)(& (object->align_bucket)[index]) =  key_data_unit4;//( (object->align_bucket)[index]).bucket_data_int4 =  key_data_unit4; 			 
/*					((object->buckets)[index]).x = k;
					((object->buckets)[index]).y = v;
				
					(object->key_size_per_bucket)[index] = key_size;
					(object->value_size_per_bucket)[index] = value_size;
					(object->pairs_per_bucket)[index] = 1;
*/
              __global char *key_data_start = (__global char *)cget_address(object, k);
              __global void *value_data_start = cget_address(object, v);

             *((__global int *)key_data_start)   = *( ( int *)key ); //copyVal_private_to_global(key_data_start, key, key_size);
             *((__global int *)value_data_start) = *( ( int *)value );  /*copyVal_private_to_global(value_data_start, value, value_size);*/

               DoWork = false;
             }
				/*compare them*/
           else
            {
                unsigned int size              = (key_data_unit4.x) &(0xffff);//(key_data_unit4.x)>>16; //cget_key_size(object, index);
                 __global void *key_data = cget_address(object, (key_data_unit4.z) )  ;//cget_key_address(object, index);

					/*if equal, conduct reduce*/
               if(equal_private_and_global(key, key_size, key_data, size)) //(equal_global_and_global(key, key_size, key_data, size))
               {
                    unsigned short value_ro_size = 4; //(key_data_unit4.x) >>16; cget_value_size(object, index);
                     __global void *value_ro_data = cget_address(object, (key_data_unit4.w) )  ;//cget_value_address(object, index);
						//int value_ = *(int*)value_ro_data;							
                    reduce_private_to_global(value_ro_data, value_ro_size, value, value_size);
                    DoWork = false;
                }
               else
                {
                   DoWork = true; //false; ^_^
                   index = (index + stride); //&(NUM_BUCKETS_C-1);
                 }
             }

           global_releaselock(&(object->locks)[index_base]); //index
			
        }

}
#endif
