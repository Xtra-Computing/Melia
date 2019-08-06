#include <iostream>
using namespace std;
#include "lib\scheduler.h"
#include "lib\util_host.h"
#include <time.h>
#include "lib\mm.h"

#define GRIDSZ 1000
#define OFFSETS 100

static float *GenMatrix(int M_ROW_COUNT, int M_COL_COUNT)
{
        float *matrix = (float*)malloc(sizeof(float)*M_ROW_COUNT*M_COL_COUNT);

        srand(10);
        for (int i = 0; i < M_ROW_COUNT; i++)
                for (int j = 0; j < M_COL_COUNT; j++)
                        matrix[i*M_COL_COUNT+j] = (float)(rand() % 100);

        return matrix;
}

void TranMatrix(float * m, int row)
{
        for(int i=0;i<row;i++)
		{
                for(int j=i+1;j<row;j++)
				{
                        float tmp=m[i*row+j];
                        m[i*row+j]=m[j*row+i];
                        m[j*row+i]=tmp;
                }
        }
}

int main()
{
	int dim = DIM;

	pos_t * input = new pos_t[dim*dim];
	for(int i = 0; i<dim; i++)
	{
		for(int j = 0; j<dim; j++)
		{
			input[i*dim+j].x = i;
			input[i*dim+j].y = j;
		}
	}

	float * matrixA = GenMatrix(dim, dim);
	float * matrixB = GenMatrix(dim, dim);
	

	float * matdata = (float *)malloc(sizeof(float)*dim*dim*2);
	memcpy(matdata, matrixA, sizeof(float)*dim*dim);
	float *ptr = matdata+dim*dim;
	memcpy(ptr, matrixB, sizeof(float)*dim*dim);

	cout<<"data size is: "<<sizeof(float)*dim*dim*2/1024<<"KB"<<endl;
/*
	{
		clock_t beforequeue = clock();
         float *matrixC = (float*)malloc(sizeof(float)*dim*dim);
		 for(int i = 0; i < dim; i++)
           for(int j = 0; j < dim; j++)
			  {	 
				 float sum = 0;
				 for (int k = 0; k < dim; k++)
                    sum += matrixA[i * dim + k] * matrixB[k * dim + j];

				 matrixC[i*dim + j] = sum;
		      }
		clock_t afterqueue = clock();
		double	time_in_seconds = (double)(afterqueue-beforequeue) / (double)CLOCKS_PER_SEC;
		printf("CPU time: %0.2f s\n", time_in_seconds);

		for (int i = 0; i< 10; i++)
			printf("%f\t",matrixC[i] );
	}
*/

	std::cout<<"Data loaded..."<<std::endl;
	Scheduler scheduler((void *)matdata, sizeof(float)*dim*dim*2, input, dim*dim, sizeof(pos_t));
	scheduler.do_mapreduce();

	struct output output = scheduler.get_output();

	int key_num = scheduler.get_key_num();

	char *output_keys = output.output_keys;
	char *output_vals = output.output_vals;
	unsigned int *key_index = output.key_index;
	unsigned int *val_index = output.val_index;

	int total = 0;

	cout<<"****************************************"<<endl;

	for(int m = 0; m<25; m++)
	{
		printf("key index: %d\n", key_index[m]);
		char *key_address = output_keys + key_index[m];
		char *val_address = output_vals + val_index[m];
		cout<<*(unsigned int *)key_address<<": "<<*(float *)val_address<<endl;
	}

	cout<<"****************************************"<<endl;

	free(matdata);
	scheduler.freeResource();
	delete[] input;
	int b = 0;
	std::cout<<"Enter any number to continue..."<<std::endl;
	std::cin>>b;
}
