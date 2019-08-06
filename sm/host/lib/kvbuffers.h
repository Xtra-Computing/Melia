/*This is the shared memory kvbuffer
 when full, it is copied to the device
 mem kvbuffer in coalesced way, the index
 info will also be copied.
*/
#ifndef KVBUFFERS
#define KVBUFFERS

struct kvbuffers
{
	/*full: 
	 * the buffer is full of kvs, 
	 * and ready for scheduling 
	 * */
	bool full;

	unsigned int size;
	Index index[KVINDEX_NUM];
	char memory_pool[KV_POOL_SIZE];
};
#endif