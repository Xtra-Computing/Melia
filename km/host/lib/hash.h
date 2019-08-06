#ifndef HASH
#define HASH

#include "util_device.h"
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

unsigned int hash(void *key, unsigned short size)
{
	 unsigned short hs = 5381;
     char *str = (char *)key;

     for(int i = 0; i<size; i++)
     {
          hs = ((hs << 5) + hs) + ((int)str[i]); /* hash * 33 + c */
     }

     return hs;
}

unsigned int hash_local(__local void *key, unsigned short size)
{
	 unsigned short hs = 5381;
     __local char *str = (__local char *)key;

     for(int i = 0; i<size; i++)
     {
          hs = ((hs << 5) + hs) + ((int)str[i]); /* hash * 33 + c */
     }

     return hs;
}

unsigned int hash_global(__global void *key, unsigned short size)
{
	 unsigned short hs = 5381;
     __global char *str = (__global char *)key;

     for(int i = 0; i<size; i++)
     {
          hs = ((hs << 5) + hs) + ((int)str[i]); /* hash * 33 + c */
     }

     return hs;
}

#endif