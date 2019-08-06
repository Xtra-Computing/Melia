#include "lib\kvbuffer.h"
#include "map.h"
#include "lib\configuration.h"
#include "lib\worker_info.h"
#include "lib\rog.h"
#include "lib\roc.h"
__kernel void test(
			__global const void *global_offset, 
		    unsigned int offset_number, 
		    unsigned int unit_size,
			__global char *global_data,
			__global Reduction_Object_C *object_c
			)
{		
		const uint local_id =  get_local_id(0);	
		const uint global_id = get_global_id(0);

		int block_index = get_group_id(0);

		cinit(object_c);
		barrier(CLK_LOCAL_MEM_FENCE);
	//	printf("object->pairs_per_bucket)[index] %d \n",(object_c->pairs_per_bucket)[4774]);
		for(int i =global_id ; i < offset_number; i += CPU_GLOBAL_THREADS)
		{
			bool success = map_cpu(object_c, global_data, ((__global char *)global_offset+unit_size*i));
		}
}

