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

#include "types.h"


#define TIMER_BASE_FREQ     (67027964)

#define TIMER_COUNT_UP      (1u<<2) // For cascading at least 2 timers
#define TIMER_IRQ_ENABLE    (1u<<6)
#define TIMER_ENABLE        (1u<<7)

// Convenience macros for calculating the ticks. Based on libnds
#define TIMER_FREQ(n)       (-TIMER_BASE_FREQ / (n))
#define TIMER_FREQ_64(n)    (-(TIMER_BASE_FREQ / 64) / (n))
#define TIMER_FREQ_256(n)   (-(TIMER_BASE_FREQ / 256) / (n))
#define TIMER_FREQ_1024(n)  (-(TIMER_BASE_FREQ / 1024) / (n))


typedef enum
{
	TIMER_0 = 0u,
	TIMER_1 = 1u,
	TIMER_2 = 2u,
	TIMER_3 = 3u
} Timer;

typedef enum
{
	TIMER_PRESCALER_1    = 0u,
	TIMER_PRESCALER_64   = 1u,
	TIMER_PRESCALER_256  = 2u,
	TIMER_PRESCALER_1024 = 3u
} TimerPrescaler;



/**
 * @brief      Resets/initializes the timer hardware. Should not be called manually.
 */
void TIMER_init(void);

/**
 * @brief      Starts a timer.
 *
 * @param[in]  timer      The timer to start.
 * @param[in]  prescaler  The prescaler to use.
 * @param[in]  ticks      The initial number of ticks. This is also the reload
 *                        value on overflow.
 * @param[in]  enableIrq  Timer fires IRQs if true.
 */
void TIMER_start(Timer timer, TimerPrescaler prescaler, u16 ticks, bool enableIrq);

/**
 * @brief      Returns the current number of ticks of the timer.
 *
 * @param[in]  timer  The timer get the ticks from.
 *
 * @return     The number of ticks.
 */
u16 TIMER_getTicks(Timer timer);

/**
 * @brief      Stops a timer and returns the current number of ticks.
 *
 * @param[in]  timer  The timer to stop.
 *
 * @return     The number of ticks.
 */
u16 TIMER_stop(Timer timer);

/**
 * @brief      Halts the CPU for the specified number of milliseconds.
 *
 * @param[in]  ms    The number of milliseconds to sleep.
 */
void TIMER_sleep(u32 ms);
