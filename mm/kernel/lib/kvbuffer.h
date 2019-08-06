/*This is the device memory kvbuffer, it is used
for storing kv data from shared memory and used for
job scheduling*/

#include "configuration.h"

#ifndef KVBUFFER
#define KVBUFFER

struct element 
{
	unsigned int x;
	unsigned int y;
};

typedef struct element Index;

struct kvbuffer
{
	/*empty: 
	 * the buffer has been emptied 
	 * and is ready for writting 
	 * */
	bool empty;

	/*full: 
	 * the buffer is full of kvs, 
	 * and ready for scheduling 
	 * */
	bool full;

	/*busy:
	 * the buffer is being written
	 * or being read
	 * once assigned to a writer
	 * or reader, it is busy
	 * */
	bool busy;

	unsigned int size;
	Index index[KVINDEX_NUM];
	char memory_pool[KV_POOL_SIZE];
};

typedef struct kvbuffer Kvbuffer;

struct doublebuffer
{
	/*current:
	 * the current index of buffer
	 * being used. 
	 * 0: first buffer
	 * 1: second buffer
	 * -1: both are full
	 * */

	int current;
	Kvbuffer buffers[2];
};

typedef struct doublebuffer Doublebuffer;

//bool insert_to_kvbuffer(unsigned short key_size, void *key, unsigned short value_size, void *value, kvbuffer *buffer, local unsigned int *index_offset, local unsigned int *memory_offset)
//{
	
//}
#endif
