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
#include <stdnoreturn.h>
#include <string.h>
#include "types.h"
#include "kernel.h"
#include "internal/config.h"
#include "internal/kernel_private.h"
#include "internal/kmemcpy_set.h"
#include "internal/slabheap.h"
#include "internal/util.h"
#include "internal/list.h"
#include "internal/contextswitch.h"
#include "arm.h"


static TaskCb *g_curTask = NULL;
static u32 g_readyBitmap = 0;
static ListNode g_runQueues[MAX_PRIO_BITS] = {0};
static SlabHeap g_taskSlab = {0};
static u32 g_numTasks = 0;
static TaskCb *g_curDeadTask = NULL; // TODO: Improve dead task handling.



static KRes scheduler(TaskState curTaskState);
noreturn static void kernelIdleTask(void);

static void initKernelState(void)
{
	for(int i = 0; i < MAX_PRIO_BITS; i++) listInit(&g_runQueues[i]);
	slabInit(&g_taskSlab, sizeof(TaskCb), MAX_TASKS);
	_eventSlabInit();
	_mutexSlabInit();
	_semaphoreSlabInit();
	//_timerInit();
}

/*
 * Public kernel API.
*/
// TODO: Are KTask handles needed? (for the main task)
// TODO: Thread local storage. Needed?
void kernelInit(uint8_t priority)
{
	if(priority > MAX_PRIO_BITS - 1u) return;

	// TODO: Split this mess into helper functions.
	initKernelState();

	TaskCb *const idleT = (TaskCb*)slabAlloc(&g_taskSlab);
	void *const iStack = malloc(IDLE_STACK_SIZE);
	TaskCb *const mainT = (TaskCb*)slabCalloc(&g_taskSlab, sizeof(TaskCb));
	if(idleT == NULL || iStack == NULL || mainT == NULL)
	{
		slabFree(&g_taskSlab, idleT);
		free(iStack);
		slabFree(&g_taskSlab, mainT);
		return;
	}

	cpuRegs *const regs = (cpuRegs*)(iStack + IDLE_STACK_SIZE - sizeof(cpuRegs));
	regs->lr            = (u32)kernelIdleTask;
	idleT->prio         = 1;
	// id is already set to 0.
	idleT->savedSp      = (uintptr_t)regs;
	idleT->stack        = iStack;

	// Main task already running. Nothing more to setup.
	mainT->id    = 1;
	mainT->prio  = priority;

	g_curTask = mainT;
	g_readyBitmap = 1u<<1; // The idle task has priority 1 and is always ready.
	listPush(&g_runQueues[1], &idleT->node);
	g_numTasks = 2;
}

KHandle createTask(size_t stackSize, uint8_t priority, TaskFunc entry, void *taskArg)
{
	if(priority > MAX_PRIO_BITS - 1u) return 0;

	// Make sure the stack is aligned to 8 bytes
	stackSize = (stackSize + 7u) & ~7u;

	SlabHeap *const taskSlabPtr = &g_taskSlab;
	TaskCb *const newT = (TaskCb*)slabAlloc(taskSlabPtr);
	void *const stack  = malloc(stackSize);
	if(newT == NULL || stack == NULL)
	{
		slabFree(taskSlabPtr, newT);
		free(stack);
		return 0;
	}

	cpuRegs *const regs = (cpuRegs*)(stack + stackSize - sizeof(cpuRegs));
	kmemset((u32*)regs, 0, sizeof(cpuRegs));
	regs->lr            = (u32)entry;
	newT->prio          = priority;
	newT->id            = g_numTasks; // TODO: Make this more sophisticated.
	// TODO: This is kinda hacky abusing the result member to pass the task arg.
	// Pass args and stuff on the stack?
	newT->res           = (KRes)taskArg;
	newT->savedSp       = (uintptr_t)regs;
	newT->stack         = stack;

	kernelLock();
	listPush(&g_runQueues[priority], &newT->node);
	g_readyBitmap |= 1u<<priority;
	g_numTasks++;
	kernelUnlock();

	return (uintptr_t)newT;
}

// TODO: setTaskPriority().

void yieldTask(void)
{
	kernelLock();
	scheduler(TASK_STATE_RUNNING);
}

void taskExit(void)
{
	kernelLock();
	scheduler(TASK_STATE_DEAD);
	while(1); // TODO: panic?
}


/*
 * Internal functions.
*/
const TaskCb* getCurrentTask(void)
{
	return g_curTask;
}

// The wait queue and scheduler functions automatically unlock the kernel lock
// and expect to be called with locked lock.
KRes waitQueueBlock(ListNode *waitQueue)
{
	listPush(waitQueue, &g_curTask->node);
	return scheduler(TASK_STATE_BLOCKED);
}

bool waitQueueWakeN(ListNode *waitQueue, u32 wakeCount, KRes res, bool reschedule)
{
	if(listEmpty(waitQueue) || !wakeCount)
	{
		kernelUnlock();
		return false;
	}

	u32 readyBitmap = 0;
	ListNode *const runQueues = g_runQueues;
	if(LIKELY(reschedule))
	{
		// Put ourself on top of the list first so we run immediately
		// after the woken tasks to finish the work we were doing.
		// TODO: Verify if this is a good strategy.
		TaskCb *const curTask = g_curTask;
		const u8 curPrio = curTask->prio;
		listPushTail(&runQueues[curPrio], &curTask->node);
		readyBitmap = 1u<<curPrio;
	}

	do
	{
		/*
		 * Edge case:
		 * 2 tasks, 1 single shot event. Task 2 waits first and then task 1.
		 * When signaled (by an IRQ) only task 1 will ever run instead of
		 * alternating between both because task 1 always lands on the
		 * head (as intended) but on wakeup we take N tasks from the head
		 * to preserve order.
		 *
		 * Workaround:
		 * Take tasks from the tail instead. This will however punish
		 * the longest waiting tasks unnecessarily.
		 */
		//TaskCb *task = LIST_ENTRY(listPopHead(waitQueue), TaskCb, node);
		TaskCb *task = LIST_ENTRY(listPop(waitQueue), TaskCb, node);
		readyBitmap |= 1u<<task->prio;
		task->res = res;
		listPushTail(&runQueues[task->prio], &task->node);
	} while(!listEmpty(waitQueue) && --wakeCount);
	g_readyBitmap |= readyBitmap;

	if(LIKELY(reschedule)) scheduler(TASK_STATE_RUNNING_SHORT);
	else                   kernelUnlock();

	return true;
}

static KRes scheduler(TaskState curTaskState)
{
#ifndef NDEBUG
	if((__getCpsr() & PSR_MODE_MASK) != PSR_SYS_MODE) panic();
#endif

	TaskCb *const curDeadTask = g_curDeadTask;
	// TODO: Get rid of this and find a better way.
	if(UNLIKELY(curDeadTask != NULL))
	{
		free(curDeadTask->stack);
		slabFree(&g_taskSlab, curDeadTask);
		g_curDeadTask = NULL;
	}

	TaskCb *const curTask = g_curTask;
	u32 readyBitmap = g_readyBitmap;
	ListNode *const runQueues = g_runQueues;
	// Warning. The result is undefined if the input of this builtin is 0!
	// Edge case: All tasks are sleeping except the (curently running) idle task.
	//            g_readyBitmap is 0 in this case.
	const unsigned int readyPrio = (readyBitmap ? 31u - __builtin_clz(readyBitmap) : 0u);
	if(LIKELY(curTaskState == TASK_STATE_RUNNING))
	{
		const u8 curPrio = curTask->prio;

		if(readyPrio < curPrio)
		{
			kernelUnlock();
			return KRES_OK;
		}

		listPush(&runQueues[curPrio], &curTask->node);
		readyBitmap |= 1u<<curPrio;
	}
	else if(UNLIKELY(curTaskState == TASK_STATE_DEAD))
	{
		g_curDeadTask = curTask;
		g_numTasks--;
	}

	TaskCb *newTask = LIST_ENTRY(listPop(&runQueues[readyPrio]), TaskCb, node);
	if(listEmpty(&runQueues[readyPrio])) readyBitmap &= ~(1u<<readyPrio);
	g_readyBitmap = readyBitmap;

	TaskCb *oldTask = curTask;
	g_curTask = newTask;
	const KRes res = newTask->res;
	kernelUnlock();

	return switchContext(res, &oldTask->savedSp, newTask->savedSp);
}

// TODO: Cleanup deleted tasks in here? Or create a worker task?
noreturn static void kernelIdleTask(void)
{
	do
	{
		__wfi();
		kernelLock();
		scheduler(TASK_STATE_RUNNING);
	} while(1);
}
