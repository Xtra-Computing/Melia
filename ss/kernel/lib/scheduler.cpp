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

	//std::cout<<"in the scheduler..."<<std::endl;
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

	std::vector<cl::Platform>::iterator i;
   	if(platforms.size() > 0)
    {
        for(i = platforms.begin(); i != platforms.end(); ++i)
        {
            	std::cout<<(*i).getInfo<CL_PLATFORM_VENDOR>(&err).c_str()<<std::endl;
			
				if(!strcmp((*i).getInfo<CL_PLATFORM_VENDOR>(&err).c_str(), "Advanced Micro Devices, Inc."))
            	{
                	break;
            	}
        }
    }

    if(err != CL_SUCCESS)
    {
        	printf("%s\n",print_cl_errstring(err));
        	return;
	}	

	platform = *i;

	cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(), 0 };

	std::cout<<"Creating a context AMD platform\n";
    	context = cl::Context(CL_DEVICE_TYPE_CPU|CL_DEVICE_TYPE_GPU, cps, NULL, NULL, &err);
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

	if(type==CL_DEVICE_TYPE_CPU)
	{
	std::cout<<"first device is cpu"<<std::endl;
	cpuqueue = cl::CommandQueue(context, devices[0], 0); 
//	gpuqueue = cl::CommandQueue(context, devices[1], 0);
	}

	else if(type==CL_DEVICE_TYPE_GPU)
	{
//	std::cout<<"first device is gpu"<<std::endl;
//	gpuqueue = cl::CommandQueue(context, devices[0], 0); 
	cpuqueue = cl::CommandQueue(context, devices[1], 0);
	}

	rochcpu = (Reduction_Object_C *)malloc(sizeof(Reduction_Object_C));
	memset(rochcpu, 0, sizeof(Reduction_Object_C));
	rochcpu->num_buckets = NUM_BUCKETS_C;

/*	roghgpu = (Reduction_Object_G *)malloc(sizeof(Reduction_Object_G));
	memset(roghgpu, 0, sizeof(Reduction_Object_G));
	roghgpu->num_buckets = NUM_BUCKETS_G;
*/
	int num_blocks = GPU_GLOBAL_THREADS/GPU_LOCAL_THREADS + CPU_GLOBAL_THREADS/CPU_LOCAL_THREADS;
//	workersh = (Worker *)malloc(sizeof(Worker)*num_blocks);

	std::cout<<"the number of blocks is: "<<num_blocks<<std::endl;

/*	for(int i = 0; i < num_blocks; i++)
	{
		workersh[i].has_task = 0;
		workersh[i].task_num = 0;
		workersh[i].test_data = 0;
	}*/
	
//	workers = cl::Buffer(context, CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR, sizeof(Worker)*num_blocks, workersh, &err);

	global_data_d =	cl::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR, global_data_size, global_data, &err); 

	global_offset_d = cl::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR, offset_number*unit_size, global_offset, &err); 

	roccpu = cl::Buffer(context, CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR, sizeof(Reduction_Object_C), rochcpu, &err);

//	roggpu = cl::Buffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, sizeof(Reduction_Object_G), roghgpu, &err);

	if(err != CL_SUCCESS)
    {
        	printf("%s\n",print_cl_errstring(err));
        	return;
	}

	std::cout<<"Loading and compiling CL source: kernel.cl...\n";

	/*streamsdk::SDKFile sdkfile;

	if(!sdkfile.open("kernel.cl"))
	{
		std::cerr<<"We couldn't load CL source code\n";
        	return;
	}*/
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
	get_size_cpu();
	get_result_cpu();
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
	err = kernel.setArg(karg++, scheduler->roccpu);
	if (err != CL_SUCCESS) 
		printf("%s\n",print_cl_errstring(err));

	//bufferchecking_Reduction_Object_C(scheduler->roccpu,sizeof(Reduction_Object_C),scheduler->cpuqueue);

	printf("CPU queue enqueuing kernel...\n");
	clock_t beforequeue = clock();
	int cpu_global_threads=((scheduler->offset_number/CPU_LOCAL_THREADS+1)*CPU_LOCAL_THREADS);
	err = scheduler->cpuqueue.enqueueNDRangeKernel(kernel, cl::NullRange, 
		cl::NDRange(CPU_GLOBAL_THREADS), cl::NDRange(CPU_LOCAL_THREADS), 0, 0); 
	

	if (err != CL_SUCCESS) 
	{
    	printf("%s\n",print_cl_errstring(err));
	}
	
	scheduler->cpuqueue.finish();
	//bufferchecking_Reduction_Object_C(scheduler->roccpu,sizeof(Reduction_Object_C),scheduler->cpuqueue);
	clock_t afterqueue = clock();
	printf("CPU time: %d\n", afterqueue-beforequeue);
	
 return 0;
}

void Scheduler::get_size_cpu()
{
	printf("In the get size cpu\n");
	cl_int err;
	cl::Kernel kernel(program, "get_size_cpu", &err);
	if(err != CL_SUCCESS)
    {
        	printf("%s\n",print_cl_errstring(err));
        	return;
	}

	key_num_d = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(unsigned int), NULL, &err);
	if(err != CL_SUCCESS)
    {
        	printf("%s\n",print_cl_errstring(err));
        	return;
	}
	key_size_d = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(unsigned int), NULL, &err);
	if(err != CL_SUCCESS)
    {
        	printf("%s\n",print_cl_errstring(err));
        	return;
	}
	value_size_d = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(unsigned int), NULL, &err);
	if(err != CL_SUCCESS)
    {
        	printf("%s\n",print_cl_errstring(err));
        	return;
	}

	kernel.setArg(0, roccpu);
	kernel.setArg(1, key_num_d);
	kernel.setArg(2, key_size_d);
	kernel.setArg(3, value_size_d);

	cpuqueue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(CPU_GLOBAL_THREADS), cl::NDRange(CPU_LOCAL_THREADS, 0, NULL));

	cpuqueue.finish();

	err = cpuqueue.enqueueReadBuffer(key_num_d, true, 0, sizeof(unsigned int), &key_num, 0, 0);
	err = cpuqueue.enqueueReadBuffer(key_size_d, true, 0, sizeof(unsigned int), &key_size, 0, 0);
	err = cpuqueue.enqueueReadBuffer(value_size_d, true, 0, sizeof(unsigned int), &value_size, 0, 0);

	cpuqueue.finish();

	value_num = key_num;

	if (err != CL_SUCCESS) 
	{
        std::cerr << "CommandQueue::enqueueReadBuffer() failed (" << err << ")\n";
		return;
    }
	std::cout<<"The num of keys: "<<key_num<<std::endl;
	std::cout<<"The size of keys: "<<key_size<<std::endl;
	std::cout<<"The size of values: "<<value_size<<std::endl;

	std::cout<<"The number of buckets: "<<rochcpu->num_buckets<<std::endl;
}


void Scheduler::get_result_cpu()
{
	cl_int err;
	cl::Kernel kernel(program, "copy_to_array_cpu", &err);
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
	kernel.setArg(0, roccpu);
	kernel.setArg(1, key_array_d);
	kernel.setArg(2, value_array_d);
	kernel.setArg(3, key_index_d);
	kernel.setArg(4, value_index_d);

	cpuqueue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(CPU_GLOBAL_THREADS), cl::NDRange(CPU_LOCAL_THREADS, 0, NULL));
	cpuqueue.finish();

	char *key_array = (char *)malloc(key_size); 
	char *value_array = (char *)malloc(value_size);
	unsigned int *key_index = (unsigned int *)malloc(sizeof(unsigned int)*key_num);
	unsigned int *value_index = (unsigned int *)malloc(sizeof(unsigned int)*value_num);

	cpuqueue.enqueueReadBuffer(key_array_d, true, 0, key_size, key_array, 0, 0);
	cpuqueue.enqueueReadBuffer(value_array_d, true, 0, value_size, value_array, 0, 0);
	cpuqueue.enqueueReadBuffer(key_index_d, true, 0, sizeof(unsigned int)*key_num, key_index, 0, 0);
	cpuqueue.enqueueReadBuffer(value_index_d, true, 0, sizeof(unsigned int)*value_num, value_index, 0, 0);

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