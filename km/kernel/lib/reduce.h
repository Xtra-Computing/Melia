#ifndef REDUCE
#define REDUCE
bool equal_private_and_local(void *key1, unsigned size1, __local void *key2, unsigned size2)
{
 /*     bool flag1 = true, flag2 = true;
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
	return flag1 &  flag2;*/

     bool flag = ( *(int *)key1 == *(__local int *)key2 ); 

     return flag;
	
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
	return flag1 &  flag2;
}
	
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
	return flag1 &  flag2;
}

equal_global_and_local(__global void *key1, unsigned size1, __local void *key2, unsigned size2)
{
	if(size1!=size2)
		return false;

	__global char *k1 = (__global char *)key1;
	  __local char *k2 = (__local char *)key2;
	
	for(int i = 0; i< size1; i++)
	{
		if(k1[i]!=k2[i])
			return false;
	}

	//printf("equal 2...\n");
	return true;
}
	
bool equal_global_and_global(__global void *key1, unsigned size1, __global void *key2, unsigned size2)
{
	if(size1!=size2)
		return false;

	__global char *k1 = (__global char *)key1;
	__global char *k2 = (__global char *)key2;
	
	for(int i = 0; i< size1; i++)
	{
		if(k1[i]!=k2[i])
			return false;
	}

	//printf("equal 2...\n");
	return true;
}


bool equal_local_and_global(__local void *key1, unsigned size1, __global void *key2, unsigned size2)
{
/*      bool flag1 = true, flag2 = true;
	if(size1!=size2)
		flag1 = false;

	  __local char *k1 =   (__local char *)key1;
	__global char *k2 = (__global char *)key2;
	
	for(int i = 0; i< size1; i++)
	{
		if(k1[i]!=k2[i])
			flag2 = false;
	}

	//printf("equal 2...\n");
	return flag1 &  flag2;
*/
	    bool flag = ( *(__local int *)key1 == *(__global int *)key2 ); 
           return flag;
}

void reduce_local_to_global(__global void *value1, unsigned short value1_size, __local void *value2, unsigned short value2_size)
{
	//printf("reducing .......********\n");
	
	float num_points1 = ((__global float *)value1)[3];
	float num_points2 = ((__local float *)value2)[3];
	float dist1 = ((__global float *)value1)[4];
	float dist2 = ((__local float *)value2)[4];
	float total_points = num_points1 + num_points2;
	float temp[5];
	temp[0] = ((__global float *)value1)[0] + ((__local float *)value2)[0];
	temp[1] = ((__global float *)value1)[1] + ((__local float *)value2)[1];
	temp[2] = ((__global float *)value1)[2] + ((__local float *)value2)[2];
	temp[3] = total_points;
	temp[4] = dist1 + dist2;
	
	copyVal_private_to_global(value1, temp, sizeof(float)*5);
}

void reduce_private_to_global(__global void *value1, unsigned short value1_size, void *value2, unsigned short value2_size)
{
	unsigned int num_points1 = ((__global float *)value1)[3];
	unsigned int num_points2 = ((float *)value2)[3];
	float dist1 = ((__global float *)value1)[4];
	float dist2 = ((float *)value2)[4];
	unsigned int total_points = num_points1 + num_points2;
	float temp[5];
	temp[0] = ((__global float *)value1)[0] + ((float *)value2)[0];
	temp[1] = ((__global float *)value1)[1] + ((float *)value2)[1];
	temp[2] = ((__global float *)value1)[2] + ((float *)value2)[2];
	temp[3] = total_points;
	temp[4] = dist1 + dist2;

	copyVal_private_to_global(value1, temp, sizeof(float)*5);
}



void reduce_private_to_local(__local void *value1, unsigned short value1_size, void *value2, unsigned short value2_size)
{
	float num_points1 = ((__local float *)value1)[3];
	float num_points2 = ((float *)value2)[3];
	float dist1 = ((__local float *)value1)[4];
	float dist2 = ((float *)value2)[4];
	float total_points = num_points1 + num_points2;
	float temp[5];
	temp[0] = ((__local float *)value1)[0] + ((float *)value2)[0];
	temp[1] = ((__local float *)value1)[1] + ((float *)value2)[1];
	temp[2] = ((__local float *)value1)[2] + ((float *)value2)[2];
	temp[3] = total_points;
	temp[4] = dist1 + dist2;
	copyVal_private_to_local(value1, temp, sizeof(float)*5);
}




void reduce_private_to_cpu(__global void *value1, unsigned short value1_size, void *value2, unsigned short value2_size)
{
	float num_points1 = ((__global float *)value1)[3];
	float num_points2 = ((float *)value2)[3];
	float dist1 = ((__global float *)value1)[4];
	float dist2 = ((float *)value2)[4];
	float total_points = num_points1 + num_points2;
	float temp[5];
	temp[0] = ((__global float *)value1)[0] + ((float *)value2)[0];
	temp[1] = ((__global float *)value1)[1] + ((float *)value2)[1];
	temp[2] = ((__global float *)value1)[2] + ((float *)value2)[2];
	temp[3] = total_points;
	temp[4] = dist1 + dist2;

	copyVal_private_to_global(value1, temp, sizeof(float)*5);
}

void reduce_global_to_cpu(__global void *value1, unsigned short value1_size, __global void *value2, unsigned short value2_size)
{
	unsigned int num_points1 = ((__global float *)value1)[3];
	unsigned int num_points2 = ((__global float *)value2)[3];
	float dist1 = ((__global float *)value1)[4];
	float dist2 = ((__global float *)value2)[4];
	unsigned int total_points = num_points1 + num_points2;
	float temp[5];
	temp[0] = ((__global float *)value1)[0] + ((__global float *)value2)[0];
	temp[1] = ((__global float *)value1)[1] + ((__global float *)value2)[1];
	temp[2] = ((__global float *)value1)[2] + ((__global float *)value2)[2];
	temp[3] = total_points;
	temp[4] = dist1 + dist2;

	copyVal_private_to_global(value1, temp, sizeof(float)*5);
}

#endif
