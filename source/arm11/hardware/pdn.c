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
#include "arm11/hardware/pdn.h"
#include "arm11/hardware/cfg11.h"
#include "arm.h"
#include "arm11/hardware/interrupt.h"
#include "arm11/start.h"
#include "util.h"
#include "arm11/hardware/scu.h"


//#define CORE123_INIT 1



#ifdef CORE123_INIT
static void NAKED core23Entry(void)
{
	__cpsid(aif);
	REG_GIC_CPU_CTRL = 1;

	// Tell core 0 we are here.
	const u32 cpuId = __getCpuId();
	if(cpuId == 3) REGs_PDN_LGR_CPU_CNT[3] = LGR_CPU_CNT_NORST;
	else           REGs_PDN_LGR_CPU_CNT[2] = LGR_CPU_CNT_NORST;

	// Wait for IPI 2 (core 2) or IPI 3 (core 3).
	u32 tmp;
	do
	{
		__wfi();
		tmp = REG_GIC_CPU_INTACK;
		REG_GIC_CPU_EOI = tmp;
	} while(tmp != cpuId);

	// Jump to real entrypoint.
	_start();
}
#endif

// This must be called before the MMU is enabled on any core or it will hang!
void PDN_core123Init(void)
{
	if(REG_CFG11_SOCINFO & SOCINFO_LGR1)
	{
		REG_GIC_CPU_CTRL = 1;
		for(u32 i = 0; i < 4; i++) REGs_GIC_DIST_ENABLE_CLEAR[i] = 0xFFFFFFFFu; // Disable all interrupts.
		REGs_GIC_DIST_PENDING_CLEAR[2] = 1u<<24; // Clear interrupt ID 88.
		REGs_GIC_DIST_PRI[22] = 0;               // Id 88 highest priority.
		REGs_GIC_DIST_TARGET[22] = 1;            // Id 88 target core 0.
		REGs_GIC_DIST_ENABLE_SET[2] = 1u<<24;    // Enable interrupt ID 88.

		// Certain bootloaders leave the ack bit set. Clear it.
		REG_PDN_LGR_SOCMODE = REG_PDN_LGR_SOCMODE;

#ifdef CORE123_INIT
		// Use 804 MHz for LGR2 and 536 for LGR1.
		u16 socmode;
		if(REG_CFG11_SOCINFO & SOCINFO_LGR2) socmode = SOCMODE_LGR2_804MHz;
		else                                 socmode = SOCMODE_LGR1_536MHz;

		if((REG_PDN_LGR_SOCMODE & SOCMODE_MASK) != socmode)
		{
			// Enable L2 cache and/or extra WRAM.
			if(REG_CFG11_SOCINFO & SOCINFO_LGR2) REG_PDN_LGR_CNT = PDN_LGR_CNT_L2C_E | PDN_LGR_CNT_WRAM_EXT_E;
			else                                 REG_PDN_LGR_CNT = PDN_LGR_CNT_WRAM_EXT_E;

			// Necessary delay.
			wait_cycles(403);

			PDN_setSocmode(socmode);
			REGs_GIC_DIST_PENDING_CLEAR[2] = 1u<<24; // Clear interrupt ID 88.

			// Fixes for the GPU to work in non-CTR mode.
			REG_CFG11_GPU_N3DS_CNT = GPU_N3DS_CNT_TEX_FIX | GPU_N3DS_CNT_N3DS_MODE;
		}

		REG_CFG11_CDMA_PERIPHERALS = CDMA_PERIPHERALS_ALL; // Redirect all to CDMA2.

		if((REG_SCU_CONFIG & 3) == 3)
		{
			// Set core 2/3 to normal mode (running) in the SCU.
			REG_SCU_CPU_STAT &= ~0b11110000;

			// Temporarily switch to 268 MHz for core 2/3 bringup.
			u16 tmpSocmode;
			if(REG_CFG11_SOCINFO & SOCINFO_LGR2) tmpSocmode = SOCMODE_LGR2_268MHz;
			else                                 tmpSocmode = SOCMODE_LGR1_268MHz;

			if(socmode != tmpSocmode)
			{
				PDN_setSocmode(tmpSocmode);
				REGs_GIC_DIST_PENDING_CLEAR[2] = 1u<<24; // Clear interrupt ID 88.
			}

			REG_CFG11_BOOTROM_OVERLAY_CNT = BOOTROM_OVERLAY_CNT_E;
			REG_CFG11_BOOTROM_OVERLAY_VAL = (u32)core23Entry;
			// If not already done enable instruction and data overlays.
			if(!(REGs_PDN_LGR_CPU_CNT[2] & LGR_CPU_CNT_RST_STAT))
			{
				REGs_PDN_LGR_CPU_CNT[2] = LGR_CPU_CNT_D_OVERL_E | LGR_CPU_CNT_NORST;
			}
			if(!(REGs_PDN_LGR_CPU_CNT[3] & LGR_CPU_CNT_RST_STAT))
			{
				REGs_PDN_LGR_CPU_CNT[3] = LGR_CPU_CNT_D_OVERL_E | LGR_CPU_CNT_NORST;
			}
			// Wait for core 2/3 to jump out of boot11.
			while((REGs_PDN_LGR_CPU_CNT[2] & (LGR_CPU_CNT_RST_STAT | LGR_CPU_CNT_D_OVERL_E))
			      != LGR_CPU_CNT_RST_STAT);
			while((REGs_PDN_LGR_CPU_CNT[3] & (LGR_CPU_CNT_RST_STAT | LGR_CPU_CNT_D_OVERL_E))
			      != LGR_CPU_CNT_RST_STAT);
			REG_CFG11_BOOTROM_OVERLAY_CNT = 0; // Disable all overlays.

			// Switch back to highest clock.
			if(socmode != tmpSocmode) PDN_setSocmode(socmode);
		}

		REGs_GIC_DIST_ENABLE_CLEAR[2] = 1u<<24; // Clear interrupt ID 88.

		// Wakeup core 2/3 and let them jump to their entrypoint.
		IRQ_softwareInterrupt(2, 0b0100);
		IRQ_softwareInterrupt(3, 0b1000);
#else
		// Make sure we are in CTR mode (if not already).
		if((REG_PDN_LGR_SOCMODE & SOCMODE_MASK) != SOCMODE_CTR_268MHz)
		{
			PDN_setSocmode(SOCMODE_CTR_268MHz);
			REGs_GIC_DIST_ENABLE_CLEAR[2] = 1u<<24; // Clear interrupt ID 88.
		}
#endif
	}

	// Wakeup core 1
	*((vu32*)0x1FFFFFDC) = (u32)_start;  // Core 1 entrypoint.
	IRQ_softwareInterrupt(1, 0b0010);
}

void PDN_setSocmode(PdnSocmode socmode)
{
	REG_PDN_LGR_SOCMODE = socmode;

	do
	{
		__wfi();
	} while(!(REG_PDN_LGR_SOCMODE & PDN_LGR_SOCMODE_ACK));

	REG_PDN_LGR_SOCMODE = REG_PDN_LGR_SOCMODE; // Acknowledge (PDN_LGR_SOCMODE_ACK bit set).
}

void PDN_poweroffCore23(void)
{
	if(REG_CFG11_SOCINFO & SOCINFO_LGR1)
	{
		REGs_PDN_LGR_CPU_CNT[2] = 0;
		REGs_PDN_LGR_CPU_CNT[3] = 0;

		REG_CFG11_CDMA_PERIPHERALS = 0;
		REG_CFG11_GPU_N3DS_CNT = 0;

		REG_PDN_LGR_CNT = 0;
		if(REG_CFG11_SOCINFO & SOCINFO_LGR2) PDN_setSocmode(SOCMODE_LGR2_268MHz);
		else                                 PDN_setSocmode(SOCMODE_LGR1_268MHz);

		REG_SCU_CPU_STAT |= 0b1111<<4;

		PDN_setSocmode(SOCMODE_CTR_268MHz);
	}
}
