#include "error_handling.h"
#include "output.h"
#include <cstdio>
#include <cstdlib>
#include "scheduler.h"
#include <iostream>
#include <fstream>
#include "MyThreadPoolCop.h"
#define __NO_STD_STRING
//#include "common.h"
//extern int tony;
#define AOCL_ALIGNMENT 64
int convertToString(const char *filename, std::string& s)
{
    size_t size;
    char*  str;

    std::fstream f(filename, (std::fstream::in | std::fstream::binary));

    if(f.is_open())
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);

        str = new char[size+1];
        if(!str)
        {
            f.close();
            return NULL;
        }

        f.read(str, fileSize);
        f.close();
        str[size] = '\0';
    
        s = str;
        delete[] str;
        return 0;
    }
    printf("Error: Failed to open file %s\n", filename);
    return 1;
}
Scheduler::Scheduler(void *global_data, 
		unsigned int global_data_size,
		void *global_data_offset,
		unsigned int global_data_offset_number,
		unsigned int offset_unit_size)
{
	this->global_data = global_data;
	this->global_data_size = global_data_size;
	this->global_offset = global_data_offset;
	this->offset_number = global_data_offset_number;
	this->unit_size = offset_unit_size;

	cl_int err;
	std::vector<cl::Platform> platforms;
    std::cout<<"Getting Platform Information\n";
    err = cl::Platform::get(&platforms);
    if(err != CL_SUCCESS)
    {
        printf("%s\n",print_cl_errstring(err));
        return;
    }

	std::cout<<"Number of platforms found: "<<platforms.size()<<std::endl;

    if(err != CL_SUCCESS)
    {
        	printf("%s\n",print_cl_errstring(err));
        	return;
	}	

	platform = *platforms.begin();//get the first platform

	cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(), 0 };

	std::cout<<"Creating a context FPGA platform\n";
    	context = cl::Context(CL_DEVICE_TYPE_ALL, cps, NULL, NULL, &err);
    	if(err != CL_SUCCESS)
    	{
        	printf("%s\n",print_cl_errstring(err));
        	return;
	}

	std::cout<<"Getting device info\n";
    	devices = context.getInfo<CL_CONTEXT_DEVICES>();

	std::cout<<"Num of devices: "<<devices.size()<<std::endl;

	cl_device_type type;
	devices[0].getInfo(CL_DEVICE_TYPE, &type);

	if(type==CL_DEVICE_TYPE_GPU)
		gpuqueue = cl::CommandQueue(context, devices[1], 0); 
	else
		gpuqueue = cl::CommandQueue(context, devices[0], 0); 



	roghgpu = (Reduction_Object_G *)_aligned_malloc(sizeof(Reduction_Object_G),AOCL_ALIGNMENT); 
	memset(roghgpu, 0, sizeof(Reduction_Object_G));
	roghgpu->num_buckets = NUM_BUCKETS_G;

	{
/*	   *(int *)(&(roghgpu->align_bucket[0])) = 0x11223344; //.key_size_per_bucket 
	  
	   printf(" key = 0x%x, val = 0x%x\n",(roghgpu->align_bucket[0].key_size_per_bucket), (roghgpu->align_bucket[0].value_size_per_bucket) );
	   printf("addr key = 0x%x, val = 0x%x\n",&(roghgpu->align_bucket[0].key_size_per_bucket), &(roghgpu->align_bucket[0].value_size_per_bucket) );
	   printf("addr pairs  = 0x%x, k = 0x%x\n",&(roghgpu->align_bucket[0].pairs_per_bucket), &(roghgpu->align_bucket[0].buckets.x) );

	   memset(roghgpu, 0, sizeof(Reduction_Object_G));
*/
	}

	global_data_d =	cl::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR, global_data_size, global_data, &err); 

	global_offset_d = cl::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR, offset_number*unit_size, global_offset, &err); 

	//roccpu = cl::Buffer(context, CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR, sizeof(Reduction_Object_C), rochcpu, &err);

	roggpu = cl::Buffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, sizeof(Reduction_Object_G), roghgpu, &err);

	if(err != CL_SUCCESS)
    {
        	printf("%s\n",print_cl_errstring(err));
        	return;
	}

	std::cout<<"Loading and compiling CL source: kernel.aocx...\n";

//sm_atomic_add_copy_sim_share_sim_private_burst_g_burst_new2_correct_equal_213
//sm_atomic_add_copy_sim_share_sim_private_burst_g_burst_new2_ce_197 sm_atomic_add_copy_sim_share_sim_private_burst_g_burst_2_new2_ce_207
//sm_atomic_add_copy_sim_share_sim_private_burst_int_g_burst_new2_ce_221 sm_atomic_add_copy_sim_share_sim_private_burst_int_g_burst_2_new2_ce_197
//sm_atomic_add_copy_sim_share_sim_private_g_burst_2_new2_ce_197_2

/*final test cases*/
//sm_atomic_add_copy_sim_share_sim_private_g_burst_new2_ce_212 sm_atomic_add_copy_sim_share_sim_private_g_burst_2_new2_ce_184

//sm_atomic_add_copy_sim_share_sim_private_g_burst_new1_ce_218 sm_atomic_add_copy_sim_share_sim_private_g_burst_2_new1_ce_184
//sm_atomic_add_copy_sim_share_sim_private_g_burst_3_new2_ce_148_2 sm_atomic_add_copy_sim_share_sim_private_g_burst_3_new2_ce_148_2
	//kernel  :::for no locking

  FILE* fp = fopen("sm_atomic_add_copy_sim_share_sim_private_g_burst_3_new2_ce_148_2.aocx", "rb");
  FILE* fp1 = fopen("get_size_gpu.aocx", "rb"); //vectorAdd
  FILE* fp2 = fopen("copy_to_array_gpu.aocx", "rb"); //vectorAdd
  if (fp == NULL||fp1 == NULL||fp2 == NULL) {
    printf("Failed to open aocx file (fopen).\n");
  }
  fseek(fp, 0, SEEK_END); fseek(fp1, 0, SEEK_END); fseek(fp2, 0, SEEK_END);
  size_t binary_length = ftell(fp); size_t binary_length1 = ftell(fp1); size_t binary_length2 = ftell(fp2);
  unsigned char*binary = (unsigned char*) malloc(sizeof(unsigned char) * binary_length);
  unsigned char*binary1 = (unsigned char*) malloc(sizeof(unsigned char) * binary_length1);
  unsigned char*binary2 = (unsigned char*) malloc(sizeof(unsigned char) * binary_length2);
  rewind(fp);  rewind(fp1);  rewind(fp2);
  if (fread((void*)binary, binary_length, 1, fp) == 0) {
    printf("Failed to read from moving_average.aocx file (fread).\n");
	return;
  }
  if (fread((void*)binary1, binary_length1, 1, fp1) == 0) {
    printf("Failed to read from moving_average.aocx file (fread).\n");
	return;
  }
  if (fread((void*)binary2, binary_length2, 1, fp2) == 0) {
    printf("Failed to read from moving_average.aocx file (fread).\n");
	return;
  }
  fclose(fp);  fclose(fp1);  fclose(fp2);
  cl_context ctx=context.object_;
  cl_device_id id=devices.front().object_;
  cl_int kernel_status;
  cl_int status;
  program = clCreateProgramWithBinary(ctx, 1,&id , &binary_length, (const unsigned char**)&binary, &kernel_status, &status);
  program1 = clCreateProgramWithBinary(ctx, 1,&id , &binary_length1, (const unsigned char**)&binary1, &kernel_status, &status);
  program2 = clCreateProgramWithBinary(ctx, 1,&id , &binary_length2, (const unsigned char**)&binary2, &kernel_status, &status);
  if(status != CL_SUCCESS || kernel_status != CL_SUCCESS) {
//    dump_error("Failed clCreateProgramWithBinary.", status);
//    freeResources();
//    return 1;
  }
 
    err = program.build(devices, "-I . -I lib");
	err |= program1.build(devices, "-I . -I lib");
	err |= program2.build(devices, "-I . -I lib");
    if (err != CL_SUCCESS) 
	{

			std::string str = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
			std::cout << " \n\t\t\tBUILD LOG\n";
			std::cout << " ************************************************\n";
			std::cout << str.c_str() << std::endl;
            std::cout << " ************************************************\n";
       
        	std::cerr << "Program::build() failed (" << err << ")\n";
			printf("%s\n",print_cl_errstring(err));
        	return;
    }
}

void Scheduler::do_mapreduce()
{
	usetest();
	get_size_gpu();
	get_result_gpu();

}

void Scheduler::usetest()
{

	MyThreadPoolCop *pool=(MyThreadPoolCop*)malloc(sizeof(MyThreadPoolCop));
	pool->create(1);
	pool->assignParameter(0, this);
	pool->assignTask(0, test);
		
	pool->run();
	pool->destory();

	printf("test finish...\n");

}

DWORD WINAPI Scheduler::test(void *arg)
{
	Scheduler *scheduler = reinterpret_cast<Scheduler*>(arg);

	cl_int err;
	cl::Kernel kernel(scheduler->program, "test", &err);

	cl_uint karg=0;
	err = kernel.setArg(karg++, scheduler->global_offset_d);
	if (err != CL_SUCCESS) 
		printf("%s\n",print_cl_errstring(err));
	err = kernel.setArg(karg++, scheduler->offset_number);
	if (err != CL_SUCCESS) 
		printf("%s\n",print_cl_errstring(err));
	err = kernel.setArg(karg++, scheduler->unit_size);
	if (err != CL_SUCCESS) 
		printf("%s\n",print_cl_errstring(err));
	err = kernel.setArg(karg++, scheduler->global_data_d);
	if (err != CL_SUCCESS) 
		printf("%s\n",print_cl_errstring(err));
	err = kernel.setArg(karg++, scheduler->roggpu);
	if (err != CL_SUCCESS) 
		printf("%s\n",print_cl_errstring(err));
		
	printf("GPU queue enqueuing kernel...\n");
	clock_t beforequeue = clock();
	err = scheduler->gpuqueue.enqueueNDRangeKernel(kernel, cl::NullRange, 
	cl::NDRange(GPU_GLOBAL_THREADS), cl::NDRange(GPU_LOCAL_THREADS), 0, 0);
	

	if (err != CL_SUCCESS) 
	{
    	printf("%s\n",print_cl_errstring(err));
	}

	
	scheduler->gpuqueue.finish();
	clock_t afterqueue = clock();
	double timeinsec=(double)(afterqueue-beforequeue)/(double)CLOCKS_PER_SEC;
	printf("FPGA time: %0.4f \n", timeinsec);
	
	#if 1  //DATA_PRINT     Reduction_Object_G), roghgpu
		{ 
			int i;
		    int err = scheduler->gpuqueue.enqueueReadBuffer(scheduler->roggpu, true, 0, sizeof(Reduction_Object_G), scheduler->roghgpu, 0, 0);
	        scheduler->gpuqueue.finish();
/*
			for (i=0; i<NUM_BUCKETS_C;i++)
			{ 
				if (( (scheduler->rochcpu)->key_size_per_bucket)[i] != 0 )
		          printf("[%d] : %d, %d, %d\n", i,  ((scheduler->rochcpu)->key_size_per_bucket)[i], ((scheduler->rochcpu)->value_size_per_bucket)[i],((scheduler->rochcpu)->pairs_per_bucket)[i] );
			}
*/

			for (i=0; i<54;i++)
			{ 
		          printf("0x%x\t", ((scheduler->roghgpu)->memory_pool)[i] );
			}

			printf("num_buckets[%d] : %d\n", i, ((scheduler->roghgpu)->num_buckets) );
		}
#endif


 return 0;
}
void Scheduler::get_size_gpu()
{
	cl_int err;
	cl::Kernel kernel(program1, "get_size_gpu", &err);
	if(err != CL_SUCCESS)
    {
        	printf("%s\n",print_cl_errstring(err));
        	return;
	}

	key_num_d = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(unsigned int), NULL, &err);
	key_size_d = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(unsigned int), NULL, &err);
	value_size_d = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(unsigned int), NULL, &err);

	kernel.setArg(0, roggpu);
	kernel.setArg(1, key_num_d);
	kernel.setArg(2, key_size_d);
	kernel.setArg(3, value_size_d);

	gpuqueue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(CPU_GLOBAL_THREADS), cl::NDRange(CPU_LOCAL_THREADS, 0, NULL));

	gpuqueue.finish();

	err = gpuqueue.enqueueReadBuffer(key_num_d, true, 0, sizeof(unsigned int), &key_num, 0, 0);
	err = gpuqueue.enqueueReadBuffer(key_size_d, true, 0, sizeof(unsigned int), &key_size, 0, 0);
	err = gpuqueue.enqueueReadBuffer(value_size_d, true, 0, sizeof(unsigned int), &value_size, 0, 0);

	value_num = key_num;

	if (err != CL_SUCCESS) 
	{
        std::cerr << "CommandQueue::enqueueReadBuffer() failed (" << err << ")\n";
		return;
    }
	std::cout<<"The num of keys: "<<key_num<<std::endl;
	std::cout<<"The size of keys: "<<key_size<<std::endl;
	std::cout<<"The size of values: "<<value_size<<std::endl;

	std::cout<<"The number of buckets: "<<roghgpu->num_buckets<<std::endl;
}

void Scheduler::get_result_gpu()
{
	cl_int err;
	cl::Kernel kernel(program2, "copy_to_array_gpu", &err);
	if(err != CL_SUCCESS)
    {
        	printf("%s\n",print_cl_errstring(err));
        	return;
	}

	cl::Buffer key_array_d = cl::Buffer(context, CL_MEM_READ_WRITE, key_size, NULL, &err);
	cl::Buffer value_array_d = cl::Buffer(context, CL_MEM_READ_WRITE, value_size, NULL, &err);
	cl::Buffer key_index_d = cl::Buffer(context, CL_MEM_READ_WRITE, key_num*sizeof(unsigned int), NULL, &err);
	cl::Buffer value_index_d = cl::Buffer(context, CL_MEM_READ_WRITE, value_num*sizeof(unsigned int), NULL, &err);

	//set kernel args
	kernel.setArg(0, roggpu);
	kernel.setArg(1, key_array_d);
	kernel.setArg(2, value_array_d);
	kernel.setArg(3, key_index_d);
	kernel.setArg(4, value_index_d);

	gpuqueue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(GPU_GLOBAL_THREADS), cl::NDRange(GPU_LOCAL_THREADS, 0, NULL));
	gpuqueue.finish();

	char *key_array = (char *)_aligned_malloc(key_size,AOCL_ALIGNMENT); 
	char *value_array = (char *)_aligned_malloc(value_size,AOCL_ALIGNMENT); 
	unsigned int *key_index = (unsigned int *)_aligned_malloc(sizeof(unsigned int)*key_num,AOCL_ALIGNMENT); 
	unsigned int *value_index = (unsigned int *)_aligned_malloc(sizeof(unsigned int)*value_num,AOCL_ALIGNMENT); 

	gpuqueue.enqueueReadBuffer(key_array_d, true, 0, key_size, key_array, 0, 0);
	gpuqueue.enqueueReadBuffer(value_array_d, true, 0, value_size, value_array, 0, 0);
	gpuqueue.enqueueReadBuffer(key_index_d, true, 0, sizeof(unsigned int)*key_num, key_index, 0, 0);
	gpuqueue.enqueueReadBuffer(value_index_d, true, 0, sizeof(unsigned int)*value_num, value_index, 0, 0);

	std::cout<<"The number of keys: "<<key_num<<std::endl;

	output.key_index = key_index;
	output.val_index = value_index;
	output.output_keys = key_array;
	output.output_vals = value_array;
}

unsigned int Scheduler::get_key_num()
{
	return key_num;
}

struct output Scheduler::get_output()
{
	return output;
}
void Scheduler::freeResource(){
	_aligned_free(roghgpu);
	_aligned_free(output.key_index);
	_aligned_free(output.val_index);
	_aligned_free(output.output_keys);
	_aligned_free(output.output_vals);
	
}