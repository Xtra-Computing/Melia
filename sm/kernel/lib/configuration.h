#ifndef CONFIGURATION
#define CONFIGURATION

#define NUM_BUCKETS_C 8192    //number of buckets in a CPU ro

#define NUM_BUCKETS_G 4096
#define NUM_BUCKETS_S 1024//1200
#define MAX_POOL_S 2048*4
#define MAX_POOL_G 4*1024*1024
#define MAX_POOL_C 4*1024*1024     //pool size in GPU ro
#define ALIGN_SIZE 4
#define KVINDEX_NUM 4096 
#define KV_POOL_SIZE  524288 
#define KVBUFFER_SIZE 32768			  //number of int space in device memory
//#define TASK_BLOCK_SIZE 5000000 //each time schedule one block to an idle thread block

#define KVBUFFERS_SIZE 4096           //4096 ints space in local memory

#define CPU_LOCAL_THREADS 1
#define CPU_GLOBAL_THREADS 3

#define GPU_LOCAL_THREADS 2
#define GPU_GLOBAL_THREADS 6
#define NUM_GROUPS 1
#define WAVEFRONT_SIZE 2
#define USE_LOCAL

#define TYPE_GPU 0
#define TYPE_CPU 1

#define MAX_KEY_SIZE 8


#endif
