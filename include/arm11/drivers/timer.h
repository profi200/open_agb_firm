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

#include <assert.h>
#include "types.h"
#include "mem_map.h"


#define TIMER_REGS_BASE  (MPCORE_PRIV_REG_BASE + 0x600)

typedef struct
{
	vu32 load;          // 0x00
	vu32 counter;       // 0x04
	vu32 cnt;           // 0x08
	vu32 int_stat;      // 0x0C
	u8 _0x10[0x10];
	vu32 wd_load;       // 0x20
	vu32 wd_counter;    // 0x24
	vu32 wd_cnt;        // 0x28
	vu32 wd_int_stat;   // 0x2C
	vu32 wd_reset_stat; // 0x30
	vu32 wd_disable;    // 0x34
} Timer;
static_assert(offsetof(Timer, wd_disable) == 0x34, "Error: Member wd_disable of Timer is not at offset 0x34!");

ALWAYS_INLINE Timer* getTimerRegs(void)
{
	return (Timer*)TIMER_REGS_BASE;
}


// REG_TIMER_CNT/REG_WD_CNT
#define TIMER_EN           (1u)
#define TIMER_SINGLE_SHOT  (0u)
#define TIMER_AUTO_RELOAD  (1u<<1)
#define TIMER_IRQ_EN       (1u<<2)
#define WD_TIMER_MODE      (0u)    // Watchdog only. Watchdog in timer mode.
#define WD_WD_MODE         (1u<<3) // Watchdog only. Watchdog in watchdog mode.
#define TIMER_PRESC_SHIFT  (8u)    // Shift helper for the prescalers.

// REG_WD_DISABLE
#define WD_DISABLE_MAGIC1  (0x12345678u)
#define WD_DISABLE_MAGIC2  (0x87654321u)


#define TIMER_BASE_FREQ  (268111856.f)

// p is the prescaler value and n the frequency.
// Note: With highest prescaler and sub-microsecond frequency
//       this may give wrong results due to overflows.
#define TIMER_FREQ(p, f)  (TIMER_BASE_FREQ / (2 * (p) * (f)))



/**
 * @brief      Resets/initializes the timer hardware. Should not be called manually.
 */
void TIMER_init(void);

/**
 * @brief      Starts the timer.
 *
 * @param[in]  prescaler  The prescaler (1-256).
 * @param[in]  ticks      The initial number of ticks. This is also the reload
 *                        value in auto reload mode.
 * @param[in]  params     The parameters. See /regs/timer.h "REG_TIMER_CNT" defines.
 */
void TIMER_start(u16 prescaler, u32 ticks, u8 params);

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
 * @brief      Halts the CPU for the given number of ticks.
 *             Use the function below for milliseconds.
 *
 * @param[in]  ticks  The number of ticks to sleep.
 */
void TIMER_sleepTicks(u32 ticks);


// Sleeps ms milliseconds. ms can be up to 32000.
// TODO: Should this be inline? Generates a bunch of vfp code.
ALWAYS_INLINE void TIMER_sleepMs(u32 ms)
{
	TIMER_sleepTicks(TIMER_FREQ(1, 1000) * ms);
}
