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


#define TIMER_BASE_FREQ    (268111856.f)

#define TIMER_ENABLE       (1u)
#define TIMER_SINGLE_SHOT  (0u)
#define TIMER_AUTO_RELOAD  (1u<<1)
#define TIMER_IRQ_ENABLE   (1u<<2)

// p is the prescaler value and n the frequence
#define TIMER_FREQ(p, f)  (TIMER_BASE_FREQ / 2 / (p) / (f))



/**
 * @brief      Resets/initializes the timer hardware. Should not be called manually.
 */
void TIMER_init(void);

/**
 * @brief      Starts the timer.
 *
 * @param[in]  prescaler   The prescaler value.
 * @param[in]  ticks       The initial number of ticks. This is also the
 *                         reload value in auto reload mode.
 * @param[in]  autoReload  Set to true for auto reload. false for single shot.
 * @param[in]  enableIrq   Timer fires IRQs on underflow if true.
 */
void TIMER_start(u8 prescaler, u32 ticks, bool autoReload, bool enableIrq);

/**
 * @brief      Returns the current number of ticks of the timer.
 *
 * @return     The number of ticks.
 */
u32 TIMER_getTicks(void);

/**
 * @brief      Stops the timer and returns the current number of ticks.
 *
 * @return     The number of ticks.
 */
u32 TIMER_stop(void);

/**
 * @brief      Halts the CPU for the specified number of ticks.
 *             Use the function below for a milliseconds version.
 *
 * @param[in]  ticks  The number of ticks to sleep.
 */
void TIMER_sleepTicks(u32 ticks);


// Sleeps ms milliseconds. ms can be up to 32000.
static inline void TIMER_sleepMs(u32 ms)
{
	TIMER_sleepTicks(TIMER_FREQ(1, 1000) * ms);
}
