#include "lib\kvbuffer.h"
#include "map.h"
#include "lib\configuration.h"
#include "lib\worker_info.h"
#include "lib\rog.h"
#include "lib\roc.h"

#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
//#pragma OPENCL EXTENSION cl_amd_printf : enable

void merge(__global Reduction_Object_G *object_g, __local Reduction_Object_S *object_s, 
__local unsigned int *global_object_offset)
{
	const unsigned int local_id = get_local_id(0);
	const unsigned int group_size = GPU_LOCAL_THREADS/NUM_GROUPS;
	
	for(int index = local_id%group_size; index < NUM_BUCKETS_S; index += group_size)
	{
		//printf("x: %d y: %d\n", (object_s->buckets)[index].x, (object_s->buckets)[index].y);
		if(!((object_s->buckets)[index].x==0&&(object_s->buckets)[index].y==0))
		{
			
			int key_size = lget_key_size(object_s,index);
			int value_size = lget_value_size(object_s, index);
			__local void *key = lget_key_address(object_s, index);
			__local void *value = lget_value_address(object_s, index);

		//	printf("key is: %c\n", *(__local char *)key);

			ginsert_from_local(object_g, key, key_size, value, value_size, global_object_offset);
		}
	}
}

__kernel void test(
			__global const void *global_offset, 
		    unsigned int offset_number, 
		    unsigned int unit_size,
			__global char *global_data,
			__global Reduction_Object_G *object_g
			)
{
		//__local int do_merge;
		//__local int finished;
		__local Reduction_Object_S objects[NUM_GROUPS];
		const uint local_id =  get_local_id(0);	
		const uint global_id = get_global_id(0);
		const unsigned int group_size = GPU_LOCAL_THREADS/NUM_GROUPS;
		const unsigned int gid = local_id/group_size;

		linit(&objects[gid]);
	
		//int cpu_blocks = CPU_GLOBAL_THREADS/CPU_LOCAL_THREADS;
		//int gpu_blocks = GPU_GLOBAL_THREADS/GPU_LOCAL_THREADS;
		
		//const uint block_id = get_group_id(0);
		//int block_index = block_id + cpu_blocks;

		__local unsigned int global_object_offset[GPU_LOCAL_THREADS/WAVEFRONT_SIZE];

		ginit(global_object_offset);

		barrier(CLK_LOCAL_MEM_FENCE);

		//bool flag = true;

		int i =global_id;
	

			for(; i < offset_number; i += GPU_GLOBAL_THREADS)
			{
				int success = map_local(&objects[gid], global_data, ((__global char *)global_offset+unit_size*i));

				if(!success)
				{
				//	do_merge = 1;
					break;
				}
			}

			barrier(CLK_LOCAL_MEM_FENCE);

			merge(object_g, &objects[gid], global_object_offset);

			//barrier(CLK_LOCAL_MEM_FENCE);

			//linit(&objects[gid]);

}
__kernel void copy_to_array_gpu(__global Reduction_Object_G *object_g, __global void *key_array, 
    __global void *value_array, __global unsigned int *key_index, __global unsigned int *value_index)
{
	int i = get_global_id(0);
    for(; i < NUM_BUCKETS_G; i += GPU_GLOBAL_THREADS)
    {
        if(!((object_g->buckets)[i].x==0&&(object_g->buckets)[i].y==0))
        {
            int key_size;
            int value_size;
            
			if(i!=0)
			{
				key_size = gget_key_size(object_g, i) - gget_key_size(object_g, i-1);
				value_size = gget_value_size(object_g, i) - gget_value_size(object_g, i-1);
			}

			else
			{
				key_size = gget_key_size(object_g, i);
				value_size = gget_value_size(object_g, i);
			}
			
			__global void *key = gget_key_address(object_g, i);
            __global void *value = gget_value_address(object_g, i);

			//int key_test;
			//copyVal_global_to_private(&key_test, key, key_size);
			
			//printf("the key_size is: %d and key is: %d\n", key_size, key_test);

            unsigned int key_array_start = (object_g->key_size_per_bucket)[i] - key_size;
            unsigned int value_array_start = (object_g->value_size_per_bucket)[i] - value_size;
            unsigned int offset_pos = (object_g->pairs_per_bucket)[i] - 1;
            copyVal_global_to_global((__global char *)key_array + key_array_start, key, key_size);
            copyVal_global_to_global((__global char *)value_array + value_array_start, value, value_size);
            key_index[offset_pos] = key_array_start;
            value_index[offset_pos] = value_array_start;
        }
    }
}


__kernel void get_size_gpu(__global Reduction_Object_G *object_g, __global unsigned int *num_key, 
__global unsigned int *size_of_key, __global unsigned int *size_of_value)
{
    /*scan the arrays and get prefix sum info*/
    int global_id = get_global_id(0);
    if(global_id == 0)
    {
		for(int i = 1; i < NUM_BUCKETS_G; i++)
        {
            (object_g->pairs_per_bucket)[i] += (object_g->pairs_per_bucket)[i-1];
        }
    }

    if(global_id == 1)
    {
        for(int i = 1; i < NUM_BUCKETS_G; i++)
        {
            (object_g->key_size_per_bucket)[i] += (object_g->key_size_per_bucket)[i-1];
        }
    }

    if(global_id == 2)
    {
        for(int i = 1; i < NUM_BUCKETS_G; i++)
        {
            (object_g->value_size_per_bucket)[i] += (object_g->value_size_per_bucket)[i-1];
        }
    }

    *num_key = (object_g->pairs_per_bucket)[NUM_BUCKETS_G - 1];
    *size_of_key = (object_g->key_size_per_bucket)[NUM_BUCKETS_G - 1];
    *size_of_value = (object_g->value_size_per_bucket)[NUM_BUCKETS_G - 1];
}