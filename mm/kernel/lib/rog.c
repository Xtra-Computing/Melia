#include "rog.h"

void goma_init (Reduction_Object_G *object)
{
	object->num_buckets = 9;
}

int gomalloc(Reduction_Object_G *object, unsigned int size)
{
	return 0;
}

//bool insert(Reduction_Object_G *object, void *key, unsigned short key_size, void *value, unsigned short value_size)
//{
//}

void * gget_address(Reduction_Object_G *object, unsigned memory_index)
{
	return 0;
}

void * gget_key_address(Reduction_Object_G *object, unsigned bucket_index)
{
	return 0;
}

unsigned short gget_key_size(Reduction_Object_G *object, unsigned bucket_index)
{
	return 0;
}

void * gget_value_address(Reduction_Object_G *object, unsigned bucket_index)
{
	return 0;
}

unsigned short gget_value_size(Reduction_Object_G *object, unsigned bucket_index)
{
	return 0;
}