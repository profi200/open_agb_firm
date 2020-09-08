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

#include <stdint.h>
#include <stdbool.h>
#include "kernel.h"


typedef void* KSema;



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief      Creates a KSema handle.
 *
 * @param[in]  count  The initial count of the semaphore.
 *
 * @return     The KSema handle or NULL when out of memory.
 */
KSema createSemaphore(int32_t count);

/**
 * @brief      Deletes a KSema handle.
 *
 * @param[in]  ksema  The KSema handle.
 */
void deleteSemaphore(const KSema ksema);

/**
 * @brief      Polls a KSema.
 *
 * @param[in]  ksema  The KSema handle.
 *
 * @return     Returns KRES_OK or KRES_WOULD_BLOCK.
 */
KRes pollSemaphore(const KSema ksema);

/**
 * @brief      Decreases the semaphore and blocks if <=0.
 *
 * @param[in]  ksema  The KSema handle.
 *
 * @return     Returns the result. See Kres above.
 */
KRes waitForSemaphore(const KSema ksema);

/**
 * @brief      Increases the semaphore and wakes up signalCount waiting tasks if any.
 *
 * @param[in]  ksema        The KSema handle.
 * @param[in]  signalCount  The number to increase the semaphore by.
 * @param[in]  reschedule   Set to true to immediately reschedule.
 */
void signalSemaphore(const KSema ksema, uint32_t signalCount, bool reschedule);

#ifdef __cplusplus
} // extern "C"
#endif
