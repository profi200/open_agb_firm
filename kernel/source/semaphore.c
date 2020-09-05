#include <stdbool.h>
#include <stdlib.h>
#include "types.h"
#include "semaphore.h"
#include "internal/list.h"
#include "internal/kernel_private.h"
#include "internal/util.h"
#include "internal/slabheap.h"
#include "internal/config.h"


typedef struct
{
	s32 count;
	ListNode waitQueue;
} Semaphore;


static SlabHeap g_semaSlab = {0};



void _semaphoreSlabInit(void)
{
	slabInit(&g_semaSlab, sizeof(Semaphore), MAX_SEMAPHORES);
}

// TODO: Test semaphore with multiple cores.
KSema createSemaphore(int32_t count)
{
	Semaphore *const sema = (Semaphore*)slabAlloc(&g_semaSlab);

	sema->count = count;
	listInit(&sema->waitQueue);

	return sema;
}

void deleteSemaphore(const KSema ksema)
{
	Semaphore *const sema = (Semaphore*)ksema;

	kernelLock();
	waitQueueWakeN(&sema->waitQueue, (u32)-1, KRES_HANDLE_DELETED, true);

	slabFree(&g_semaSlab, sema);
}

KRes pollSemaphore(const KSema ksema)
{
	Semaphore *const sema = (Semaphore*)ksema;
	KRes res;

	// TODO: Plain spinlocks instead?
	kernelLock();
	if(UNLIKELY(sema->count <= 0)) res = KRES_WOULD_BLOCK;
	else {sema->count--; res = KRES_OK;}
	kernelUnlock();

	return res;
}

KRes waitForSemaphore(const KSema ksema)
{
	Semaphore *const sema = (Semaphore*)ksema;
	KRes res;

	kernelLock();
	if(UNLIKELY(--sema->count < 0)) res = waitQueueBlock(&sema->waitQueue);
	else {kernelUnlock(); res = KRES_OK;}

	return res;
}

void signalSemaphore(const KSema ksema, uint32_t signalCount, bool reschedule)
{
	Semaphore *const sema = (Semaphore*)ksema;

	kernelLock();
	//if(UNLIKELY(++sema->count <= 0))
	if(UNLIKELY((sema->count += signalCount) <= 0))
		waitQueueWakeN(&sema->waitQueue, signalCount, KRES_OK, reschedule);
	else kernelUnlock();
}
