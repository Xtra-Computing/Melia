#include "lib\kvbuffer.h"
#include "map.h"
#include "lib\configuration.h"
#include "lib\worker_info.h"
#include "lib\rog.h"
#include "lib\roc.h"
__attribute__((num_compute_units(2)))
__attribute__((num_simd_work_items(1)))
//__attribute__((reqd_work_group_size(1,1,1)))
__kernel void test(
			__global const void *global_offset, 
		    unsigned int offset_number, 
		    unsigned int unit_size,
			__global char *global_data,
			__global Reduction_Object_C *restrict object_c
			)
{			
		const uint global_id   = get_global_id(0);	
        const uint global_size = get_global_size(0);	
		
		int i=global_id;		
		cinit(object_c);
		barrier(CLK_GLOBAL_MEM_FENCE);
		for(; i < offset_number; i += global_size ) //CPU_GLOBAL_THREADS)
		{
			bool success = map_cpu(object_c, global_data, ((__global char *)global_offset+unit_size*i));
		}
}