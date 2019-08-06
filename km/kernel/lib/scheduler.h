#include "rol.h"
#include "rog.h"
#include "roc.h"
#include "ds.h"
#include <CL/cl.h>
#include <CL/cl.hpp>
#include "kvbuffer.h"
#include "configuration.h"
#include <SDKCommon.hpp>
#include <SDKApplication.hpp>
#include <SDKFile.hpp>
#include "worker_info.h"
//#include <semaphore.h>


class Scheduler
{
	public:
		
		void *global_data;	//The input data in global memory
		unsigned int global_data_size; //the size of the global data
		void *local_data;	//The input data in local memory
		unsigned int local_data_size;  //the size of the local data

		void *global_offset; //Stores the offset information
		short *token;           //the thread block token
		unsigned int offset_number; //number of offsets
		unsigned int unit_size; //the unit_size of offset, used to jump
		
		struct output output;
		bool sort;
		bool usegpu;
		int uselocal; //whether use shared memory: 0: no, 1:yes

		Reduction_Object_C *rochcpu;
		Reduction_Object_G *roghgpu;

		cl::Buffer global_data_d;
		cl::Buffer local_data_d;
		cl::Buffer shared_local_data_d;
		cl::Buffer global_offset_d;
		
		unsigned int key_num;
		unsigned int key_size;
		unsigned int value_num;
		unsigned int value_size;
		cl::Buffer key_num_d;
		cl::Buffer key_size_d;
		cl::Buffer value_size_d;

		cl::Buffer workers;
		cl::Buffer roccpu;
		cl::Buffer roggpu;

		cl::Platform platform;
		std::vector<cl::Device> devices;
		cl::Device device;
		cl::Context context;
		cl::CommandQueue fpgaqueue;
		cl::Program program;
		cl::Program program1;
		cl::Program program2;

		Scheduler(void *global_data, 
			unsigned int global_data_size,
			void *global_data_offset,
			unsigned int global_data_offset_number,
			unsigned int offset_unit_size);

		~Scheduler()
		{
			free(rochcpu);
			free(roghgpu);
			//free(buffersh);
			free(output.key_index);
			free(output.output_keys);
			free(output.output_vals);
			free(output.val_index);
		}

		void do_mapreduce();
		unsigned int get_key_num();
		struct output get_output();
		//void destroy();
		static DWORD WINAPI test(void *arg);

	private:
		void get_size_cpu();
		void get_result_cpu();
		void usetest();
};

