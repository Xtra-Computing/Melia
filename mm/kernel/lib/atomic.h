#ifndef ATOMIC
#define ATOMIC

#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_amd_printf : enable

#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_amd_printf : enable

void GetSemaphor(__global int * semaphor) 
{
   int occupied = atomic_xchg(semaphor, 1);
   while(occupied > 0)
   {
     occupied = atomic_xchg(semaphor, 1);
   }
}
 
void ReleaseSemaphor(__global int * semaphor)
{
   int prevVal = atomic_xchg(semaphor, 0);
}

bool global_getlock(__global int *lock)
{
	//printf("Thread %d is trying to get the lock...\n", get_global_id(0));
	return atomic_cmpxchg(lock, 0, 1) == 0;
}

void global_releaselock(__global int *lock)
{
	//printf("Thread %d is releasing the lock...\n", get_global_id(0));
	atomic_xchg(lock, 0);
	//*lock = 0;
}

bool local_getlock(__local int *lock)
{
	//printf("Thread %d is trying to get the lock...\n", get_global_id(0));
	//printf("The lock value is: %d\n", *lock);
	return atomic_cmpxchg(lock, 0, 1) == 0;
}

void local_releaselock(__local int *lock)
{
	//printf("Thread %d is releasing the lock...\n", get_global_id(0));
	//printf("The lock value is: %d\n", *lock);
	atomic_xchg(lock, 0);
	//printf("The lock value after releasing is: %d\n", *lock);
	//*lock = 0;
}

#endif