#include <stdlib.h>
#include "types.h"
#include "kmutex.h"
#include "internal/list.h"
#include "internal/kernel_private.h"
#include "internal/util.h"
#include "internal/slabheap.h"
#include "internal/config.h"


typedef struct
{
	const TaskCb *owner;
	ListNode waitQueue;
} Mutex;


static SlabHeap g_mutexSlab = {0};



void _mutexSlabInit(void)
{
	slabInit(&g_mutexSlab, sizeof(Mutex), MAX_MUTEXES);
}

// TODO: Test mutex with multiple cores.
KMutex createMutex(void)
{
	Mutex *const mutex = (Mutex*)slabAlloc(&g_mutexSlab);

	mutex->owner = NULL;
	listInit(&mutex->waitQueue);

	return mutex;
}

void deleteMutex(const KMutex kmutex)
{
	Mutex *const mutex = (Mutex*)kmutex;

	kernelLock();
	waitQueueWakeN(&mutex->waitQueue, (u32)-1, KRES_HANDLE_DELETED, true);

	slabFree(&g_mutexSlab, mutex);
}

KRes lockMutex(const KMutex kmutex)
{
	KRes res;

	do
	{
		Mutex *const mutex = (Mutex*)kmutex;

		kernelLock();
		if(UNLIKELY(mutex->owner != NULL))
		{
			res = waitQueueBlock(&mutex->waitQueue);
			if(UNLIKELY(res != KRES_OK)) break;
		}
		else
		{
			mutex->owner = getCurrentTask();
			kernelUnlock();
			res = KRES_OK;
			break;
		}
	} while(1);

	return res;
}

// TODO: Test if it works and only unlocks if current task == owner.
KRes unlockMutex(const KMutex kmutex)
{
	Mutex *const mutex = (Mutex*)kmutex;
	KRes res = KRES_OK;

	kernelLock();
	if(LIKELY(mutex->owner != NULL))
	{
		if(LIKELY(mutex->owner == getCurrentTask()))
		{
			mutex->owner = NULL;
			waitQueueWakeN(&mutex->waitQueue, 1, KRES_OK, true);
		}
		else res = KRES_NO_PERMISSIONS;
	}
	else kernelUnlock();

	return res;
}
