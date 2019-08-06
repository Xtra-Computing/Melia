#ifndef MAP
#define MAP

#include "lib\util_device.h"
#include "lib\roc_f.h"
#include "lib\rog_f.h"
#include "lib\rol_f.h"
#include "lib\kvbuffer.h"
#include "mm.h"

bool map_cpu(__global Reduction_Object_C *object, __global void *global_data, __global void *offset)
{
	__global float *mat1 = (__global float *)global_data;
	__global float *mat2 = (__global float *)global_data+DIM*DIM;
	struct pos_t p = *(__global struct pos_t *)offset;
	__global float * a = mat1 + p.x*DIM;
	__global float * b = mat2;
	float value = 0;

	for(int j = 0; j<DIM; j++)
	{
		value += a[j]*b[j*DIM+p.y];
	}

	unsigned int key = p.x*DIM + p.y;
	cinsert_from_private(object, &key, sizeof(unsigned int), &value, sizeof(float));
	return true;
}

bool map_local(__local Reduction_Object_S *object, __global void *global_data, 
	__global void *offset)
{
		return true;
}

/*this map function maps inputs to the global reduction object*/
bool map_to_global(__global Reduction_Object_G *object, __global void *global_data, 
	__global void *offset, __local unsigned int *global_object_offset)
{
	__global float *mat1 = (__global float *)global_data;
	__global float *mat2 = (__global float *)global_data+DIM*DIM;
	struct pos_t p = *(__global struct pos_t *)offset;
	__global float * a = mat1 + p.x*DIM;
	__global float * b = mat2;
	float value = 0;

	for(int j = 0; j<DIM; j++)
	{
		value += a[j]*b[j*DIM+p.y];
	}

	unsigned int key = p.x*DIM + p.y;
	ginsert_from_private(object, &key, sizeof(unsigned int), &value, sizeof(float), global_object_offset);
	return true;
}

#endif