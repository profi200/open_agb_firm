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

#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C"
{
#endif

enum
{
	KRES_OK              = 0, // No error.
	KRES_INVALID_HANDLE  = 1, // The handle or object doesn't exist.
	KRES_HANDLE_DELETED  = 2, // The handle has been deleted externally.
	//KRES_WAIT_QUEUE_FULL = 3, // The wait queue is full. We can't block on it.
	KRES_WOULD_BLOCK     = 3, // The function would block. For non-blocking APIs.
	KRES_NO_PERMISSIONS  = 4  // You have no permissions. Example unlocking a mutex on a different task.
};

typedef uintptr_t KRes; // See createTask() implementation.
typedef uintptr_t KHandle;
typedef void (*TaskFunc)(void*);



/**
 * @brief      Initializes the kernel. Only call this once.
 *
 * @param[in]  priority  The priority of the main task.
 */
void kernelInit(uint8_t priority);


/**
 * @brief      Creates a new kernel task.
 *
 * @param[in]  stackSize  The stack size.
 * @param[in]  priority   The priority.
 * @param[in]  entry      The entry function.
 * @param      taskArg    The task entry function argument.
 *
 * @return     Returns a KHandle for the created task or NULL on error.
 */
KHandle createTask(size_t stackSize, uint8_t priority, TaskFunc entry, void *taskArg);

/**
 * @brief      Switches to the next task. Use with care.
 */
void yieldTask(void);

/**
 * @brief      Task exit function. Must be called from the task that exits.
 */
void taskExit(void);

#ifdef __cplusplus
} // extern "C"
#endif
