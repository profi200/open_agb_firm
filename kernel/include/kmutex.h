#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
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

#include "kernel.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct KMutex KMutex; // TODO: Implement this using semaphores?



/**
 * @brief      Creates a new KMutex.
 *
 * @return     The KMutex pointer or NULL when out of memory.
 */
KMutex* createMutex(void);

/**
 * @brief      Deletes a KMutex.
 *
 * @param[in]  kmutex  The KMutex pointer.
 */
void deleteMutex(KMutex *const kmutex);

/**
 * @brief      Locks a KMutex.
 *
 * @param[in]  kmutex  The KMutex pointer.
 *
 * @return     Returns the result. See Kres.
 */
KRes lockMutex(KMutex *const kmutex);

/**
 * @brief      Unlocks a KMutex.
 *
 * @param[in]  kmutex  The KMutex pointer.
 *
 * @return     Returns KRES_NO_PERMISSIONS if the current task
 * @return     is not the owner. Otherwise KRES_OK.
 */
KRes unlockMutex(KMutex *const kmutex);

#ifdef __cplusplus
} // extern "C"
#endif
