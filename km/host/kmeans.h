#ifndef KMEANS
#define KMEANS
#define DIM 3
#define	K 40
#define BSIZE  20000000 //2*1024*1024
struct kmeans_value
{
	float dim0;
	float dim1;
	float dim2;
	float num;
	float dist;
};
#endif