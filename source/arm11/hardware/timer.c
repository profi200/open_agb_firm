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

#include "types.h"
#include "arm11/hardware/timer.h"
#include "arm11/hardware/interrupt.h"
#include "fb_assert.h"
#include "arm.h"



void TIMER_init(void)
{
	// Timer.
	Timer *const timer = getTimerRegs();
	timer->cnt      = 0;
	timer->int_stat = 1;

	// Watchdog.
	timer->wd_cnt      = 0;
	timer->wd_disable  = WD_DISABLE_MAGIC1;
	timer->wd_disable  = WD_DISABLE_MAGIC2;
	timer->wd_int_stat = 1;

	IRQ_registerIsr(IRQ_TIMER, 12, 0, NULL);
}

void TIMER_start(u16 prescaler, u32 ticks, u8 params)
{
	fb_assert(prescaler > 0 && prescaler <= 256);

	Timer *const timer = getTimerRegs();
	timer->load = ticks;
	timer->cnt = (prescaler - 1)<<8 | params | TIMER_ENABLE;
}

u32 TIMER_getTicks(void)
{
	return getTimerRegs()->counter;
}

u32 TIMER_stop(void)
{
	Timer *const timer = getTimerRegs();
	timer->cnt = 0;
	timer->int_stat = 1;

	return timer->counter;
}

void TIMER_sleepTicks(u32 ticks)
{
	Timer *const timer = getTimerRegs();
	timer->load = ticks;
	timer->cnt = TIMER_IRQ_ENABLE | TIMER_SINGLE_SHOT | TIMER_ENABLE; // Prescaler 1.

	do
	{
		__wfi();
	} while(timer->counter); // Checking the REG_TIMER_CNT enable bit is broken.

	timer->int_stat = 1;
}
