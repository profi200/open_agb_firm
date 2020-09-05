#include <stdbool.h>
#include <stdlib.h>
#include "types.h"
#include "timer.h"
#include "internal/list.h"
#include "arm11/hardware/interrupt.h"
#include "arm11/hardware/timer.h"
#include "internal/kernel_private.h"
#include "internal/slabheap.h"
#include "internal/config.h"
 //#include "arm11/fmt.h"


/*typedef struct
{
	ListNode node;
	u32 delta;
	u32 ticks;
	const bool pulse;
	ListNode waitQueue;
} Timer;


static SlabHeap g_timerSlab = {0};
static ListNode g_deltaQueue = {0};



static void timerIrqHandler(UNUSED u32 intSource);
static void addToDeltaQueue(Timer *const timer, u32 ticks);

void _timerInit(void)
{
	slabInit(&g_timerSlab, sizeof(Timer), MAX_TIMERS);
	listInit(&g_deltaQueue);
	IRQ_registerHandler(IRQ_TIMER, 12, 0, true, timerIrqHandler);
}

KTimer createTimer(bool pulse)
{
	Timer *const timer = (Timer*)slabAlloc(&g_timerSlab);

	*(bool*)&timer->pulse = pulse;
	listInit(&timer->waitQueue);

	return timer;
}

void deleteTimer(const KTimer ktimer)
{
	Timer *const timer = (Timer*)ktimer;

	kernelLock();
	waitQueueWakeN(&timer->waitQueue, (u32)-1, KRES_HANDLE_DELETED, true);

	slabFree(&g_timerSlab, timer);
}

static void timerIrqHandler(UNUSED u32 intSource)
{
	kernelLock();
//if(listEmpty(&g_deltaQueue)) *((vu32*)4) = 4; // This should never happen
	Timer *timer = LIST_ENTRY(listPop(&g_deltaQueue), Timer, node);
	if(timer->pulse) addToDeltaQueue(timer, timer->ticks);
	if(!listEmpty(&g_deltaQueue))
	{
		// Don't use fp math in ISRs.
		TIMER_start(1, LIST_FIRST_ENTRY(&g_deltaQueue, Timer, node)->delta, false, true);
	}
	waitQueueWakeN(&timer->waitQueue, (u32)-1, KRES_OK, false);
}

static void addToDeltaQueue(Timer *const timer, u32 ticks)
{
	Timer *pos;
	u32 deltaSum = 0;
	LIST_FOR_EACH_ENTRY(pos, &g_deltaQueue, node)
	{
		deltaSum += pos->delta;
		if(deltaSum > ticks)
		{
			timer->delta = ticks - (deltaSum - pos->delta);
			listAddBefore(&pos->node, &timer->node);
			return;
		}
	}

	timer->delta = ticks;
	listPush(&g_deltaQueue, &timer->node);
}

void startTimer(const KTimer ktimer, uint32_t usec)
{
	Timer *const timer = (Timer*)ktimer;

	const u32 ticks = TIMER_FREQ(1, 1000000) * usec;
	timer->ticks = ticks;

	kernelLock();
	const bool firstTimer = listEmpty(&g_deltaQueue);
	addToDeltaQueue(timer, ticks);
	kernelUnlock();
	if(firstTimer) TIMER_start(1, ticks, false, true);
}

void stopTimer(const KTimer ktimer)
{
}

KRes waitForTimer(const KTimer ktimer)
{
	Timer *const timer = (Timer*)ktimer;

	kernelLock();
	return waitQueueBlock(&timer->waitQueue);
}*/
