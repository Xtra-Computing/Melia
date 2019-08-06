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
/*	__global float *mat1 = (__global float *)global_data;
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
	return true;*/
	__global float *matrix = (__global float *)global_data;
	float up = 0.0f;
	float down = 0.0f;
	float result = 0.0f;

	struct pos_t p = *(__global struct pos_t *)offset;
	int doc1 = p.x;
	int doc2 = p.y;
	
	int M_COL_COUNT = DIM;

	__global float *a = matrix+doc1*M_COL_COUNT;

	__global float *b = matrix+doc2*M_COL_COUNT;

	float doc1Down = 0.0f;
	float doc2Down = 0.0f;

	for (int i = 0; i < M_COL_COUNT; i++)
	{
		float a1Value = a[i];
		float b1Value = b[i];
		up += (a1Value * b1Value);
		doc1Down += (a1Value*a1Value);
		doc2Down += (b1Value*b1Value);
	}
      
	down = sqrt(doc1Down*doc2Down);//down = sqrt(doc1Down)*sqrt(doc2Down);
	result = up / down;

	int key = doc1* DIM + doc2;
	cinsert_from_private(object,  &key, sizeof(int), &result,sizeof(float));

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
