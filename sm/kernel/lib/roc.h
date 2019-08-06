/*reduction object for CPU*/
#ifndef REDUCTIONOBJECTC
#define REDUCTIONOBJECTC
#include "configuration.h"
#include "ds.h"

typedef struct object_c Reduction_Object_C;

struct object_c
{
	int num_buckets;
	int locks[NUM_BUCKETS_C];
	unsigned int key_size_per_bucket[NUM_BUCKETS_C];
	unsigned int value_size_per_bucket[NUM_BUCKETS_C];
	unsigned int pairs_per_bucket[NUM_BUCKETS_C];
	struct intl buckets[NUM_BUCKETS_C];
	unsigned int memory_pool[MAX_POOL_C];
	unsigned int memory_offset[CPU_GLOBAL_THREADS/CPU_LOCAL_THREADS];
};

#endif