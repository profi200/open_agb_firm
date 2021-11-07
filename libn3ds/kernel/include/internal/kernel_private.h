#pragma once

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

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include "types.h"
#include "internal/list.h"
#include "kernel.h"
#include "arm.h"


typedef enum
{
	TASK_STATE_DEAD          = 0,
	//TASK_STATE_READY         = 1,
	TASK_STATE_RUNNING       = 1,
	TASK_STATE_BLOCKED       = 2,
	TASK_STATE_RUNNING_SHORT = 3  // Continue task as soon as the woken ones are finished.
} TaskState;

struct TaskCb
{
	ListNode node;
	u8 core; // TODO: Multicore
	u8 prio;
	u8 id;
	KRes res; // Last error code. Also abused for taskArg.
	uintptr_t savedSp;
	void *stack;
	// Name?
	// Exit code?
}; // Task context
typedef struct TaskCb TaskCb;
static_assert(offsetof(TaskCb, node) == 0, "Error: Member node of TaskCb is not at offset 0!");



const TaskCb* getCurrentTask(void);
KRes waitQueueBlock(ListNode *waitQueue);
bool waitQueueWakeN(ListNode *waitQueue, u32 wakeCount, KRes res, bool reschedule);


static inline void kernelLock(void)
{
	__cpsid(i);
	//spinlockLock(&g_lock);
}
static inline void kernelUnlock(void)
{
	__cpsie(i);
	//spinlockUnlock(&g_lock);
}


// These functions belong in other headers however we
// don't want to make them accessible in the public API.
void _eventSlabInit(void);
void _mutexSlabInit(void);
void _semaphoreSlabInit(void);
void _timerInit(void);
