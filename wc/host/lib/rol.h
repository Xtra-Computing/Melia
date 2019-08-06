#ifndef REDUCTIONOBJECTS
#define REDUCTIONOBJECTS
#include "configuration.h"
#include "ds.h"
typedef struct object_s Reduction_Object_S;

struct object_s
{
	int num_buckets;
	int locks[NUM_BUCKETS_S];
	struct ints buckets[NUM_BUCKETS_S];
	unsigned int memory_pool[MAX_POOL_S];
	unsigned int memory_offset;
};

#endif