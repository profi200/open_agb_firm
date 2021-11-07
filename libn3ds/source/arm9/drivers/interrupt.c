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
#include "arm9/drivers/interrupt.h"


#define IRQ_REGS_BASE  (IO_MEM_ARM9_ONLY + 0x1000)
#define REG_IRQ_IE     *((vu32*)(IRQ_REGS_BASE + 0x00))
#define REG_IRQ_IF     *((vu32*)(IRQ_REGS_BASE + 0x04))


IrqIsr g_irqIsrTable[32] = {0};



void IRQ_init(void)
{
	REG_IRQ_IE = 0;
	REG_IRQ_IF = 0xFFFFFFFFu;
}

void IRQ_registerIsr(Interrupt id, IrqIsr isr)
{
	const u32 oldState = enterCriticalSection();

	g_irqIsrTable[id] = isr;
	REG_IRQ_IE |= 1u<<id;

	leaveCriticalSection(oldState);
}

void IRQ_unregisterIsr(Interrupt id)
{
	const u32 oldState = enterCriticalSection();

	REG_IRQ_IE &= ~(1u<<id);
	g_irqIsrTable[id] = (IrqIsr)NULL;

	leaveCriticalSection(oldState);
}
