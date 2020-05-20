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

#include <stdatomic.h>
#include "mem_map.h"
#include "types.h"
#include "arm9/hardware/timer.h"
#include "arm9/hardware/interrupt.h"
#include "arm.h"


#define TIMER_REGS_BASE   (IO_MEM_ARM9_ONLY + 0x3000)
#define REG_TIMER0_VAL    *((vu16*)(TIMER_REGS_BASE + 0x00))
#define REG_TIMER0_CNT    *((vu16*)(TIMER_REGS_BASE + 0x02))

#define REG_TIMER1_VAL    *((vu16*)(TIMER_REGS_BASE + 0x04))
#define REG_TIMER1_CNT    *((vu16*)(TIMER_REGS_BASE + 0x06))

#define REG_TIMER2_VAL    *((vu16*)(TIMER_REGS_BASE + 0x08))
#define REG_TIMER2_CNT    *((vu16*)(TIMER_REGS_BASE + 0x0A))

#define REG_TIMER3_VAL    *((vu16*)(TIMER_REGS_BASE + 0x0C))
#define REG_TIMER3_CNT    *((vu16*)(TIMER_REGS_BASE + 0x0E))

#define REG_TIMER_VAL(n)  *((vu16*)(TIMER_REGS_BASE + 0x00 + ((n) * 4)))
#define REG_TIMER_CNT(n)  *((vu16*)(TIMER_REGS_BASE + 0x02 + ((n) * 4)))


// For TIMER_sleep()
static u32 overflows;



static void timerSleepHandler(UNUSED u32 id);

void TIMER_init(void)
{
	for(u32 i = 0; i < 4; i++)
	{
		REG_TIMER_CNT(i) = 0;
	}

	IRQ_registerIsr(IRQ_TIMER_3, timerSleepHandler);
}

void TIMER_start(Timer timer, TimerPrescaler prescaler, u16 ticks, bool enableIrq)
{
	REG_TIMER_VAL(timer) = ticks;
	REG_TIMER_CNT(timer) = TIMER_ENABLE | (enableIrq ? TIMER_IRQ_ENABLE : 0) | prescaler;
}

u16 TIMER_getTicks(Timer timer)
{
	return REG_TIMER_VAL(timer);
}

u16 TIMER_stop(Timer timer)
{
	REG_TIMER_CNT(timer) = 0;

	return REG_TIMER_VAL(timer);
}

void TIMER_sleep(u32 ms)
{
	REG_TIMER3_VAL = TIMER_FREQ_64(1000);
	overflows = ms;
	atomic_signal_fence(memory_order_release); // Don't move the write to overflows.

	REG_TIMER3_CNT = TIMER_ENABLE | TIMER_IRQ_ENABLE | TIMER_PRESCALER_64;

	do
	{
		__wfi();
	} while(REG_TIMER3_CNT & TIMER_ENABLE);
}

static void timerSleepHandler(UNUSED u32 id)
{
	u32 tmp = overflows;
	if(!--tmp)
	{
		REG_TIMER3_CNT = 0;
		return;
	}
	overflows = tmp;
}
