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

#include <stdlib.h>
#include "types.h"
#include "internal/list.h"
#include "internal/kernel_private.h"
#include "internal/util.h"
#include "internal/slabheap.h"
#include "internal/config.h"


typedef struct
{
	const TaskCb *owner;
	ListNode waitQueue;
} KMutex;


static SlabHeap g_mutexSlab = {0};



void _mutexSlabInit(void)
{
	slabInit(&g_mutexSlab, sizeof(KMutex), MAX_MUTEXES);
}

// TODO: Test mutex with multiple cores.
KHandle createMutex(void)
{
	KMutex *const kmutex = (KMutex*)slabAlloc(&g_mutexSlab);

	kmutex->owner = NULL;
	listInit(&kmutex->waitQueue);

	return (KHandle)kmutex;
}

void deleteMutex(KHandle const kmutex)
{
	KMutex *const mutex = (KMutex*)kmutex;

	kernelLock();
	waitQueueWakeN(&mutex->waitQueue, (u32)-1, KRES_HANDLE_DELETED, true);

	slabFree(&g_mutexSlab, mutex);
}

KRes lockMutex(KHandle const kmutex)
{
	KMutex *const mutex = (KMutex*)kmutex;
	KRes res;

	do
	{
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
KRes unlockMutex(KHandle const kmutex)
{
	KMutex *const mutex = (KMutex*)kmutex;
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
