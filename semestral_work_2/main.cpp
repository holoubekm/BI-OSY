#ifndef __PROGTEST__
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <cmath>
using namespace std;
#endif /* __PROGTEST__ */

using byte = uint8_t;

//Optimize this value
//#define HEADER_SIZE (ALIGN(sizeof(uint32_t)))

#define THRESHOLD 10000

#define IS_FULL(header) ((header) & 1L)
#define IS_FREE(header) (!IS_FULL(header))
#define SET_FREE(header) (header) = (header) & ~1L
#define SET_FULL(header) (header) = (header) | 1L

#define PTR_TO_BLOCK_T(ptr) reinterpret_cast<block_t*>(ptr)
#define HEADER_SIZE sizeof(block_t)

struct block_t
{
	uint32_t size;
	block_t* next;
	block_t* prev;
};

void* gPool;
uint32_t gPoolSize;

void* gHeapStart;
void* gHeapEnd;

uint32_t gHeapSize;

uint32_t gNumAllocs;

static const uint32_t	gBlkMin			= 5;
static const uint32_t	gBlkMax			= 0;
static const uint8_t	gBlkMinSize		= 32;
static uint32_t			gBlkMaxSize		= 0;
static const uint8_t	gBlkSizesCnt	= 27;
block_t					gBlks[gBlkSizesCnt];


#define ALIGNMENT gBlkMinSize
//#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))


void addBlock(block_t* header);

void HeapInit(void* memPool, int memSize)
{
	gNumAllocs = 0;

	gPool = memPool;
	gPoolSize = memSize;
	gBlkMaxSize = gBlkMinSize;
	while (gBlkMaxSize << 1 <= (uint32_t)memSize)
		gBlkMaxSize <<= 1;


	for (int x = 0; x < gBlkSizesCnt; x++)
	{
		gBlks[x].size = 1 << (gBlkMin + x);
		gBlks[x].next = gBlks[x].prev = &gBlks[x];
	}

	//gHeapSize = ALIGN((gPoolSize - ALIGN(gDelta)));
	gHeapSize = gBlkMaxSize;
	gHeapStart = reinterpret_cast<uint32_t*>(gPool);
	gHeapEnd = reinterpret_cast<uint32_t*>(reinterpret_cast<byte*>(gHeapStart) + gHeapSize);


	block_t* cur = reinterpret_cast<block_t*>(gHeapStart);
	cur->size = gHeapSize;
	addBlock(cur);
}

void addBlock(block_t* header)
{
	int i = 0;
	uint32_t size = header->size;
	while (size >>= 1)
		i++;
	i -= gBlkMin;

	header->prev = &gBlks[i];
	header->next = gBlks[i].next;
	gBlks[i].next = gBlks[i].next->prev = header;
}

void removeBlock(block_t* block)
{
	if (block != nullptr)
	{
		block->next->prev = block->prev;
		block->prev->next = block->next;
	}
}

block_t* findBlk(uint32_t size)
{
	uint32_t power = 1 << gBlkMin;
	while (power < size)
		power <<= 1;



	int x = 0;
	for (x = 0; x < gBlkSizesCnt; x++)
	{
		if (gBlks[x].size >= power && (&gBlks[x]) != gBlks[x].next)
		{
			break;
		}
	}

	if (x < gBlkSizesCnt)
	{
		block_t* cur = gBlks[x].next;
		removeBlock(cur);

		while (cur->size > power)
		{
			uint32_t half = cur->size >> 1;

			cur->size = half;

			block_t* rest = reinterpret_cast<block_t*>(reinterpret_cast<byte*>(cur) + half);
			rest->size = half;
			addBlock(rest);
		}
		return cur;
	}
	
	return nullptr;
}

void* HeapAlloc(int size)
{
	if (size == 0)
		return nullptr;

	uint32_t blkSize = size + HEADER_SIZE;
	block_t* header = findBlk(blkSize);
	if (header)
	{
		SET_FULL(header->size);
		gNumAllocs++;
		return reinterpret_cast<byte*>(header) + HEADER_SIZE;
	}
	return nullptr;
}

block_t* findBuddy(block_t* block)
{

	uint64_t _block = (uint64_t)block - (uint64_t)gHeapStart;
	uint64_t _buddy = _block ^ block->size;
	block_t* buddy = reinterpret_cast<block_t*>(reinterpret_cast<byte*>(gHeapStart) + _buddy);

	if (buddy >= gHeapStart && buddy < gHeapEnd)
		return buddy;
	return nullptr;
}

bool HeapFree(void* blk)
{
	block_t* header = reinterpret_cast<block_t*>(reinterpret_cast<byte*>(blk) - HEADER_SIZE);
	if (blk >= gHeapStart && blk <= gHeapEnd)
	{
		if (IS_FULL(header->size))
		{
			SET_FREE(header->size);

			block_t* buddy = nullptr;
			while (header->size < gBlkMaxSize)
			{
				buddy = findBuddy(header);
				if (buddy == nullptr)
					break;
				if (IS_FULL(buddy->size))
					break;
				if (buddy->size != header->size)
					break;

				removeBlock(buddy);

				if (buddy < header)
					header = buddy;
				header->size <<= 1;
			}
			
			addBlock(header);

			gNumAllocs--;
			return true;
		}
	}
	return false;
}

void HeapDone(int* pendingBlk)
{
	*pendingBlk = gNumAllocs;
}


#ifndef __PROGTEST__

void dump()
{
	int x = 0;
	block_t* cur = reinterpret_cast<block_t*>(gHeapStart);
	while (cur < gHeapEnd)
	{
		uint32_t size = IS_FREE(cur->size) ? cur->size : (cur->size & ~1L);
		cout << "x: " << x << ", size: " << size << ", free: " << IS_FREE(cur->size) << endl;
		cur = reinterpret_cast<block_t*>(reinterpret_cast<byte*>(cur)+size);
		x++;
	} cout << endl;
}

int main(void)
{
	uint8_t       * p0, *p1, *p2, *p3, *p4;
	int             pendingBlk;
	static uint8_t  memPool[3 * 1048576];

	HeapInit(memPool, 2048);

	void* a = HeapAlloc(16);
	dump();
	void* b = HeapAlloc(16);
	dump();
	//HeapAlloc(1);
	void* c = HeapAlloc(16);
	dump();
	//HeapAlloc(1);
	void* d = HeapAlloc(16);
	dump();
	//HeapAlloc(1);
	void* e = HeapAlloc(16);
	dump();
	//HeapAlloc(1);
	void* f = HeapAlloc(16);
	//HeapAlloc(1);

	dump();
	HeapFree(b);
	dump();
	HeapFree(c);
	dump();
	HeapFree(d);
	dump();
	HeapFree(e);
	dump();
	HeapFree(f);
	dump();

	HeapFree(a);
	dump();

	HeapAlloc(240);
	dump();
	//system("pause");
	//return 0;



	



	HeapInit(memPool, 2097152);
	assert((p0 = (uint8_t*)HeapAlloc(512000)) != NULL);
	memset(p0, 0, 512000);
	assert((p1 = (uint8_t*)HeapAlloc(511000)) != NULL);
	memset(p1, 0, 511000);
	assert((p2 = (uint8_t*)HeapAlloc(26000)) != NULL);
	memset(p2, 0, 26000);
	HeapDone(&pendingBlk);
	assert(pendingBlk == 3);



	HeapInit(memPool, 2097152);
	assert((p0 = (uint8_t*)HeapAlloc(1000000)) != NULL);
	memset(p0, 0, 1000000);
	assert((p1 = (uint8_t*)HeapAlloc(250000)) != NULL);
	memset(p1, 0, 250000);
	assert((p2 = (uint8_t*)HeapAlloc(250000)) != NULL);
	memset(p2, 0, 250000);
	assert((p3 = (uint8_t*)HeapAlloc(250000)) != NULL);
	memset(p3, 0, 250000);
	assert((p4 = (uint8_t*)HeapAlloc(50000)) != NULL);
	memset(p4, 0, 50000);
	assert(HeapFree(p2));
	assert(HeapFree(p4));
	assert(HeapFree(p3));
	assert(HeapFree(p1));
	assert((p1 = (uint8_t*)HeapAlloc(500000)) != NULL);
	memset(p1, 0, 500000);
	assert(HeapFree(p0));
	assert(HeapFree(p1));
	HeapDone(&pendingBlk);
	assert(pendingBlk == 0);


	HeapInit(memPool, 2359296);
	assert((p0 = (uint8_t*)HeapAlloc(1000000)) != NULL);
	memset(p0, 0, 1000000);
	assert((p1 = (uint8_t*)HeapAlloc(500000)) != NULL);
	memset(p1, 0, 500000);
	assert((p2 = (uint8_t*)HeapAlloc(500000)) != NULL);
	memset(p2, 0, 500000);
	assert((p3 = (uint8_t*)HeapAlloc(500000)) == NULL);
	assert(HeapFree(p2));
	assert((p2 = (uint8_t*)HeapAlloc(300000)) != NULL);
	memset(p2, 0, 300000);
	assert(HeapFree(p0));
	assert(HeapFree(p1));
	HeapDone(&pendingBlk);
	assert(pendingBlk == 1);


	HeapInit(memPool, 2359296);
	assert((p0 = (uint8_t*)HeapAlloc(1000000)) != NULL);
	memset(p0, 0, 1000000);
	assert(!HeapFree(p0 + 1000));
	HeapDone(&pendingBlk);
	assert(pendingBlk == 1);


	system("pause");
	return 0;
}
#endif /* __PROGTEST__ */