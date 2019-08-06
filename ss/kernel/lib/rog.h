#ifndef REDUCTIONOBJECTG
#define REDUCTIONOBJECTG
#include "configuration.h"
#include "ds.h"

typedef struct object_g Reduction_Object_G;

struct object_g
{
	int num_buckets;
	int locks[NUM_BUCKETS_G];
	unsigned int key_size_per_bucket[NUM_BUCKETS_G];
	unsigned int value_size_per_bucket[NUM_BUCKETS_G];
	unsigned int pairs_per_bucket[NUM_BUCKETS_G];
	struct intl buckets[NUM_BUCKETS_G];
	unsigned int memory_pool[MAX_POOL_G];
	unsigned int memory_offset;
	unsigned int offsets[GPU_GLOBAL_THREADS/WAVEFRONT_SIZE];
};

#endif