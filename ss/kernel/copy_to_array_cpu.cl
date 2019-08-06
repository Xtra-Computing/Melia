#include "lib\kvbuffer.h"
#include "map.h"
#include "lib\configuration.h"
#include "lib\worker_info.h"
#include "lib\rog.h"
#include "lib\roc.h"

__kernel void copy_to_array_cpu(__global Reduction_Object_C *object_g, __global void *key_array, 
    __global void *value_array, __global unsigned int *key_index, __global unsigned int *value_index)
{
	int i = get_global_id(0);
       const uint glocal_size = get_global_size(0);	
	   
    for(; i < NUM_BUCKETS_C; i += glocal_size) //GPU_GLOBAL_THREADS)
    {
        if(! (object_g->align_bucket[i].buckets.x==0&&object_g->align_bucket[i].buckets.y==0) )
        {
            int key_size;
            int value_size;
            
			if(i!=0)
			{
				key_size   = object_g->align_bucket[i].key_size_per_bucket     - object_g->align_bucket[i-1].key_size_per_bucket ;  //gget_key_size(object_g, i) - gget_key_size(object_g, i-1);
				value_size = object_g->align_bucket[i].value_size_per_bucket  - object_g->align_bucket[i-1].value_size_per_bucket;//gget_value_size(object_g, i) - gget_value_size(object_g, i-1);
			}

			else
			{
				key_size    = object_g->align_bucket[i].key_size_per_bucket ; //gget_key_size(object_g, i);
				value_size = object_g->align_bucket[i].value_size_per_bucket;//gget_value_size(object_g, i);
			}
			
			  __global void *key  = cget_address(object_g, object_g->align_bucket[i].buckets.x);//gget_key_address(object_g, i);
                     __global void *value = cget_address(object_g, object_g->align_bucket[i].buckets.y);////gget_value_address(object_g, i);

			//int key_test;
			//copyVal_global_to_private(&key_test, key, key_size);
			
			//printf("the key_size is: %d and key is: %d\n", key_size, key_test);

            unsigned int key_array_start    = object_g->align_bucket[i].key_size_per_bucket  - key_size;//(object_g->key_size_per_bucket)[i] - key_size;
            unsigned int value_array_start = object_g->align_bucket[i].value_size_per_bucket - value_size;//(object_g->value_size_per_bucket)[i] - value_size;
            unsigned int offset_pos = (object_g->align_bucket[i].pairs_per_bucket) - 1;
            copyVal_global_to_global((__global char *)key_array + key_array_start, key, key_size);
            copyVal_global_to_global((__global char *)value_array + value_array_start, value, value_size);
            key_index[offset_pos] = key_array_start;
            value_index[offset_pos] = value_array_start;
        }
    }
}
/*
__kernel void copy_to_array_cpu(__global Reduction_Object_C *object_g, __global void *key_array, 
    __global void *value_array, __global unsigned int *key_index, __global unsigned int *value_index)
{
	int i = get_global_id(0);
    for(; i < NUM_BUCKETS_C; i += CPU_GLOBAL_THREADS)
    {
        if(!((object_g->buckets)[i].x==0&&(object_g->buckets)[i].y==0))
        {
            int key_size;
            int value_size;
            
			if(i!=0)
			{
				key_size = cget_key_size(object_g, i) - cget_key_size(object_g, i-1);
				value_size = cget_value_size(object_g, i) - cget_value_size(object_g, i-1);
			}

			else
			{
				key_size = cget_key_size(object_g, i);
				value_size = cget_value_size(object_g, i);
			}
			
			__global void *key = cget_key_address(object_g, i);
            __global void *value = cget_value_address(object_g, i);

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
}*/
