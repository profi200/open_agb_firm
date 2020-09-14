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

#include <stdbool.h>
#include "kernel.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct KEvent KEvent;



/**
 * @brief      Creates a new KEvent.
 *
 * @param[in]  oneShot  Event fires only once and auto clears if true.
 *
 * @return     The KEvent pointer or NULL when out of memory.
 */
KEvent* createEvent(bool oneShot);

/**
 * @brief      Deletes a KEvent.
 *
 * @param[in]  kevent  The KEvent pointer.
 */
void deleteEvent(KEvent *const kevent);

/**
 * @brief      Binds the given KEvent to an interrupt.
 *
 * @param[in]  kevent  The KEvent pointer.
 * @param[in]  id      The interrupt id.
 * @param[in]  prio    The interrupt priority.
 */
void bindInterruptToEvent(KEvent *const kevent, uint8_t id, uint8_t prio);

void unbindInterruptEvent(uint8_t id);

/**
 * @brief      Waits for the given KEvent to be signaled.
 *
 * @param[in]  kevent  The KEvent pointer.
 *
 * @return     Returns the result. See Kres above.
 */
KRes waitForEvent(KEvent *const kevent);

/**
 * @brief      Signals a KEvent.
 *
 * @param[in]  kevent      The KEvent pointer.
 * @param[in]  reschedule  Set to true to immediately reschedule.
 */
void signalEvent(KEvent *const kevent, bool reschedule);

/**
 * @brief      Clears a KEvent.
 *
 * @param[in]  kevent  The KEvent pointer.
 */
void clearEvent(KEvent *const kevent);

#ifdef __cplusplus
} // extern "C"
#endif
