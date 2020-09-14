#include <stdbool.h>
#include <stdlib.h>
#include "types.h"
#include "ksemaphore.h"
#include "internal/list.h"
#include "internal/kernel_private.h"
#include "internal/util.h"
#include "internal/slabheap.h"
#include "internal/config.h"


struct KSema
{
	s32 count;
	ListNode waitQueue;
};


static SlabHeap g_semaSlab = {0};



void _semaphoreSlabInit(void)
{
	slabInit(&g_semaSlab, sizeof(KSema), MAX_SEMAPHORES);
}

// TODO: Test semaphore with multiple cores.
KSema* createSemaphore(int32_t count)
{
	KSema *const ksema = (KSema*)slabAlloc(&g_semaSlab);

	ksema->count = count;
	listInit(&ksema->waitQueue);

	return ksema;
}

void deleteSemaphore(KSema *const ksema)
{
	kernelLock();
	waitQueueWakeN(&ksema->waitQueue, (u32)-1, KRES_HANDLE_DELETED, true);

	slabFree(&g_semaSlab, ksema);
}

KRes pollSemaphore(KSema *const ksema)
{
	KRes res;

	// TODO: Plain spinlocks instead?
	kernelLock();
	if(UNLIKELY(ksema->count <= 0)) res = KRES_WOULD_BLOCK;
	else {ksema->count--; res = KRES_OK;}
	kernelUnlock();

	return res;
}

KRes waitForSemaphore(KSema *const ksema)
{
	KRes res;

	kernelLock();
	if(UNLIKELY(--ksema->count < 0)) res = waitQueueBlock(&ksema->waitQueue);
	else {kernelUnlock(); res = KRES_OK;}

	return res;
}

void signalSemaphore(KSema *const ksema, uint32_t signalCount, bool reschedule)
{
	kernelLock();
	//if(UNLIKELY(++ksema->count <= 0))
	if(UNLIKELY((ksema->count += signalCount) <= 0))
		waitQueueWakeN(&ksema->waitQueue, signalCount, KRES_OK, reschedule);
	else kernelUnlock();
}
