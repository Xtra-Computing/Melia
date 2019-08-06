#ifndef REDUCTIONOBJECTG
#define REDUCTIONOBJECTG
#include "configuration.h"
#include "ds.h"

typedef struct bucket_info_g Bucket_Info_G;
struct bucket_info_g
{
	unsigned short key_size_per_bucket;
	unsigned short value_size_per_bucket;
	unsigned int    pairs_per_bucket;
	struct intl      buckets;
};

typedef struct object_g Reduction_Object_G;

struct object_g
{  
       Bucket_Info_G align_bucket[NUM_BUCKETS_G]; 
	                              int locks[NUM_BUCKETS_G];
/*	unsigned int key_size_per_bucket[NUM_BUCKETS_G];
	unsigned int value_size_per_bucket[NUM_BUCKETS_G];
	unsigned int pairs_per_bucket[NUM_BUCKETS_G];
	struct intl buckets[NUM_BUCKETS_G];
*/	unsigned int memory_pool[MAX_POOL_G];
	unsigned int memory_offset[16];
	              int num_buckets;
	
};
/*
struct object_g
{
	int num_buckets;
	int locks[NUM_BUCKETS_G];
	unsigned int key_size_per_bucket[NUM_BUCKETS_G];
	unsigned int value_size_per_bucket[NUM_BUCKETS_G];
	unsigned int pairs_per_bucket[NUM_BUCKETS_G];
	struct intl buckets[NUM_BUCKETS_G];
	unsigned int memory_pool[MAX_POOL_G];
	unsigned int memory_offset[16];
};
*/
#endif
