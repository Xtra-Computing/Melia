/*reduction object for CPU*/
#ifndef REDUCTIONOBJECTC
#define REDUCTIONOBJECTC
#include "configuration.h"
#include "ds.h"

typedef struct object_c Reduction_Object_C;
/*
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
	//unsigned int *memory_offset;
};
*/
typedef struct bucket_info_c Bucket_Info_C;

struct bucket_info_c
{
	unsigned short key_size_per_bucket;
	unsigned short value_size_per_bucket;
	unsigned int    pairs_per_bucket;
	struct intl      buckets;
};
/*
union union_bucket
 {
  Bucket_Info_C bucket_data;
  uint4 bucket_data_int4;
  }; 
typedef union union_bucket union_bucket_c;
*/

struct object_c
{
	Bucket_Info_C align_bucket[NUM_BUCKETS_C]; 
	int locks[NUM_BUCKETS_C];
	unsigned int memory_pool[MAX_POOL_C];
	unsigned int memory_offset[NUM_BLOCKS];//[CPU_GLOBAL_THREADS/CPU_LOCAL_THREADS];
	int num_buckets;
	//unsigned int *memory_offset;
};

#endif
