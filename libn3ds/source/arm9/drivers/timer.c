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

#include <stdatomic.h>
#include "types.h"
#include "arm9/drivers/timer.h"
#include "arm9/drivers/interrupt.h"
#include "arm.h"


// For TIMER_sleepMs().
static u32 g_overflows;



static void timerSleepHandler(UNUSED u32 id);

void TIMER_init(void)
{
	for(u32 i = 0; i < 4; i++)
	{
		getTimerRegs(i)->cnt = 0;
	}

	IRQ_registerIsr(IRQ_TIMER_3, timerSleepHandler);
}

void TIMER_start(u8 tmr, u16 ticks, u8 params)
{
	Timer *const timer = getTimerRegs(tmr);
	timer->val = ticks;
	timer->cnt = TIMER_EN | params;
}

u16 TIMER_getTicks(u8 tmr)
{
	return getTimerRegs(tmr)->val;
}

u16 TIMER_stop(u8 tmr)
{
	Timer *const timer = getTimerRegs(tmr);
	timer->cnt = 0;

	return timer->val;
}

void TIMER_sleepMs(u32 ms)
{
	Timer *const timer = getTimerRegs(3);
	timer->val = TIMER_FREQ_64(1000);
	g_overflows = ms;
	atomic_signal_fence(memory_order_release); // Don't move the write to g_overflows.

	timer->cnt = TIMER_EN | TIMER_IRQ_EN | TIMER_PRESC_64;

	do
	{
		__wfi();
	} while(timer->cnt & TIMER_EN);
}

static void timerSleepHandler(UNUSED u32 id)
{
	u32 tmp = g_overflows;
	if(--tmp == 0)
	{
		getTimerRegs(3)->cnt = 0;
		return;
	}
	g_overflows = tmp;
}
