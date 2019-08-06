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

	if(type==CL_DEVICE_TYPE_CPU)
		fpgaqueue = cl::CommandQueue(context, devices[1], 0); 
	else
		fpgaqueue = cl::CommandQueue(context, devices[0], 0); 



	rochcpu = (Reduction_Object_C *)_aligned_malloc(sizeof(Reduction_Object_C),AOCL_ALIGNMENT);
	memset(rochcpu, 0, sizeof(Reduction_Object_C));
	rochcpu->num_buckets = NUM_BUCKETS_C;
	//debug info
/*	{
	   printf("addr of rochcpu is 0x%x\n", (int)rochcpu);
	   rochcpu->bucket_data[1].key_size_per_bucket = 0x1122;
	   rochcpu->bucket_data[1].value_size_per_bucket = 0x3344;

	   printf("addr of rochcpu [1] is 0x%x\n", (int)(&(rochcpu->bucket_data[1])));

	   printf("data of rochcpu [1] is 0x%x\n", *(int *)(&(rochcpu->bucket_data[1])));

	   *(int *)(&(rochcpu->bucket_data[1])) = 0x55667788;
	   printf("new data of rochcpu [1] is 0x%x\n", *(int *)(&(rochcpu->bucket_data[1])));

	   printf("key val of rochcpu [1] is 0x%x 0x%x\n", rochcpu->bucket_data[1].key_size_per_bucket, rochcpu->bucket_data[1].value_size_per_bucket );

	}*/
	global_data_d =	cl::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR, global_data_size, global_data, &err); 

	global_offset_d = cl::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR, offset_number*unit_size, global_offset, &err); 

	roccpu = cl::Buffer(context, CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR, sizeof(Reduction_Object_C), rochcpu, &err);

//	roggpu = cl::Buffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, sizeof(Reduction_Object_G), roghgpu, &err);

	if(err != CL_SUCCESS)
    {
        	printf("%s\n",print_cl_errstring(err));
        	return;
	}

	std::cout<<"Loading and compiling CL source: kernel.aocx...\n";
/*
	std::string  sourceStr;
    convertToString("kernel.aocx", sourceStr);

	const char * source    = sourceStr.c_str();
    size_t sourceSize[]    = { strlen(source) };
	
	//cl::Program::Sources sources_start(1, std::make_pair(sdkfile.source().c_str(), sdkfile.source().size()));

	program = cl::Program(context, source, &err);

	if (err != CL_SUCCESS) 
	{
        	printf("%s\n",print_cl_errstring(err));
    }
*/


// mm_reg_v_cfg_v_v_unroll_8 mm_reg_v_cfg_v_v_2 mm_reg_v_cfg_v_v
//mm_reg_v_cfg_v_v_unroll_10

	
//MM_no_shared_memory_private_burst_210 MM_no_shared_memory_private_burst_2_217

//MM_no_shared_memory_private_burst_210 MM_no_shared_memory_private_burst_2_217
//MM_no_shared_memory_private_burst_10_2_184
//MM_no_shared_memory_private_burst_16_2_149
// MM_no_shared_memory_private_burst_25_default_195 MM_no_shared_memory_private_burst_25_default_1_195
//MM_no_shared_memory_private_burst_40_146  burst
// kernel MM_no_shared_memory_private_burst_32_175

//MM_no_shared_memory_private_burst_25_default_2_195 MM_no_shared_memory_private_burst_no_atomic_2_25


//MM_nsm_private_burst_25_no_mapreduce 
// MM_no_shared_memory_private_burst_no_atomic_2_25
	//kernel
	//
  FILE* fp = fopen("MM_no_shared_memory_private_burst_no_atomic_25.aocx", "rb"); 
  FILE* fp1 = fopen("get_size_cpu.aocx", "rb"); //vectorAdd
  FILE* fp2 = fopen("copy_to_array_cpu.aocx", "rb"); //vectorAdd
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
//	return -1;
  }
  if (fread((void*)binary1, binary_length1, 1, fp1) == 0) {
    printf("Failed to read from moving_average.aocx file (fread).\n");
//	return -1;
  }
  if (fread((void*)binary2, binary_length2, 1, fp2) == 0) {
    printf("Failed to read from moving_average.aocx file (fread).\n");
//	return -1;
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
	get_size_cpu();
	get_result_cpu();

}
void Scheduler::usetest()
{

	MyThreadPoolCop *pool=(MyThreadPoolCop*)malloc(sizeof(MyThreadPoolCop));
	pool->create(1);

		//Worker *workersp = (Worker *)this->fpgaqueue.enqueueMapBuffer(this->workers, CL_TRUE, CL_MAP_WRITE, 0, 1*sizeof(Worker), NULL, NULL, NULL);
		//workersp[0].task_num = this->offset_number;
		//workersp[0].has_task = 1;

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

	//err = kernel.setArg(karg++, scheduler->workers);
//	if (err != CL_SUCCESS) 
//		printf("%s\n",print_cl_errstring(err));
	err = kernel.setArg(karg++, scheduler->roccpu);
	if (err != CL_SUCCESS) 
		printf("%s\n",print_cl_errstring(err));

		//bufferchecking_Reduction_Object_C(scheduler->roccpu,sizeof(Reduction_Object_C),scheduler->cpuqueue);

		printf("FPGA queue enqueuing kernel...\n");
		clock_t beforequeue = clock();
	//	int cpu_global_threads=((scheduler->offset_number/CPU_LOCAL_THREADS+1)*CPU_LOCAL_THREADS);
		err = scheduler->fpgaqueue.enqueueNDRangeKernel(kernel, cl::NullRange, 
			cl::NDRange(CPU_GLOBAL_THREADS), cl::NDRange(CPU_LOCAL_THREADS), 0, 0); 
		

		if (err != CL_SUCCESS) 
		{
        	printf("%s\n",print_cl_errstring(err));
		}
		
		scheduler->fpgaqueue.finish();
		//bufferchecking_Reduction_Object_C(scheduler->roccpu,sizeof(Reduction_Object_C),scheduler->cpuqueue);
		clock_t afterqueue = clock();
		double	time_in_seconds = (double)(afterqueue-beforequeue) / (double)CLOCKS_PER_SEC;
		printf("FPGA time: %0.2f s\n", time_in_seconds);

#if 1//DATA_PRINT     
		{ 
			int i;
		    int err = scheduler->fpgaqueue.enqueueReadBuffer(scheduler->roccpu, true, 0, sizeof(Reduction_Object_C), scheduler->rochcpu, 0, 0);
	        scheduler->fpgaqueue.finish();

			for (i=0; i<54;i++)
			{ 
		          printf("0x%x\t", ((scheduler->rochcpu)->memory_pool)[i] );
			}

			printf("num_buckets[%d] : %d\n", i, ((scheduler->rochcpu)->num_buckets) );
		}
#endif
 return 0;
}
void Scheduler::get_size_cpu()
{
	printf("In the get size cpu\n");
	cl_int err;
	cl::Kernel kernel(program1, "get_size_cpu", &err);
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

	fpgaqueue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(CPU_GLOBAL_THREADS), cl::NDRange(CPU_LOCAL_THREADS, 0, NULL));

	fpgaqueue.finish();

	err = fpgaqueue.enqueueReadBuffer(key_num_d, true, 0, sizeof(unsigned int), &key_num, 0, 0);
	err = fpgaqueue.enqueueReadBuffer(key_size_d, true, 0, sizeof(unsigned int), &key_size, 0, 0);
	err = fpgaqueue.enqueueReadBuffer(value_size_d, true, 0, sizeof(unsigned int), &value_size, 0, 0);

	fpgaqueue.finish();

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

	#if 1//DATA_PRINT     
		{ 
			int i;
		    int err = fpgaqueue.enqueueReadBuffer(roccpu, true, 0, sizeof(Reduction_Object_C), rochcpu, 0, 0);
	        fpgaqueue.finish();

			int count = 0;
			for (i=0; i<NUM_BUCKETS_C;i++)
			{    if (rochcpu->locks[i] != *(int *)( &(rochcpu->align_bucket[i])) )
			      { 
					  count++;
		              printf("0x%x: 0x%x 0x%x\n", i, rochcpu->locks[i], *(int *)( &(rochcpu->align_bucket[i]) ) );
					  if (count >10) break;
			      }
			}

			printf("num_buckets[%d] : %d\n", i, ((rochcpu)->num_buckets) );
		}
    #endif
}

void Scheduler::get_result_cpu()
{
	cl_int err;
	cl::Kernel kernel(program2, "copy_to_array_cpu", &err);
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

	fpgaqueue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(CPU_GLOBAL_THREADS), cl::NDRange(CPU_LOCAL_THREADS, 0, NULL));
	fpgaqueue.finish();

	char *key_array = (char *)_aligned_malloc(key_size,AOCL_ALIGNMENT); 
	char *value_array = (char *)_aligned_malloc(value_size,AOCL_ALIGNMENT);
	unsigned int *key_index = (unsigned int *)_aligned_malloc(sizeof(unsigned int)*key_num,AOCL_ALIGNMENT);
	unsigned int *value_index = (unsigned int *)_aligned_malloc(sizeof(unsigned int)*value_num,AOCL_ALIGNMENT);

	fpgaqueue.enqueueReadBuffer(key_array_d, true, 0, key_size, key_array, 0, 0);
	fpgaqueue.enqueueReadBuffer(value_array_d, true, 0, value_size, value_array, 0, 0);
	fpgaqueue.enqueueReadBuffer(key_index_d, true, 0, sizeof(unsigned int)*key_num, key_index, 0, 0);
	fpgaqueue.enqueueReadBuffer(value_index_d, true, 0, sizeof(unsigned int)*value_num, value_index, 0, 0);

	std::cout<<"The number of keys: "<<key_num<<std::endl;

	output.key_index = key_index;
	output.val_index = value_index;
	output.output_keys = key_array;
	output.output_vals = value_array;

}
void Scheduler::freeResource(){
	_aligned_free(rochcpu);
	_aligned_free(output.key_index);
	_aligned_free(output.val_index);
	_aligned_free(output.output_keys);
	_aligned_free(output.output_vals);
	
}

unsigned int Scheduler::get_key_num()
{
	return key_num;
}

struct output Scheduler::get_output()
{
	return output;
}