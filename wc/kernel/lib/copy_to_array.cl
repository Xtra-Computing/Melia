#include "lib\rog.h"
#include "lib\rol.h"
#include "lib\rol_f.h"
#include "lib\configuration.h"
#include "lib\util_device.h"

#include "mapreduce.h"

#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_amd_printf : enable

__kernel void copy_to_array(__global Reduction_Object_G *object_g, __global void *key_array, 
    __global void *value_array, __global unsigned int *key_index, __global unsigned int *value_index)
{
	int i = get_global_id(0);
    for(; i < NUM_BUCKETS_G; i += GLOBAL_THREADS)
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