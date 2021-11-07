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
#include "internal/list.h"
#include "arm11/drivers/interrupt.h"
#include "internal/kernel_private.h"
#include "internal/slabheap.h"
#include "internal/config.h"


typedef struct
{
	bool signaled;
	const bool oneShot;
	ListNode waitQueue;
} KEvent;


static SlabHeap g_eventSlab = {0};
static KHandle g_irqEventTable[128 - 32] = {0}; // 128 - 32 private interrupts.



void signalEvent(KHandle const kevent, bool reschedule);

void _eventSlabInit(void)
{
	slabInit(&g_eventSlab, sizeof(KEvent), MAX_EVENTS);
}

static void eventIrqHandler(u32 intSource)
{
	signalEvent(g_irqEventTable[intSource - 32], false);
}

KHandle createEvent(bool oneShot)
{
	KEvent *const event = (KEvent*)slabAlloc(&g_eventSlab);

	event->signaled = false;
	*(bool*)&event->oneShot = oneShot;
	listInit(&event->waitQueue);

	return (KHandle)event;
}

void deleteEvent(KHandle const kevent)
{
	KEvent *const event = (KEvent*)kevent;

	kernelLock();
	waitQueueWakeN(&event->waitQueue, (u32)-1, KRES_HANDLE_DELETED, true);

	slabFree(&g_eventSlab, event);
}

// TODO: Critical sections needed for bind/unbind?
void bindInterruptToEvent(KHandle const kevent, uint8_t id, uint8_t prio)
{
	if(id < 32 || id > 127) return;

	g_irqEventTable[id - 32] = kevent;
	IRQ_registerIsr(id, prio, 0, eventIrqHandler);
}

void unbindInterruptEvent(uint8_t id)
{
	if(id < 32 || id > 127) return;

	g_irqEventTable[id - 32] = 0;
	IRQ_unregisterIsr(id);
}

// TODO: Timeout.
KRes waitForEvent(KHandle const kevent)
{
	KEvent *const event = (KEvent*)kevent;
	KRes res;

	kernelLock();
	if(event->signaled)
	{
		if(event->oneShot) event->signaled = false;
		kernelUnlock();
		res = KRES_OK;
	}
	else res = waitQueueBlock(&event->waitQueue);

	return res;
}

void signalEvent(KHandle const kevent, bool reschedule)
{
	KEvent *const event = (KEvent*)kevent;

	kernelLock();
	if(!event->signaled)
	{
		if(event->oneShot)
		{
			if(!waitQueueWakeN(&event->waitQueue, 1, KRES_OK, reschedule))
				event->signaled = true;
		}
		else
		{
			event->signaled = true;
			waitQueueWakeN(&event->waitQueue, (u32)-1, KRES_OK, reschedule);
		}
	}
	else kernelUnlock();
}

void clearEvent(KHandle const kevent)
{
	kernelLock(); // TODO: Can we do this without locks?
	((KEvent*)kevent)->signaled = false;
	kernelUnlock();
}
