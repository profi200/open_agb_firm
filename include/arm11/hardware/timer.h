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
	vu32 load;
	vu32 counter;
	vu32 cnt;
	vu32 int_stat;
	u8 _0x10[0x10];
	vu32 wd_load;
	vu32 wd_counter;
	vu32 wd_cnt;
	vu32 wd_int_stat;
	vu32 wd_reset_stat;
	vu32 wd_disable;
} Timer;
static_assert(offsetof(Timer, wd_disable) == 0x34, "Error: Member wd_disable of Timer is not at offset 0x34!");

ALWAYS_INLINE Timer* getTimerRegs(void)
{
	return (Timer*)TIMER_REGS_BASE;
}


#define TIMER_BASE_FREQ    (268111856.f)

// REG_TIMER_CNT/REG_WD_CNT
#define TIMER_ENABLE       (1u)
#define TIMER_SINGLE_SHOT  (0u)
#define TIMER_AUTO_RELOAD  (1u<<1)
#define TIMER_IRQ_ENABLE   (1u<<2)
#define WD_TIMER_MODE      (0u)    // Watchdog only. Watchdog in timer mode.
#define WD_WD_MODE         (1u<<3) // Watchdog only. Watchdog in watchdog mode.

// REG_WD_DISABLE
#define WD_DISABLE_MAGIC1  (0x12345678u)
#define WD_DISABLE_MAGIC2  (0x87654321u)

// p is the prescaler value and n the frequence.
#define TIMER_FREQ(p, f)  (TIMER_BASE_FREQ / 2 / (p) / (f))



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
 * @param[in]  params     The parameters. See REG_TIMER_CNT defines above.
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
 * @brief      Halts the CPU for the specified number of ticks.
 *             Use the function below for a milliseconds version.
 *
 * @param[in]  ticks  The number of ticks to sleep.
 */
void TIMER_sleepTicks(u32 ticks);


// Sleeps ms milliseconds. ms can be up to 32000.
ALWAYS_INLINE void TIMER_sleepMs(u32 ms)
{
	TIMER_sleepTicks(TIMER_FREQ(1, 1000) * ms);
}
