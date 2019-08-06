#include "lib\kvbuffer.h"
#include "map.h"
#include "lib\configuration.h"
#include "lib\worker_info.h"
#include "lib\rog.h"
#include "lib\roc.h"
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
__attribute__((num_compute_units(1)))
__attribute__((num_simd_work_items(4)))
__attribute__((reqd_work_group_size(4,1,1)))
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