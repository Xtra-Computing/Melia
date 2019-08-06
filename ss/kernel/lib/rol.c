#include "rol.h"

void loma_init(Reduction_Object_S *object)
{
}

int lomalloc(Reduction_Object_S *object, unsigned int size)
{
	return 0;
}

//bool insert(Reduction_Object_S *object, void *key, unsigned short key_size, void *value, unsigned short value_size)
//{
//}

void * lget_address(Reduction_Object_S *object, unsigned memory_index)
{
	return 0;
}

void * lget_key_address(Reduction_Object_S *object, unsigned bucket_index)
{
	return 0;
}

unsigned short lget_key_size(Reduction_Object_S *object, unsigned bucket_index)
{
	return 0;
}

void * lget_value_address(Reduction_Object_S *object, unsigned bucket_index)
{
	return 0;
}

unsigned short lget_value_size(Reduction_Object_S *object, unsigned bucket_index)
{
	return 0;
}