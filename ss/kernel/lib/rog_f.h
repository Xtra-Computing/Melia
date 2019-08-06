#ifndef ROGF
#define ROGF
#include "rog.h"
#include "util_device.h"
#include "hash.h"

//#ifndef MAPREDUCE
#include "reduce.h"
//#endif
#include "atomic.h"

#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
#pragma OPENCL EXTENSION cl_amd_printf : enable

/*initialize the memory offsets for the global memory reduction object*/
void ginit(__local unsigned int * global_object_offset)
{
	int global_id = get_global_id(0);
	int local_id = get_local_id(0);
	int local_group_id = local_id/WAVEFRONT_SIZE;
	if(local_id%WAVEFRONT_SIZE==0)
	{
		global_object_offset[local_group_id] = ((float)global_id/(float)GPU_GLOBAL_THREADS)*MAX_POOL_G;
		//printf("starting offset: %d\n", global_object_offset[local_group_id]);
	}
}

int gmalloc(__global Reduction_Object_G *object, unsigned int size, local unsigned int *global_object_offset)
{
	size = align(size)/ALIGN_SIZE;
	unsigned int tid = get_local_id(0);
	unsigned int gid = tid/WAVEFRONT_SIZE;   //group id
	unsigned int offset = atomic_add(&global_object_offset[gid], size);
	return offset;
}

__global void * gget_address(__global Reduction_Object_G *object, int memory_index)
{
	return object->memory_pool + memory_index;
}

__global void * gget_key_address(__global Reduction_Object_G *object, int bucket_index)
{
	unsigned int key_index = (object->buckets)[bucket_index].x;
	__global void *key_address = gget_address(object, key_index);
	return key_address;
}

unsigned int gget_key_size(__global Reduction_Object_G *object, int bucket_index)
{
	return (object->key_size_per_bucket)[bucket_index];
}

__global void * gget_value_address(__global Reduction_Object_G *object, int bucket_index)
{
	unsigned int value_index = (object->buckets)[bucket_index].y;
	return gget_address(object, value_index);
}

unsigned int gget_value_size(__global Reduction_Object_G *object, int bucket_index)
{
	return (object->value_size_per_bucket)[bucket_index];
}

/*insert from local to global*/
bool ginsert_from_local(__global Reduction_Object_G *object, __local void *key, 
unsigned short key_size, __local void *value, unsigned short value_size, 
__local unsigned int *global_object_offset)
{
	unsigned int h = hash_local(key, key_size);
	unsigned int index = h%NUM_BUCKETS_G;
	unsigned int finish = 0;

	int DoWork = 1;
	bool ret = true;
	int stride = 1;

	while(!finish)
	{
		DoWork = 1;

		while(DoWork)
		{
			if(global_getlock(&(object->locks)[index]))
			{
				if((object->pairs_per_bucket)[index]==0)
				{
					int k = gmalloc(object, key_size, global_object_offset);
					if(k == -1)
						ret = false;

					int v = gmalloc(object, value_size, global_object_offset);
					if(v == -1)
						ret = false;

					if(ret == false)
					{
						global_releaselock(&(object->locks)[index]);
						finish = 1;
						ret = false;
						break;
					}

					((object->buckets)[index]).x = k;
					((object->buckets)[index]).y = v;

					(object->key_size_per_bucket)[index] = key_size;
					(object->value_size_per_bucket)[index] = value_size;
					(object->pairs_per_bucket)[index] = 1;

					global void *key_data_start = gget_address(object, k);
					global void *value_data_start = gget_address(object, v);

					copyVal_local_to_global(key_data_start, key, key_size);
					copyVal_local_to_global(value_data_start, value, value_size);

					global_releaselock(&(object->locks)[index]);
					finish = 1;
					DoWork = 0;
					//printf("success insert...\n");
				}

				/*compare them*/
				else
				{
					unsigned int size = gget_key_size(object, index);
					__global void *key_data = gget_key_address(object, index);

					//size = ((object->key_size_per_bucket)[index]);

					/*if equal, conduct reduce*/
					if(equal_local_and_global(key, key_size, key_data, size))
					{	
						unsigned int value_ro_size = gget_value_size(object, index);
						__global void *value_ro_data = gget_value_address(object, index);
						reduce_local_to_global(value_ro_data, value_ro_size, value, value_size);
						DoWork = 0;
						finish = 1;
						ret = true;
						global_releaselock(&(object->locks)[index]);
						//printf("success reduce...\n");
						
					}

					/*else, compute a new index and continue*/
					else
					{
						DoWork = 0;
						//finish = 1;
						global_releaselock(&(object->locks)[index]);
						index = (index + stride)%NUM_BUCKETS_G;
						//printf("********new index...\n");
					}
				}
			}
		}
	}
	return ret;
}

/*insert from private to global directly*/
bool ginsert_from_private(__global Reduction_Object_G *object, void *key, 
unsigned short key_size, void *value, unsigned short value_size, 
__local unsigned int *global_object_offset)
{
	unsigned int h = hash(key, key_size);
	unsigned int index = h%NUM_BUCKETS_G;
	unsigned int finish = 0;
	//volatile LONG_INT kvn;
	//kvn.first = kvn.second = 0;

	int DoWork = 1;
	bool ret = true;
	int stride = 1;

	while(!finish)
	{
		DoWork = 1;

		while(DoWork)
		{
			if(global_getlock(&((object->locks)[index])))
			{
				//printf("The pair per bucket: %d\n", (object->pairs_per_bucket)[index]);
				//if(*(long long *)&((object->buckets)[index])==0)
				if((object->pairs_per_bucket)[index]==0)
				{	
					//printf("index is: %d\n", index);

					int k = gmalloc(object, key_size, global_object_offset);
					if(k == -1)
						ret = false;

					int v = gmalloc(object, value_size, global_object_offset);
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

					global char *key_size_address = (global char *)gget_address(object, k);

					//store the key data and value data into the ro
					global void *key_data_start = key_size_address;
					global void *value_data_start = gget_address(object, v);

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
					unsigned int size = gget_key_size(object, index);
					global void *key_data = gget_key_address(object, index);

					//size = ((object->key_size_per_bucket)[index]);
					//printf("index is: %d key1 is: %d and size is: %d\n", index, key1, size);

					/*if equal, conduct reduce*/
					if(equal_private_and_global(key, key_size, key_data, size))
					{
						//printf("I am thread %d and I am reducing at bucket %d\n", get_global_id(0), index);
						unsigned int value_ro_size = gget_value_size(object, index);
						global void *value_ro_data = gget_value_address(object, index);

						reduce_private_to_global(value_ro_data, value_ro_size, value, value_size);
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
						index = (index + stride)%NUM_BUCKETS_G;
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
bool ginsert_from_global(__global Reduction_Object_G *object, __global void *key, 
unsigned short key_size, __global void *value, unsigned short value_size, 
__local unsigned int *global_object_offset)
{
	unsigned int h = hash_global(key, key_size);
	unsigned int index = h%NUM_BUCKETS_G;
	unsigned int finish = 0;
	//volatile LONG_INT kvn;
	//kvn.first = kvn.second = 0;

	int DoWork = 1;
	bool ret = true;
	int stride = 1;

	while(!finish)
	{
		DoWork = 1;

		while(DoWork)
		{
			if(global_getlock(&((object->locks)[index])))
			{
				//printf("The pair per bucket: %d\n", (object->pairs_per_bucket)[index]);
				//if(*(long long *)&((object->buckets)[index])==0)
				if((object->pairs_per_bucket)[index]==0)
				{	
					//printf("index is: %d\n", index);

					int k = gmalloc(object, key_size, global_object_offset);
					if(k == -1)
						ret = false;

					int v = gmalloc(object, value_size, global_object_offset);
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

					global char *key_size_address = (global char *)gget_address(object, k);

					//store the key data and value data into the ro
					global void *key_data_start = key_size_address;
					global void *value_data_start = gget_address(object, v);

					copyVal_global_to_global(key_data_start, key, key_size);
					copyVal_global_to_global(value_data_start, value, value_size);

					global_releaselock(&((object->locks)[index]));

					finish = 1;
					DoWork = false;
				}

				/*compare them*/
				else
				{
					//printf("******not zero**********\n");
					unsigned int size = gget_key_size(object, index);
					global void *key_data = gget_key_address(object, index);

					//size = ((object->key_size_per_bucket)[index]);
					//printf("index is: %d key1 is: %d and size is: %d\n", index, key1, size);

					/*if equal, conduct reduce*/
					if(equal_global_and_global(key, key_size, key_data, size))
					{
						//printf("I am thread %d and I am reducing at bucket %d\n", get_global_id(0), index);
						unsigned int value_ro_size = gget_value_size(object, index);
						global void *value_ro_data = gget_value_address(object, index);

						reduce_global_to_global(value_ro_data, value_ro_size, value, value_size);
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
						index = (index + stride)%NUM_BUCKETS_G;
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