/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2021 derrek, profi200
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*#include <stdbool.h>
#include <stdlib.h>
#include "types.h"
#include "ktimer.h"
#include "internal/list.h"
#include "arm11/drivers/interrupt.h"
//#include "arm11/drivers/timer.h"
#include "internal/kernel_private.h"
#include "internal/slabheap.h"
#include "internal/config.h"
 //#include "arm11/fmt.h"


typedef struct
{
	ListNode node;
	u32 delta;
	u32 ticks;
	const bool pulse;
	ListNode waitQueue;
} KTimer;


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

KHandle createTimer(bool pulse)
{
	KTimer *const ktimer = (KTimer*)slabAlloc(&g_timerSlab);

	*(bool*)&ktimer->pulse = pulse;
	listInit(&ktimer->waitQueue);

	return (KHandle)ktimer;
}

void deleteTimer(KHandle const ktimer)
{
	KTimer *const timer = (KTimer*)ktimer;

	kernelLock();
	waitQueueWakeN(&timer->waitQueue, (u32)-1, KRES_HANDLE_DELETED, true);

	slabFree(&g_timerSlab, timer);
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

void startTimer(KHandle const ktimer, uint32_t usec)
{
	KTimer *const timer = (KTimer*)ktimer;

	const u32 ticks = TIMER_FREQ(1, 1000000) * usec;
	timer->ticks = ticks;

	kernelLock();
	const bool firstTimer = listEmpty(&g_deltaQueue);
	addToDeltaQueue(timer, ticks);
	kernelUnlock();
	if(firstTimer) TIMER_start(1, ticks, false, true);
}

void stopTimer(KHandle const ktimer)
{
}

KRes waitForTimer(KHandle const ktimer)
{
	KTimer *const timer = (KTimer*)ktimer;

	kernelLock();
	return waitQueueBlock(&timer->waitQueue);
}*/
