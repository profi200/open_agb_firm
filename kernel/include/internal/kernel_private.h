#pragma once

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

typedef struct
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
} TaskCb; // Task context
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
