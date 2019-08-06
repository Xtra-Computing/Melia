#ifndef MAP
#define MAP

#include "lib\util_device.h"
#include "lib\roc_f.h"
#include "lib\rog_f.h"
#include "lib\rol_f.h"
#include "lib\kvbuffer.h"

bool map_cpu(__global Reduction_Object_C *object, __global void *global_data, __global void *offset,int num_a)
{
	if(*(__global int *)offset < 0)
		return true;

	__global char *key = (__global char *)global_data + *(__global int *)offset;

	//printf("char: %c\n", *key);

	__global char *p = key;

	int key_size = 1;
	while(*p!='\0')
	{
		key_size++;
		p++;
	};

	int value = 1;

	//return ginsert_from_gp(object, key, key_size, &value, sizeof(int), global_object_offset);
	return cinsert_from_gp(object, key, key_size, &value, sizeof(int),num_a);
}

#if 0
bool map_local(__local Reduction_Object_S *object, __global void *global_data, 
	__global void *offset)
{
	//printf("offset is: %d\n", *(__global int *)offset);
/*	
	if(*(__global int *)offset < 0)
		return true;

	__global char *key = (__global char *)global_data + *(__global int *)offset;

	//printf("word: %c\n", *key);

	__global char *p = key;

	int key_size = 1;
	while(*p!='\0')
	{
		key_size++;
		p++;
	};

	int value = 1;
*/
	__global char *key = (__global char *)global_data + *(__global int *)offset;

	__global char *p = key;

	int key_size = 0;
	char p_result = 1;
	
	while(p_result != '\0')
	{
	       p_result = *p;
		key_size++;
		p++;
	};
	int value = 1;
	//return ginsert_from_private(object, key, key_size, &value, sizeof(float)*5, global_object_offset);

	return linsert_from_gp(object, key, key_size, &value, sizeof(int));
}
#endif
bool map_local(__local Reduction_Object_S *object, __global void *global_data, __local char *private_key,
	__global void *offset)
{
/*	__global char *key = (__global char *)global_data + *(__global int *)offset;

	__global char *p = key;
	
      char private_key[MAX_KEY_SIZE];
	   
      int key_size = 0;
      char result = 1;
      while (result != '\0')
     	{
		result                         = *p;
		private_key[key_size] = result;
		key_size++;
		p++;
	 }
	int value = 1;

	//return ginsert_from_gp(object, key, key_size, &value, sizeof(int), global_object_offset);
//	return cinsert_from_gp(object, key, key_size, &value, sizeof(int));
   return linsert_from_private(object, private_key, key_size, &value, sizeof(int));
*/
	//__global char *key_target=(__global char *)global_data;
	 __local char *key_target=( __local char *)private_key;
	 
	__global char *key = (__global char *)global_data + *(__global int *)offset;
       __global char *key_base = key;

     
	int key_size = 0, flag = 1;
	char key_char = 1, key_target_char = 1;
	
       char private_key_base[MAX_KEY_SIZE];
	
	while( (key_char!= '\0') | ( key_target_char!= '\0' ) ) //one valid, compare
	{    
	       key_char           = *key_base; //key
	       key_target_char = *key_target;
	       
		if (key_char != key_target_char)   
		{
		       flag = 0;
			break;
		}		

	       private_key_base[key_size] = key_char;

		key_size++;
		key_base++; //key
		key_target++;
	}

	int value = 1;
	
     if (flag == 1)
	//return ginsert_from_private(object, key, key_size, &value, sizeof(float)*5, global_object_offset);
	 linsert_from_private(object, private_key_base, key_size, &value, sizeof(int));


}
	
#endif
