#include "lib\rog.h"
#include "lib\rol.h"
#include "lib\rol_f.h"
#include "lib\configuration.h"
#include "mapreduce.h"
#include "lib\util_device.h"

#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_amd_printf : enable

void merge(__global Reduction_Object_G *object_g, __local Reduction_Object_S *object_s, 
__local unsigned int *global_object_offset)
{
	const unsigned int local_id = get_local_id(0);
	const unsigned int group_size = LOCAL_THREADS/NUM_GROUPS;
	//printf("I am thread %d\n", get_global_id(0));
	for(int index = local_id%group_size; index < NUM_BUCKETS_S; index += group_size)
	{
		if(!((object_s->buckets)[index].x==0&&(object_s->buckets)[index].y==0))
		{
			int key_size = lget_key_size(object_s,index);
			//printf("key size is: %d\n", key_size);
			int value_size = lget_value_size(object_s, index);
			__local void *key = lget_key_address(object_s, index);
			__local void *value = lget_value_address(object_s, index);
			ginsert_from_local(object_g, key, key_size, value, value_size, global_object_offset);
		}
	}
}

bool emit(void *key, unsigned short key_size, void *value, unsigned short value_size)
{
}

__kernel void start(
		__global Reduction_Object_G *object_g, 
	    	__global const void *global_data, 
	     	__global const void *global_offset, 
	     	unsigned int offset_number, 
	     	__global const void *local_data, 
	     	__local const void *shared_local_data, 
		unsigned int local_data_size, 
		unsigned int unit_size, 
		unsigned int uselocal
		)
{	
	__local unsigned int global_object_offset[LOCAL_THREADS/WAVEFRONT_SIZE];
	ginit(global_object_offset);


	for(int i = 0; i < local_data_size; i++)
	{
		((local char *)shared_local_data)[i] = ((global char *)local_data)[i];
	}

	/*for(int i = 0; i < K; i++)
	{
		for(int d = 0; d<DIM; d++)
		{
			clusters[i*DIM+d] = ((__global int *)(global_data))[DIM*i+d];
		}
	}*/

    const unsigned int global_id = get_global_id(0);
    const unsigned int local_id = get_local_id(0);
	//object_g->memory_offset = 0;

    barrier(CLK_LOCAL_MEM_FENCE);
	
    if(uselocal)
	{
		__local Reduction_Object_S objects[NUM_GROUPS];
		__local int do_merge;
		__local int finished;
		const unsigned int group_size = LOCAL_THREADS/NUM_GROUPS;
		const unsigned int gid = get_local_id(0)/group_size;
		
		linit(&objects[gid]);
		if(local_id == 0)
		{
			do_merge = 0;
			finished = 0;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
		
		bool flag = true;
		int i = global_id;
		while(finished!=LOCAL_THREADS)
		{
			for(; i < offset_number; i+= GLOBAL_THREADS)
			{
				if(do_merge)
				break;

				//bool success = true;
				bool success = map_to_local(&objects[gid], global_data, ((global char *)global_offset+unit_size*i), shared_local_data);
				if(!success)
				{
					atomic_xchg(&do_merge, 1);
					break;
				}
			}

			if(flag&&i>=offset_number)
			{
				flag = false;
				atomic_add(&finished, 1);
			}

			//printf("I am here..\n");

			barrier(CLK_LOCAL_MEM_FENCE);
			merge(object_g, &objects[gid], global_object_offset);
			barrier(CLK_LOCAL_MEM_FENCE);
			linit(&objects[gid]);
		}
	}
	
    else
	{
		for(int i = get_global_id(0); i < offset_number; i += GLOBAL_THREADS)
		{
			//printf("I am thread: %d and I am doing %d\n", get_global_id(0), i);
			map_to_global(object_g, global_data, ((__global char *)global_offset+unit_size*i), global_object_offset);
		}
		//barrier(CLK_GLOBAL_MEM_FENCE);
	}
}
