#include <iostream>
using namespace std;
#include "lib\scheduler.h"
#include "kmeans.h"
#include "lib\util_host.h"
#include <time.h>

#define GRIDSZ 1000
#define OFFSETS 100

int main()
{
	//Allocate space for all points and clusters. The first K points are cluster centers
	float *points = (float *)malloc(sizeof(float)*DIM*K + sizeof(float)*DIM*BSIZE);
	unsigned int *offsets = (unsigned int *)malloc(sizeof(unsigned int)*BSIZE); 

	float *h_Points = points + DIM*K;
	float *h_means = points;

	cout<<"Generating points..."<<endl;
	clock_t beforegen = clock();
	//Generate data
	srand(2006);
	for(int i = 0; i < BSIZE; i++) {
		offsets[i] = i*DIM + K * DIM;
		for(int j = 0; j < DIM; j++) {
			h_Points[i * DIM + j] =(float) (rand() % GRIDSZ);
			//cout<<"Point: "<< h_Points[i * DIM + j]<<" ";
		}
		//cout<<endl;
	}

	cout<<"Generating clusters..."<<endl;
	for(int i = 0; i < K; i++) {
		for(int j = 0; j < DIM; j++) {
			h_means[i * DIM + j] =(float)  (rand() % GRIDSZ);
		}
	}



{ 
  float *result;
  result = (float *)malloc(5*K*sizeof(float));
 
  for(int i = 0;i<5*K;i++)
	  result[i] = 0;

  int off = 0;

    for (int index = 0; index < BSIZE; index++, off+=3 )
  { 
	float dim1 = ((float *)h_Points)[off];
	float dim2 = (( float *)h_Points)[off+1];
	float dim3 = (( float *)h_Points)[off+2];

	//printf("%d %d %d\n", dim1, dim2, dim3);

	unsigned int key = 0;
	float min_dist = 65536*65, dist;

	for(int i = 0; i < K; i++)
	{
		dist = 0;
		float cluster_dim1 = (( float *)h_means)[DIM*i];//((__global int *)global_data)[DIM*i];
		float cluster_dim2 = (( float *)h_means)[DIM*i+1];//((__global int *)global_data)[DIM*i+1];
		float cluster_dim3 = (( float *)h_means)[DIM*i+2];//((__global int *)global_data)[DIM*i+2];

		dist =	(cluster_dim1-dim1)*(cluster_dim1-dim1)+
				(cluster_dim2-dim2)*(cluster_dim2-dim2)+
				(cluster_dim3-dim3)*(cluster_dim3-dim3);
		dist = sqrt(dist);
		if(dist < min_dist)
		{
			min_dist = dist;
			key = i;
		}
	 }

	float value[5];
	value[0] = dim1;
	value[1] = dim2;
	value[2] = dim3;
	value[3] = 1;
	value[4] = min_dist;

	result[key*5+0] = result[key*5+0] + value[0];
	result[key*5+1] = result[key*5+1] + value[1];
	result[key*5+2] = result[key*5+2] + value[2];
	result[key*5+3] = result[key*5+3] + 1;
	result[key*5+4] = result[key*5+4] + value[4];

  }

  for(int i = 0; i < K; i++)
   {
     printf("%d: 0x%x 0x%x 0x%x 0x%x 0x%x\n", i, ((int*)result)[i*5+0],((int*)result)[i*5+1],((int*)result)[i*5+2],((int*)result)[i*5+3],((int*)result)[i*5+4]);
   }
}

	clock_t aftergen = clock();
	cout<<"Data generation time: "<<(aftergen-beforegen)<<" ms"<<endl;

	Scheduler scheduler((void *)h_means, sizeof(int)*DIM*(BSIZE+K), offsets, BSIZE, sizeof(int));
	scheduler.do_mapreduce();

	struct output output = scheduler.get_output();

	int key_num = scheduler.get_key_num();
	int total_num = 0;
	for(int i = 0; i < key_num; i++)
	{
		char *key_address = output.output_keys + (output.key_index)[i];
		char *value_address = output.output_vals + (output.val_index)[i];
		struct kmeans_value value = *(struct kmeans_value *)value_address;
		float number = value.num;
		float dist = value.dist;
		cout<<*(int *)key_address<<": ";
		cout<<"Average point: ("<<value.dim0/number<<", "<<value.dim1/number
			<<", "<<value.dim2/number<<")";
		printf("\t Number of points: %d", (int)number);
		printf("\t Dist: %f", dist);
		total_num += (int)number;
		cout<<endl;
	}
	cout<<"total num of points: "<<total_num<<endl;

	free(points);
	free(offsets);

	int a;
	std::cout<<"Enter any number to continue..."<<std::endl;
	std::cin>>a;
}
