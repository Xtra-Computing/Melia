#ifndef CONFIGURATION
#define CONFIGURATION

#define NUM_BUCKETS_C 5000000    //number of buckets in a CPU ro

#define NUM_BUCKETS_G 5000000
#define NUM_BUCKETS_S 101
#define MAX_POOL_S 2048
#define MAX_POOL_G 16*1024*1024
#define MAX_POOL_C 16*1024*1024       //pool size in GPU ro
#define ALIGN_SIZE 4
#define KVINDEX_NUM 4096 
#define KV_POOL_SIZE  524288 
#define KVBUFFER_SIZE 32768			  //number of int space in device memory
#define TASK_BLOCK_SIZE 5000  //each time schedule one block to an idle thread block

#define KVBUFFERS_SIZE 4096           //4096 ints space in local memory

#define CPU_LOCAL_THREADS 256/2
#define CPU_GLOBAL_THREADS 512*2

#define GPU_LOCAL_THREADS 256
#define GPU_GLOBAL_THREADS 2048
#define NUM_BLOCKS 31
#define NUM_GROUPS 2
#define WAVEFRONT_SIZE 64
//#define USE_LOCAL

#define TYPE_GPU 0
#define TYPE_CPU 1
#endif
