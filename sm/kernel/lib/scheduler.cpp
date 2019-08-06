#include <cstdio>
#include <cstdlib>
#include <iostream>
#define __NO_STD_STRING
#include <CL/cl.hpp>
#include "scheduler.h"
#include "util_host.h"
#include <time.h>
#include "ds.h"
#include "error_handling.h"
#include "worker_info.h"
#include "MyThreadPoolCop.h"
#include "common.h"
//#include <pthread.h>
extern int tony;
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
		gpuqueue = cl::CommandQueue(context, devices[0], 0); 
	else
		gpuqueue = cl::CommandQueue(context, devices[1], 0); 



	roghgpu = (Reduction_Object_G *)malloc(sizeof(Reduction_Object_G));
	memset(roghgpu, 0, sizeof(Reduction_Object_G));
	roghgpu->num_buckets = NUM_BUCKETS_G;


	global_data_d =	cl::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR, global_data_size, global_data, &err); 

	global_offset_d = cl::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR, offset_number*unit_size, global_offset, &err); 

	//roccpu = cl::Buffer(context, CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR, sizeof(Reduction_Object_C), rochcpu, &err);

	roggpu = cl::Buffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, sizeof(Reduction_Object_G), roghgpu, &err);

	if(err != CL_SUCCESS)
    {
        	printf("%s\n",print_cl_errstring(err));
        	return;
	}

	std::cout<<"Loading and compiling CL source: kernel.cl...\n";

	std::string  sourceStr;
    convertToString("kernel.cl", sourceStr);

	const char * source    = sourceStr.c_str();
    size_t sourceSize[]    = { strlen(source) };
	
	//cl::Program::Sources sources_start(1, std::make_pair(sdkfile.source().c_str(), sdkfile.source().size()));

	program = cl::Program(context, source, &err);

	if (err != CL_SUCCESS) 
	{
        	printf("%s\n",print_cl_errstring(err));
    }

    err = program.build(devices, "-I . -I lib");
    if (err != CL_SUCCESS) 
	{

			cl::string str = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
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
	printf("GPU time: %0.2f \n", timeinsec);
	
 return 0;
}
void Scheduler::get_size_gpu()
{
	cl_int err;
	cl::Kernel kernel(program, "get_size_gpu", &err);
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
	cl::Kernel kernel(program, "copy_to_array_gpu", &err);
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

	char *key_array = (char *)malloc(key_size); 
	char *value_array = (char *)malloc(value_size);
	unsigned int *key_index = (unsigned int *)malloc(sizeof(unsigned int)*key_num);
	unsigned int *value_index = (unsigned int *)malloc(sizeof(unsigned int)*value_num);

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