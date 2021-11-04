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

#include <stdbool.h>
#include "kernel.h"


#ifdef __cplusplus
extern "C"
{
#endif



/**
 * @brief      Creates a new kernel event.
 *
 * @param[in]  oneShot  Event fires only once and auto clears if true.
 *
 * @return     The KHandle for the event or NULL on error.
 */
KHandle createEvent(bool oneShot);

/**
 * @brief      Deletes an kernel event.
 *
 * @param[in]  kevent  The KHandle of the event to delete.
 */
void deleteEvent(KHandle const kevent);

/**
 * @brief      Binds the given kernel event to an interrupt.
 *
 * @param[in]  kevent  The KHandle of the event.
 * @param[in]  id      The interrupt id.
 * @param[in]  prio    The interrupt priority.
 */
void bindInterruptToEvent(KHandle const kevent, uint8_t id, uint8_t prio);

void unbindInterruptEvent(uint8_t id);

/**
 * @brief      Waits for a kernel event to be signaled.
 *
 * @param[in]  kevent  The KHandle of the event.
 *
 * @return     Returns the result. See Kres in kernel.h.
 */
KRes waitForEvent(KHandle const kevent);

/**
 * @brief      Signals an kernel event.
 *
 * @param[in]  kevent      The KHandle of the event.
 * @param[in]  reschedule  Set to true to immediately reschedule.
 */
void signalEvent(KHandle const kevent, bool reschedule);

/**
 * @brief      Clears an kernel event.
 *
 * @param[in]  kevent  The KHandle of the event.
 */
void clearEvent(KHandle const kevent);

#ifdef __cplusplus
} // extern "C"
#endif
