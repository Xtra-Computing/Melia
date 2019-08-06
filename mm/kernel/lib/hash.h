#ifndef HASH
#define HASH

#include "util_device.h"
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

unsigned int hash(void *key, unsigned short size)
{
	unsigned int tmp;
	copyVal(&tmp, key, size);
	return tmp;
}

unsigned int hash_local(local void *key, unsigned short size)
{
	unsigned int tmp;
	copyVal_local_to_private(&tmp, key, size);
	return tmp;
}

unsigned int hash_global(__global void *key, unsigned short size)
{
	unsigned int tmp;
	copyVal_global_to_private(&tmp, key, size);
	return tmp;
}
#endif