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

typedef struct KSema KSema;



/**
 * @brief      Creates a new KSema.
 *
 * @param[in]  count  The initial count of the semaphore.
 *
 * @return     The KSema pointer or NULL when out of memory.
 */
KSema* createSemaphore(int32_t count);

/**
 * @brief      Deletes a KSema.
 *
 * @param[in]  ksema  The KSema handle.
 */
void deleteSemaphore(KSema *const ksema);

/**
 * @brief      Polls a KSema.
 *
 * @param[in]  ksema  The KSema pointer.
 *
 * @return     Returns KRES_OK or KRES_WOULD_BLOCK.
 */
KRes pollSemaphore(KSema *const ksema);

/**
 * @brief      Decreases the semaphore and blocks if <=0.
 *
 * @param[in]  ksema  The KSema pointer.
 *
 * @return     Returns the result. See Kres above.
 */
KRes waitForSemaphore(KSema *const ksema);

/**
 * @brief      Increases the semaphore and wakes up signalCount waiting tasks if any.
 *
 * @param[in]  ksema        The KSema pointer.
 * @param[in]  signalCount  The number to increase the semaphore by.
 * @param[in]  reschedule   Set to true to immediately reschedule.
 */
void signalSemaphore(KSema *const ksema, uint32_t signalCount, bool reschedule);

#ifdef __cplusplus
} // extern "C"
#endif
