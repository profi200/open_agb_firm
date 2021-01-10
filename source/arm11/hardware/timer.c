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
#include "mem_map.h"
#include "arm11/hardware/timer.h"
#include "arm11/hardware/interrupt.h"
#include "fb_assert.h"
#include "arm.h"


#define TIMER_REGS_BASE     (MPCORE_PRIV_REG_BASE + 0x600)
#define REG_TIMER_LOAD      *((vu32*)(TIMER_REGS_BASE + 0x00))
#define REG_TIMER_COUNTER   *((vu32*)(TIMER_REGS_BASE + 0x04))
#define REG_TIMER_CNT       *((vu32*)(TIMER_REGS_BASE + 0x08))
#define REG_TIMER_INT_STAT  *((vu32*)(TIMER_REGS_BASE + 0x0C))



void TIMER_init(void)
{
	REG_TIMER_CNT = 0;
	REG_TIMER_INT_STAT = 1;

	IRQ_registerIsr(IRQ_TIMER, 12, 0, NULL);
}

void TIMER_start(u8 prescaler, u32 ticks, bool autoReload, bool enableIrq)
{
	fb_assert(prescaler > 0);

	REG_TIMER_LOAD = ticks;
	REG_TIMER_CNT = (prescaler - 1u)<<8 | (enableIrq ? TIMER_IRQ_ENABLE : 0u) |
	                (autoReload ? TIMER_AUTO_RELOAD : TIMER_SINGLE_SHOT) | TIMER_ENABLE;
}

u32 TIMER_getTicks(void)
{
	return REG_TIMER_COUNTER;
}

u32 TIMER_stop(void)
{
	REG_TIMER_CNT = 0;
	REG_TIMER_INT_STAT = 1;

	return REG_TIMER_COUNTER;
}

void TIMER_sleepTicks(u32 ticks)
{
	REG_TIMER_LOAD = ticks;
	REG_TIMER_CNT = TIMER_IRQ_ENABLE | TIMER_SINGLE_SHOT | TIMER_ENABLE;

	do
	{
		__wfi();
	} while(REG_TIMER_COUNTER); // Checking the REG_TIMER_CNT enable bit is broken.

	REG_TIMER_INT_STAT = 1;
}
