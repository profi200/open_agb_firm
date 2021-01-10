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

#include <stdbool.h>
#include <stdlib.h>
#include "types.h"
#include "kevent.h"
#include "internal/list.h"
#include "arm11/hardware/interrupt.h"
#include "internal/kernel_private.h"
#include "internal/slabheap.h"
#include "internal/config.h"


struct KEvent
{
	bool signaled;
	const bool oneShot;
	ListNode waitQueue;
};


static SlabHeap g_eventSlab = {0};
static KEvent *g_irqEventTable[128 - 32] = {0}; // 128 - 32 private interrupts.



void _eventSlabInit(void)
{
	slabInit(&g_eventSlab, sizeof(KEvent), MAX_EVENTS);
}

static void eventIrqHandler(u32 intSource)
{
	signalEvent(g_irqEventTable[intSource - 32], false);
}

KEvent* createEvent(bool oneShot)
{
	KEvent *const kevent = (KEvent*)slabAlloc(&g_eventSlab);

	kevent->signaled = false;
	*(bool*)&kevent->oneShot = oneShot;
	listInit(&kevent->waitQueue);

	return kevent;
}

void deleteEvent(KEvent *const kevent)
{
	kernelLock();
	waitQueueWakeN(&kevent->waitQueue, (u32)-1, KRES_HANDLE_DELETED, true);

	slabFree(&g_eventSlab, kevent);
}

// TODO: Critical sections needed for bind/unbind?
void bindInterruptToEvent(KEvent *const kevent, uint8_t id, uint8_t prio)
{
	if(id < 32 || id > 127) return;

	g_irqEventTable[id - 32] = kevent;
	IRQ_registerIsr(id, prio, 0, eventIrqHandler);
}

void unbindInterruptEvent(uint8_t id)
{
	if(id < 32 || id > 127) return;

	g_irqEventTable[id - 32] = NULL;
	IRQ_unregisterIsr(id);
}

// TODO: Timeout.
KRes waitForEvent(KEvent *const kevent)
{
	KRes res;

	kernelLock();
	if(kevent->signaled)
	{
		if(kevent->oneShot) kevent->signaled = false;
		kernelUnlock();
		res = KRES_OK;
	}
	else res = waitQueueBlock(&kevent->waitQueue);

	return res;
}

void signalEvent(KEvent *const kevent, bool reschedule)
{
	kernelLock();
	if(!kevent->signaled)
	{
		if(kevent->oneShot)
		{
			if(!waitQueueWakeN(&kevent->waitQueue, 1, KRES_OK, reschedule))
				kevent->signaled = true;
		}
		else
		{
			kevent->signaled = true;
			waitQueueWakeN(&kevent->waitQueue, (u32)-1, KRES_OK, reschedule);
		}
	}
	else kernelUnlock();
}

void clearEvent(KEvent *const kevent)
{
	kernelLock(); // TODO: Can we do this without locks?
	kevent->signaled = false;
	kernelUnlock();
}
