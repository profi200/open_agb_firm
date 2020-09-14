#include <stdlib.h>
#include "types.h"
#include "kmutex.h"
#include "internal/list.h"
#include "internal/kernel_private.h"
#include "internal/util.h"
#include "internal/slabheap.h"
#include "internal/config.h"


struct KMutex
{
	const TaskCb *owner;
	ListNode waitQueue;
};


static SlabHeap g_mutexSlab = {0};



void _mutexSlabInit(void)
{
	slabInit(&g_mutexSlab, sizeof(KMutex), MAX_MUTEXES);
}

// TODO: Test mutex with multiple cores.
KMutex* createMutex(void)
{
	KMutex *const kmutex = (KMutex*)slabAlloc(&g_mutexSlab);

	kmutex->owner = NULL;
	listInit(&kmutex->waitQueue);

	return kmutex;
}

void deleteMutex(KMutex *const kmutex)
{
	kernelLock();
	waitQueueWakeN(&kmutex->waitQueue, (u32)-1, KRES_HANDLE_DELETED, true);

	slabFree(&g_mutexSlab, kmutex);
}

KRes lockMutex(KMutex *const kmutex)
{
	KRes res;

	do
	{
		kernelLock();
		if(UNLIKELY(kmutex->owner != NULL))
		{
			res = waitQueueBlock(&kmutex->waitQueue);
			if(UNLIKELY(res != KRES_OK)) break;
		}
		else
		{
			kmutex->owner = getCurrentTask();
			kernelUnlock();
			res = KRES_OK;
			break;
		}
	} while(1);

	return res;
}

// TODO: Test if it works and only unlocks if current task == owner.
KRes unlockMutex(KMutex *const kmutex)
{
	KRes res = KRES_OK;

	kernelLock();
	if(LIKELY(kmutex->owner != NULL))
	{
		if(LIKELY(kmutex->owner == getCurrentTask()))
		{
			kmutex->owner = NULL;
			waitQueueWakeN(&kmutex->waitQueue, 1, KRES_OK, true);
		}
		else res = KRES_NO_PERMISSIONS;
	}
	else kernelUnlock();

	return res;
}
