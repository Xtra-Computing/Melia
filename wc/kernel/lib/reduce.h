#ifndef REDUCE
#define REDUCE

bool equal_private_and_cpu(void *key1, unsigned size1, __global void *key2, unsigned size2)
{
  bool flag1 = true, flag2 = true;
	if(size1!=size2)
		flag1 = false;
		
	char *k1 = (char *)key1;
	__global char *k2 = (__global char *)key2;
	
	for(int i = 0; i< size1; i++)
	{
		if(k1[i]!=k2[i])
			flag2 = false;
	}

	//printf("equal 2...\n");
	return flag1 & flag2;
}

bool equal_global_and_global(__global void *key1, unsigned short size1, __global void *key2, unsigned short size2)
{   bool flag1 = true, flag2 = true;

	if(size1!=size2)
		flag1 = false;

	__global char *k1 = (__global char *)key1;
	__global char *k2 = (__global char *)key2;
	
	for(short  i = 0; i< size1; i++)
	{
		if(k1[i]!=k2[i])
			flag2 = false;
	}

	//printf("equal 2...\n");
	return flag1 & flag2;
}

bool equal_private_and_local(void *key1, unsigned size1, __local void *key2, unsigned size2)
{
    bool flag1 = true, flag2 = true;

	if(size1!=size2)
		flag1 = false;
		
	char *k1 = (char *)key1;
	__local char *k2 = (__local char *)key2;
	
	for(int i = 0; i< size1; i++)
	{
      if(k1[i]!=k2[i])
	     flag2 = false;
	}

	//printf("equal 2...\n");
	return flag1 & flag2;
}

bool equal_private_and_global(void *key1, unsigned size1, __global void *key2, unsigned size2)
{
    bool flag1 = true, flag2 = true;

	if(size1!=size2)
		flag1 = false;
		
	char *k1 = (char *)key1;
	__global char *k2 = (__global char *)key2;
	
	for(int i = 0; i< size1; i++)
	{
      if(k1[i]!=k2[i])
	    flag2 = false;
	}

	//printf("equal 2...\n");
	return flag1 & flag2;
}

bool equal_global_and_local(__global void *key1, unsigned size1, __local void *key2, unsigned size2)
{
    bool flag1 = true, flag2 = true;

	if(size1!=size2)
		flag1 = false;

	//printf("equal 1\n");

	__global char *k1 = (__global char *)key1;
	__local char *k2 = (__local char *)key2;
	
	for(int i = 0; i< size1; i++)
	{
      if(k1[i]!=k2[i])
         flag2 = false;
	}

	//printf("equal 2...\n");
	return flag1 & flag2;
}

bool equal_local_and_global(__local void *key1, unsigned size1, __global void *key2, unsigned size2)
{
    bool flag1 = true, flag2 = true;
	if(size1!=size2)
		flag1 = false;
		
	//printf("equal 1\n");
	__local char *k1 = (__local char *)key1;
	__global char *k2 = (__global char *)key2;

	for(int i = 0; i< size1; i++)
	{
		if(k1[i]!=k2[i])
         flag2 = false;
	}

	return flag1 & flag2;
}

void reduce_private_to_cpu(__global void *value1, unsigned short value1_size, void *value2, unsigned short value2_size)
{
	int temp = *(__global int *)value1 + *(int *)value2;
	copyVal_private_to_global(value1, &temp, sizeof(int));
}

void reduce_global_to_cpu(__global void *value1, unsigned short value1_size, __global void *value2, unsigned short value2_size)
{
	int temp = *(__global int *)value1 + *(__global int *)value2;
	copyVal_private_to_global(value1, &temp, sizeof(int));
}

void reduce_private_to_local(__local void *value1, unsigned short value1_size, void *value2, unsigned short value2_size)
{
	int temp = *(__local int *)value1 + *(char *)value2;
	*(__local int *)value1 = temp;//copyVal_private_to_local(value1, &temp, sizeof(int));
}

void reduce_local_to_global(__global void *value1, unsigned short value1_size, __local void *value2, unsigned short value2_size)
{
	int temp = *(__global int *)value1 + *(__local int *)value2;
	*(__global int *)value1  = temp;//copyVal_private_to_global(value1, &temp, sizeof(int));
}

void reduce_private_to_global(__global void *value1, unsigned short value1_size, void *value2, unsigned short value2_size)
{
	int temp = *(__global int *)value1 + *(int *)value2;
	copyVal_private_to_global(value1, &temp, sizeof(int));
}

#endif
