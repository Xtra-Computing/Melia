#include "lib\kvbuffer.h"
#include "map.h"
#include "lib\configuration.h"
#include "lib\worker_info.h"
#include "lib\rog.h"
#include "lib\roc.h"
void merge(__global Reduction_Object_G *object_g, __local Reduction_Object_S *object_s, 
__local unsigned int *global_object_offset)
{
	const unsigned int local_id     = get_local_id(0);
	const unsigned int local_size  = get_local_size(0);
	
	for(int index = local_id; index < NUM_BUCKETS_S; index += local_size)
	{
		//printf("x: %d y: %d\n", (object_s->buckets)[index].x, (object_s->buckets)[index].y);
		if(  *(__local int *)( &( (object_s->buckets)[index] ) ) !=0   )//if( !( (object_s->buckets)[index].x==0 )  )//if(!((object_s->buckets)[index].x==0&&(object_s->buckets)[index].y==0))//to be one load
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
__attribute__((num_compute_units(2)))
__attribute__((num_simd_work_items(1)))
//__attribute__(max_work_group_size(256))
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
		__local Reduction_Object_S objects;
		const uint local_id      =  get_local_id(0);	
		const uint global_id    = get_global_id(0);
		const uint glocal_size = get_global_size(0);

		linit(&objects);
	
          
		__local unsigned int global_object_offset[1];//need it?

		ginit(object_g); //bug
		
//        if (global_id == 0)
//		 object_g->memory_offset = 0;
//        if (local_id == 0)
//		  global_object_offset[1] = 0;
                    __global char *global_data_tmp = global_data;
                      
                      __local   char private_key[4];//MAX_KEY_SIZE
		                 short key_size = 0;
	                     char  key_target_char = 1;
	                     while(  ( key_target_char!= '\0' ) ) //one valid, compare
	                     {    
	                        key_target_char = *global_data_tmp;
							private_key[key_size] = key_target_char;
							global_data_tmp++;
							key_size++;
						 }
	       

		  
		barrier(CLK_LOCAL_MEM_FENCE);
		

			 

		//bool flag = true;

              for(int i =global_id; i < offset_number; i += glocal_size)
               {
                 //int success = map_local(&objects, global_data, ((__global char *)global_offset+unit_size*i));
				 
				   int success = map_local(&objects, global_data, private_key, ((__global char *)global_offset+unit_size*i));
/*
				if(!success)
				{
				//	do_merge = 1;
					break;
				}
*/
                }

              barrier(CLK_LOCAL_MEM_FENCE);

              merge(object_g, &objects, global_object_offset);

			//barrier(CLK_LOCAL_MEM_FENCE);

			//linit(&objects[gid]);

}