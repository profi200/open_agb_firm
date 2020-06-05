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
#include "arm11/hardware/cpu.h"
#include "arm.h"
#include "arm11/hardware/interrupt.h"
#include "arm11/hardware/cfg11.h"
#include "arm11/start.h"
#include "util.h"
#include "arm11/hardware/scu.h"


#ifdef CORE123_INIT
static void NAKED core23Entry(void)
{
	__cpsid(aif);
	REG_GIC_CPU_CTRL = 1;

	const u32 cpuId = __getCpuId();
	// Tell core 0 we are here
	if(cpuId == 3) REGs_CFG11_MPCORE_BOOTCNT[3] = 1;
	else           REGs_CFG11_MPCORE_BOOTCNT[2] = 1;

	// Wait for IPI 2 (core 2) or IPI 3 (core 3)
	u32 tmp;
	do
	{
		__wfi();
		tmp = REG_GIC_CPU_INTACK;
		REG_GIC_CPU_EOI = tmp;
	} while(tmp != cpuId);

	// Jump to real entrypoint
	_start();
}
#endif

void core123Init(void)
{
	if(REG_CFG11_SOCINFO & 2)
	{
		REG_GIC_CPU_CTRL = 1;
		for(u32 i = 0; i < 4; i++) REGs_GIC_DIST_ENABLE_CLEAR[i] = 0xFFFFFFFFu;
		REGs_GIC_DIST_PENDING_CLEAR[2] = 0x1000000; // Interrupt ID 88
		REGs_GIC_DIST_PRI[22] = 0;
		REGs_GIC_DIST_TARGET[22] = 1;
		REGs_GIC_DIST_ENABLE_SET[2] = 0x1000000;

#ifdef CORE123_INIT
		u16 clkCnt;
		// If clock modifier is 3x use clock 3x. Also enables FCRAM extension?
		if(REG_CFG11_SOCINFO & 4) clkCnt = 2<<1 | 1;
		else                      clkCnt = 1<<1 | 1;

		if((REG_CFG11_MPCORE_CLKCNT & 7) != clkCnt)
		{
			// No idea what this does
			if(REG_CFG11_SOCINFO & 4) REG_CFG11_MPCORE_CNT = 0x101;
			else                      REG_CFG11_MPCORE_CNT = 1;

			// Necessary delay
			wait(403);

			CPU_setClock(clkCnt);
			REGs_GIC_DIST_PENDING_CLEAR[2] = 0x1000000;
			REG_UNK_10140400 = 3;   // Clock related?
		}
		REG_UNK_10140410 = 0x3FFFF; // Clock related?

		if((REG_SCU_CONFIG & 3) == 3)
		{
			// Set core 2/3 to normal mode (running)
			REG_SCU_CPU_STAT &= ~0b11110000;

			const u16 clkCnt = REG_CFG11_MPCORE_CLKCNT & 7;
			u16 tmpClkCnt;
			if(REG_CFG11_SOCINFO & 4) tmpClkCnt = 0<<1 | 1;
			else                      tmpClkCnt = 1<<1 | 0;

			if(clkCnt != tmpClkCnt)
			{
				CPU_setClock(tmpClkCnt);
				REGs_GIC_DIST_PENDING_CLEAR[2] = 0x1000000;
			}

			REG_CFG11_BOOTROM_OVERLAY_CNT = 1;
			REG_CFG11_BOOTROM_OVERLAY_VAL = (u32)core23Entry;
			// If not already done enable instruction and data overlays
			if(!(REGs_CFG11_MPCORE_BOOTCNT[2] & 0x10)) REGs_CFG11_MPCORE_BOOTCNT[2] = 3;
			if(!(REGs_CFG11_MPCORE_BOOTCNT[3] & 0x10)) REGs_CFG11_MPCORE_BOOTCNT[3] = 3;
			// Wait for core 2/3 to jump out of boot11
			while((REGs_CFG11_MPCORE_BOOTCNT[2] & 0x12) != 0x10);
			while((REGs_CFG11_MPCORE_BOOTCNT[3] & 0x12) != 0x10);
			REG_CFG11_BOOTROM_OVERLAY_CNT = 0; // Disable all overlays

			// Set clock back to original one
			if(clkCnt != tmpClkCnt) CPU_setClock(clkCnt);
		}

		REGs_GIC_DIST_ENABLE_CLEAR[2] = 0x1000000;

		// Wakeup core 2/3 and let them jump to their entrypoint.
		IRQ_softwareInterrupt(2, 0b0100);
		IRQ_softwareInterrupt(3, 0b1000);
#else
		// Just enables the New 3DS FCRAM extension (if not already done)
		if((REG_CFG11_MPCORE_CLKCNT & 7) != 1) CPU_setClock(1);

		REGs_GIC_DIST_ENABLE_CLEAR[2] = 0x1000000;
#endif
	}

	// Wakeup core 1
	*((vu32*)0x1FFFFFDC) = (u32)_start;  // Core 1 entrypoint
	IRQ_softwareInterrupt(1, 0b0010);
}

void CPU_setClock(u16 clk)
{
	REG_CFG11_MPCORE_CLKCNT = 0x8000 | (clk & 7);
	do
	{
		__wfi();
	} while(!(REG_CFG11_MPCORE_CLKCNT & 0x8000));
}

void CPU_poweroffCore23(void)
{
	if(REG_CFG11_SOCINFO & 2)
	{
		REGs_CFG11_MPCORE_BOOTCNT[2] = 0;
		REGs_CFG11_MPCORE_BOOTCNT[3] = 0;

		REG_UNK_10140410 = 0;
		REG_UNK_10140400 = 0;

		REG_CFG11_MPCORE_CNT = 0;
		CPU_setClock(1);

		REG_SCU_CPU_STAT |= 0b1111<<4;

		CPU_setClock(0);
	}
}
