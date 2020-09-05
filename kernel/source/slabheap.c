#include <stddef.h>
#include <stdlib.h>
#include "internal/slabheap.h"
#include "internal/kmemcpy_set.h"



void slabInit(SlabHeap *slab, size_t objSize, size_t num)
{
	if(objSize < sizeof(SlabHeap) || !num) return;

	listInit(slab);

	void *pool = malloc(objSize * num);
	if(!pool) return;
	do
	{
		listPush(slab, (SlabHeap*)pool);
		pool += objSize;
	} while(--num);
}

void* slabAlloc(SlabHeap *slab)
{
	if(!slab || listEmpty(slab)) return NULL;

	return listPop(slab);
}

void* slabCalloc(SlabHeap *slab, size_t clrSize)
{
	void *const ptr = slabAlloc(slab);
	if(ptr) kmemset(ptr, 0, clrSize);
	return ptr;
}

void slabFree(SlabHeap *slab, void *ptr)
{
	if(!slab || !ptr) return;

	// Keep gaps filled by allocating the same mem
	// again next time an object is allocated.
	listPushTail(slab, (SlabHeap*)ptr);
}
