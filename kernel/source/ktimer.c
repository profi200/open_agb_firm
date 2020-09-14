#include <stdbool.h>
#include <stdlib.h>
#include "types.h"
#include "ktimer.h"
#include "internal/list.h"
#include "arm11/hardware/interrupt.h"
#include "arm11/hardware/timer.h"
#include "internal/kernel_private.h"
#include "internal/slabheap.h"
#include "internal/config.h"
 //#include "arm11/fmt.h"


/*struct KTimer
{
	ListNode node;
	u32 delta;
	u32 ticks;
	const bool pulse;
	ListNode waitQueue;
};


static SlabHeap g_timerSlab = {0};
static ListNode g_deltaQueue = {0};



static void timerIsr(UNUSED u32 intSource);
static void addToDeltaQueue(KTimer *const timer, u32 ticks);

void _timerInit(void)
{
	slabInit(&g_timerSlab, sizeof(KTimer), MAX_TIMERS);
	listInit(&g_deltaQueue);
	IRQ_registerIsr(IRQ_TIMER, 12, 0, timerIsr);
}

KTimer* createTimer(bool pulse)
{
	KTimer *const ktimer = (KTimer*)slabAlloc(&g_timerSlab);

	*(bool*)&ktimer->pulse = pulse;
	listInit(&ktimer->waitQueue);

	return ktimer;
}

void deleteTimer(KTimer *const ktimer)
{
	kernelLock();
	waitQueueWakeN(&ktimer->waitQueue, (u32)-1, KRES_HANDLE_DELETED, true);

	slabFree(&g_timerSlab, ktimer);
}

static void timerIsr(UNUSED u32 intSource)
{
	kernelLock();
//if(listEmpty(&g_deltaQueue)) *((vu32*)4) = 4; // This should never happen
	KTimer *ktimer = LIST_ENTRY(listPop(&g_deltaQueue), KTimer, node);
	if(ktimer->pulse) addToDeltaQueue(ktimer, ktimer->ticks);
	if(!listEmpty(&g_deltaQueue))
	{
		// Don't use fp math in ISRs.
		TIMER_start(1, LIST_FIRST_ENTRY(&g_deltaQueue, KTimer, node)->delta, false, true);
	}
	waitQueueWakeN(&ktimer->waitQueue, (u32)-1, KRES_OK, false);
}

static void addToDeltaQueue(KTimer *const ktimer, u32 ticks)
{
	KTimer *pos;
	u32 deltaSum = 0;
	LIST_FOR_EACH_ENTRY(pos, &g_deltaQueue, node)
	{
		deltaSum += pos->delta;
		if(deltaSum > ticks)
		{
			ktimer->delta = ticks - (deltaSum - pos->delta);
			listAddBefore(&pos->node, &ktimer->node);
			return;
		}
	}

	ktimer->delta = ticks;
	listPush(&g_deltaQueue, &ktimer->node);
}

void startTimer(KTimer *const ktimer, uint32_t usec)
{
	const u32 ticks = TIMER_FREQ(1, 1000000) * usec;
	ktimer->ticks = ticks;

	kernelLock();
	const bool firstTimer = listEmpty(&g_deltaQueue);
	addToDeltaQueue(ktimer, ticks);
	kernelUnlock();
	if(firstTimer) TIMER_start(1, ticks, false, true);
}

void stopTimer(KTimer *const ktimer)
{
}

KRes waitForTimer(KTimer *const ktimer)
{
	kernelLock();
	return waitQueueBlock(&ktimer->waitQueue);
}*/
