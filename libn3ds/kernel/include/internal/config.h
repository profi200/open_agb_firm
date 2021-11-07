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

// MAX_PRIO_BITS   The number of available priorities. Minimum 3. Maximum 32.
#define MAX_PRIO_BITS    (4)

/*
 * Maximum number of objects we can create (Slabheap).
*/
#define MAX_TASKS        (3) // Including main and idle task.
#define MAX_EVENTS       (10)
#define MAX_MUTEXES      (3)
#define MAX_SEMAPHORES   (0)
#define MAX_TIMERS       (0)

#define IDLE_STACK_SIZE  (0x1000) // Keep in mind this stack is used in interrupt contex! TODO: Change this.



// TODO: More checks. For example slabheap.
#if (MAX_PRIO_BITS < 3 || MAX_PRIO_BITS > 32)
	#error "Invalid number of maximum task priorities!"
#endif
