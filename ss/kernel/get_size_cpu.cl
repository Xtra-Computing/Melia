#include "lib\kvbuffer.h"
#include "map.h"
#include "lib\configuration.h"
#include "lib\worker_info.h"
#include "lib\rog.h"
#include "lib\roc.h"

__kernel void get_size_cpu(__global Reduction_Object_C *object_g, __global unsigned int *num_key, 
__global unsigned int *size_of_key, __global unsigned int *size_of_value)
{
    /*scan the arrays and get prefix sum info*/
    int global_id = get_global_id(0);
    if(global_id == 0)
    {
		for(int i = 1; i < NUM_BUCKETS_C; i++)
        {
            (object_g->align_bucket[i].pairs_per_bucket) += (object_g->align_bucket[i-1].pairs_per_bucket);
        }
    }

    if(global_id == 1)
    {
        for(int i = 1; i < NUM_BUCKETS_C; i++)
        {
            (object_g->align_bucket[i].key_size_per_bucket) += (object_g->align_bucket[i-1].key_size_per_bucket);
        }
    }

    if(global_id == 2)
    {
        for(int i = 1; i < NUM_BUCKETS_C; i++)
        {
            (object_g->align_bucket[i].value_size_per_bucket) += (object_g->align_bucket[i-1].value_size_per_bucket);
        }
    }

    *num_key = (object_g->align_bucket[NUM_BUCKETS_C - 1].pairs_per_bucket);
    *size_of_key = (object_g->align_bucket[NUM_BUCKETS_C - 1].key_size_per_bucket);
    *size_of_value = (object_g->align_bucket[NUM_BUCKETS_C - 1].value_size_per_bucket);
}

#if 0
__kernel void get_size_cpu(__global Reduction_Object_C *object_g, __global unsigned int *num_key, 
__global unsigned int *size_of_key, __global unsigned int *size_of_value)
{
    /*scan the arrays and get prefix sum info*/
    int global_id = get_global_id(0);
    if(global_id == 0)
    {
		for(int i = 1; i < NUM_BUCKETS_C; i++)
        {
            (object_g->pairs_per_bucket)[i] += (object_g->pairs_per_bucket)[i-1];
			//if(i==5830)
			//	printf("(object_g->pairs_per_bucket)[%d] %d\n",i,(object_g->pairs_per_bucket)[i]);
        }
		//*num_key = (object_g->pairs_per_bucket)[NUM_BUCKETS_G - 1];
    }

    if(global_id == 1)
    {
        for(int i = 1; i < NUM_BUCKETS_C; i++)
        {
            (object_g->key_size_per_bucket)[i] += (object_g->key_size_per_bucket)[i-1];
			//if(i==5863)
			//	printf("(object_g->key_size_per_bucket)[%d] %d\n",i,(object_g->key_size_per_bucket)[i]);
        }
		//*size_of_key = (object_g->key_size_per_bucket)[NUM_BUCKETS_G - 1];
    }

    if(global_id == 2)
    {
        for(int i = 1; i < NUM_BUCKETS_C; i++)
        {
            (object_g->value_size_per_bucket)[i] += (object_g->value_size_per_bucket)[i-1];
			//if(i==5830)
			//	printf("(object_g->value_size_per_bucket)[%d] %d\n",i,(object_g->value_size_per_bucket)[i]);
        }
		//*size_of_value = (object_g->value_size_per_bucket)[NUM_BUCKETS_G - 1];
    }
	barrier(CLK_LOCAL_MEM_FENCE);
    
	*num_key = (object_g->pairs_per_bucket)[NUM_BUCKETS_C - 1];
    *size_of_key = (object_g->key_size_per_bucket)[NUM_BUCKETS_C - 1];
    *size_of_value = (object_g->value_size_per_bucket)[NUM_BUCKETS_C - 1];
}
#endif
