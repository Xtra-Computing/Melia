#ifndef UTIL_DEVICE
#define UTIL_DEVICE

#define ALIGN_SIZE 4
#pragma OPENCL EXTENSION cl_amd_printf : enable
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

unsigned int align(unsigned int size)
{
	return (size+ALIGN_SIZE-1)&(~(ALIGN_SIZE-1));
}

void copyVal(void *dst, void *src, unsigned short size)
{
	char *d=(char*)dst; 
	const char *s = (const char *)src;
	for(unsigned short i = 0; i < size; i++)
		d[i] = s[i];
}

void copyVal_global_to_global(__global void *dst, __global void *src, unsigned short size)
{
	__global char *d=(__global char *)dst; 
	__global char *s = (__global char *)src;
	for(unsigned short i = 0; i < size; i++)
		d[i] = s[i];
}

void copyVal_global_to_local(__local void *dst, __global void *src, unsigned short size)
{
	__local char *d=(__local char *)dst; 
	__global char *s = (__global char *)src;
	for(unsigned short i = 0; i < size; i++)
		d[i] = s[i];
}

void copyVal_local_to_global(__global void *dst, __local void *src, unsigned short size)
{
	__global char *d=(__global char*)dst; 
	__local char *s = (__local char *)src;
	for(unsigned short i = 0; i < size; i++)
		d[i] = s[i];
}


void copyVal_private_to_local(__local void *dst, void *src, unsigned short size)
{
	__local char *d = (__local char *)dst;
	const char *s = (const char *)src;
	for(unsigned short i = 0; i < size; i++)
		d[i] = s[i];
}

void copyVal_local_to_private(void *dst, local void *src, unsigned short size)
{
	char *d = (char *)dst;
	__local const char *s = (__local const char *)src;
	for(unsigned short i = 0; i < size; i++)
		d[i] = s[i];
}

void copyVal_global_to_private(void *dst, __global void *src, unsigned short size)
{
	char *d = (char *)dst;
	__global const char *s = (__global const char *)src;
	for(unsigned short i = 0; i < size; i++)
		d[i] = s[i];
}

void copyVal_private_to_global(__global void *dst, void *src, unsigned short size)
{
	__global char *d = (__global char *)dst;
	const char *s = (const char *)src;
	for(unsigned short i = 0; i < size; i++)
		d[i] = s[i];
}

void copyVal_private_to_private(void *dst, void *src, unsigned short size)
{
	char *d = (char *)dst;
	const char *s = (const char *)src;
	for(unsigned short i = 0; i < size; i++)
		d[i] = s[i];
}

//void copyVal_local_to_private(void *dst, __local void *src, unsigned short size)
//{
//	char *d = (char*)dst;
//	__local char *s = (__local char *)src;
//	for(unsigned short i = 0; i < size; i++)
//		d[i] = s[i];
//}

#endif