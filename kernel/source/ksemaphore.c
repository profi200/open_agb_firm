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
#include "internal/kernel_private.h"
#include "internal/util.h"
#include "internal/slabheap.h"
#include "internal/config.h"


typedef struct
{
	s32 count;
	ListNode waitQueue;
} KSema;


static SlabHeap g_semaSlab = {0};



void _semaphoreSlabInit(void)
{
	slabInit(&g_semaSlab, sizeof(KSema), MAX_SEMAPHORES);
}

// TODO: Test semaphore with multiple cores.
KHandle createSemaphore(int32_t count)
{
	KSema *const ksema = (KSema*)slabAlloc(&g_semaSlab);

	ksema->count = count;
	listInit(&ksema->waitQueue);

	return (KHandle)ksema;
}

void deleteSemaphore(KHandle const ksema)
{
	KSema *const sema = (KSema*)ksema;

	kernelLock();
	waitQueueWakeN(&sema->waitQueue, (u32)-1, KRES_HANDLE_DELETED, true);

	slabFree(&g_semaSlab, sema);
}

KRes pollSemaphore(KHandle const ksema)
{
	KSema *const sema = (KSema*)ksema;
	KRes res;

	// TODO: Plain spinlocks instead?
	kernelLock();
	if(UNLIKELY(sema->count <= 0)) res = KRES_WOULD_BLOCK;
	else {sema->count--; res = KRES_OK;}
	kernelUnlock();

	return res;
}

KRes waitForSemaphore(KHandle const ksema)
{
	KSema *const sema = (KSema*)ksema;
	KRes res;

	kernelLock();
	if(UNLIKELY(--sema->count < 0)) res = waitQueueBlock(&sema->waitQueue);
	else {kernelUnlock(); res = KRES_OK;}

	return res;
}

void signalSemaphore(KHandle const ksema, uint32_t signalCount, bool reschedule)
{
	KSema *const sema = (KSema*)ksema;

	kernelLock();
	//if(UNLIKELY(++sema->count <= 0))
	if(UNLIKELY((sema->count += signalCount) <= 0))
		waitQueueWakeN(&sema->waitQueue, signalCount, KRES_OK, reschedule);
	else kernelUnlock();
}
