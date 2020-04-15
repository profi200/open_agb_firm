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
#include "arm11/hardware/interrupt.h"
#include "arm.h"
#include "arm11/hardware/cfg11.h"


// First 32 interrupts are private to each core (4 * 32).
// 96 external interrupts (total 128).
IrqHandler irqHandlerTable[224] = {0};



void IRQ_init(void)
{
	REG_CPU_II_CNT = 0; // Disable the interrupt interface for this CPU.

	if(!__getCpuId()) // Core 0
	{
		REG_CFG11_FIQ_CNT = 2; // Disable FIQs.

		REG_GID_CNT = 0; // Disable the global interrupt distributor.

		// Disable all 128 interrupts.
		REGs_GID_ENA_CLR[0] = 0xFFFFFFFFu; // Interrupts 0-15 cant be disabled.
		REGs_GID_ENA_CLR[1] = 0xFFFFFFFFu;
		REGs_GID_ENA_CLR[2] = 0xFFFFFFFFu;
		REGs_GID_ENA_CLR[3] = 0xFFFFFFFFu;

		// Set all pending interrupts to inactive state.
		REGs_GID_PEN_CLR[0] = 0xFFFFFFFFu; // Interrupt 0-15 can't be set to inactive apparently.
		REGs_GID_PEN_CLR[1] = 0xFFFFFFFFu;
		REGs_GID_PEN_CLR[2] = 0xFFFFFFFFu;
		REGs_GID_PEN_CLR[3] = 0xFFFFFFFFu;

		// Set all 128 interrupts to lowest priority.
		for(u32 i = 0; i < 32; i++) REGs_GID_IPRIO[i] = 0xE0E0E0E0u;

		// Set all 128 interrupts to target no CPU.
		// Interrupt 0-31 can't be changed.
		for(u32 i = 8; i < 32; i++) REGs_GID_ITARG[i] = 0;

		// Set all interrupts to rising edge sensitive and 1-N software model.
		for(u32 i = 0; i < 8; i++) REGs_GID_ICONF[i] = 0xFFFFFFFFu;

		REG_GID_CNT = 1; // Enable the global interrupt distributor.
	}
	else // Other core. Same as above but for core specific IRQs.
	{
		REGs_GID_ENA_CLR[0] = 0xFFFFFFFFu;

		REGs_GID_PEN_CLR[0] = 0xFFFFFFFFu;

		for(u32 i = 0; i < 8; i++) REGs_GID_IPRIO[i] = 0xE0E0E0E0u;

		REGs_GID_ICONF[0] = 0xFFFFFFFFu;
		REGs_GID_ICONF[1] = 0xFFFFFFFFu;
	}


	REG_CPU_II_MASK = 0xF0; // Mask no interrupt.
	REG_CPU_II_BIN_POI = 3; // All priority bits are compared for pre-emption.
	REG_CPU_II_CNT = 1;     // Enable the interrupt interface for this CPU.

	// Get rid of all interrupts stuck in pending/active state.
	u32 tmp;
	do
	{
		tmp = REG_CPU_II_AKN; // Aknowledge
		REG_CPU_II_EOI = tmp; // End of interrupt
	} while(tmp != 1023);
}

void IRQ_registerHandler(Interrupt id, u8 prio, u8 cpuMask, bool edgeTriggered, IrqHandler handler)
{
	const u32 cpuId = __getCpuId();
	if(!cpuMask) cpuMask = 1u<<cpuId;

	const u32 oldState = enterCriticalSection();

	irqHandlerTable[(id < 32 ? 32 * cpuId + id : 96u + id)] = handler;

	// Priority
	u32 shift = (id % 4 * 8) + 4;
	u32 tmp = REGs_GID_IPRIO[id>>2] & ~(0xFu<<shift);
	REGs_GID_IPRIO[id>>2] = tmp | (u32)prio<<shift;

	// Target
	shift = id % 4 * 8;
	tmp = REGs_GID_ITARG[id>>2] & ~(0xFu<<shift);
	REGs_GID_ITARG[id>>2] = tmp | (u32)cpuMask<<shift;

	// Trigger level
	shift = (id % 16 * 2) + 1;
	tmp = REGs_GID_ICONF[id>>4] & ~(1u<<shift);
	REGs_GID_ICONF[id>>4] = tmp | (u32)edgeTriggered<<shift;

	// Enable it.
	REGs_GID_ENA_SET[id>>5] = 1u<<(id % 32);

	leaveCriticalSection(oldState);
}

void IRQ_unregisterHandler(Interrupt id)
{
	const u32 oldState = enterCriticalSection();

	REGs_GID_ENA_CLR[id>>5] = 1u<<(id % 32);

	irqHandlerTable[(id < 32 ? 32 * __getCpuId() + id : 96u + id)] = (IrqHandler)NULL;

	leaveCriticalSection(oldState);
}

void IRQ_enable(Interrupt id)
{
	REGs_GID_ENA_SET[id>>5] = 1u<<(id % 32);
}

void IRQ_disable(Interrupt id)
{
	REGs_GID_ENA_CLR[id>>5] = 1u<<(id % 32);
}

void IRQ_setPriority(Interrupt id, u8 prio)
{
	const u32 oldState = enterCriticalSection();

	const u32 shift = (id % 4 * 8) + 4;
	u32 tmp = REGs_GID_IPRIO[id>>2] & ~(0xFu<<shift);
	REGs_GID_IPRIO[id>>2] = tmp | (u32)prio<<shift;

	leaveCriticalSection(oldState);
}

void IRQ_softwareInterrupt(Interrupt id, u8 cpuMask)
{
	REG_GID_SW_INT = (u32)cpuMask<<16 | id;
}
