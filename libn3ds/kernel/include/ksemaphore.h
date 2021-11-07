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

#include <stdint.h>
#include <stdbool.h>
#include "kernel.h"


#ifdef __cplusplus
extern "C"
{
#endif



/**
 * @brief      Creates a new kernel semaphore.
 *
 * @param[in]  count  The initial count of the semaphore.
 *
 * @return     The KHandle of the semaphore or NULL on error.
 */
KHandle createSemaphore(int32_t count);

/**
 * @brief      Deletes a kernel semaphore.
 *
 * @param[in]  ksema  The KHandle of the semaphore.
 */
void deleteSemaphore(KHandle const ksema);

/**
 * @brief      Polls a kernel semaphore.
 *
 * @param[in]  ksema  The KHandle of the semaphore.
 *
 * @return     Returns KRES_OK or KRES_WOULD_BLOCK.
 */
KRes pollSemaphore(KHandle const ksema);

/**
 * @brief      Decreases the kernel semaphore and blocks if <=0.
 *
 * @param[in]  ksema  The KHandle of the semaphore.
 *
 * @return     Returns the result. See Kres in kernel.h.
 */
KRes waitForSemaphore(KHandle const ksema);

/**
 * @brief      Increases the kernel semaphore and wakes up signalCount waiting tasks if any.
 *
 * @param[in]  ksema        The KHandle of the semaphore.
 * @param[in]  signalCount  The number of tasks to wake up..
 * @param[in]  reschedule   Set to true to immediately reschedule.
 */
void signalSemaphore(KHandle const ksema, uint32_t signalCount, bool reschedule);

#ifdef __cplusplus
} // extern "C"
#endif
