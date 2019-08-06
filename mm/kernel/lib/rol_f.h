#ifndef ROLF
#define ROLF

#include "rol.h"
#include "configuration.h"
#include "util_device.h"
#include "hash.h"
#include "atomic.h"

#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
//#pragma OPENCL EXTENSION cl_amd_printf : enable

void linit(__local Reduction_Object_S *object)
{
	const unsigned int local_id = get_local_id(0);
	const unsigned int group_size = GPU_LOCAL_THREADS/NUM_GROUPS;
	for(int index = local_id%group_size; index < NUM_BUCKETS_S; index += group_size)
	{
		(object->buckets)[index].x = 0;
		(object->buckets)[index].y = 0;
		(object->locks)[index] = 0;
	}

	if(local_id%group_size == 0)
	{
		object->num_buckets = NUM_BUCKETS_S;
		object->memory_offset = 0;
	}
}

int lmalloc(__local Reduction_Object_S *object, unsigned int size)
{
	size = align(size)/ALIGN_SIZE;
	unsigned int offset = atomic_add(&(object->memory_offset), size);
	if(offset + size > MAX_POOL_S)
		return -1;
	else
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
	return size_address+2;   
}

unsigned short lget_key_size(__local Reduction_Object_S *object, unsigned bucket_index)
{
	unsigned short key_index = ((object->buckets)[bucket_index]).x;
	__local char *size_address = lget_address(object, key_index);
	return *size_address;    
}

__local void * lget_value_address(__local Reduction_Object_S *object, unsigned bucket_index)
{
	unsigned short value_index =  ((object->buckets)[bucket_index]).y;;
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

/*key in global mem, and value in private mem*/
bool linsert_from_gp(__local Reduction_Object_S *object, __global void *key, unsigned short key_size, void *value, unsigned short value_size)
{
	unsigned int h = hash_global(key, key_size);
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

					//printf("char: %c\n", *(__global char *)key);

					copyVal_global_to_local(key_data_start, key, key_size);
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
					if(equal_global_and_local(key, key_size, key_data, size))
					{
						//printf("I am thread %d and I am reducing at bucket %d\n", get_global_id(0), index);
						//printf("the key is: %c and key size is: %d\n", *(__local char *)key_data, size);
						
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


#endif