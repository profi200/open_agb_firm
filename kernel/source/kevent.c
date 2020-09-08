#include <stdbool.h>
#include <stdlib.h>
#include "types.h"
#include "kevent.h"
#include "internal/list.h"
#include "arm11/hardware/interrupt.h"
#include "internal/kernel_private.h"
#include "internal/slabheap.h"
#include "internal/config.h"


typedef struct
{
	bool signaled;
	const bool oneShot;
	ListNode waitQueue;
} Event;


static SlabHeap g_eventSlab = {0};
static KEvent g_irqEventTable[128 - 32] = {0}; // 128 - 32 private interrupts.



void _eventSlabInit(void)
{
	slabInit(&g_eventSlab, sizeof(Event), MAX_EVENTS);
}

static void eventIrqHandler(u32 intSource)
{
	signalEvent(g_irqEventTable[intSource - 32], false);
}

KEvent createEvent(bool oneShot)
{
	Event *const event = (Event*)slabAlloc(&g_eventSlab);

	event->signaled = false;
	*(bool*)&event->oneShot = oneShot;
	listInit(&event->waitQueue);

	return event;
}

void deleteEvent(const KEvent kevent)
{
	Event *const event = (Event*)kevent;

	kernelLock();
	waitQueueWakeN(&event->waitQueue, (u32)-1, KRES_HANDLE_DELETED, true);

	slabFree(&g_eventSlab, event);
}

// TODO: Critical sections needed for bin/unbind?
void bindInterruptToEvent(const KEvent kevent, uint8_t id, uint8_t prio)
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
KRes waitForEvent(const KEvent kevent)
{
	Event *const event = (Event*)kevent;
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

void signalEvent(const KEvent kevent, bool reschedule)
{
	Event *const event = (Event*)kevent;

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

void clearEvent(const KEvent kevent)
{
	Event *const event = (Event*)kevent;

	kernelLock(); // TODO: Can we do this without locks?
	event->signaled = false;
	kernelUnlock();
}
