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



#ifdef CORE123_INIT
static void NAKED core23Entry(void)
{
	__cpsid(aif);
	REG_GIC_CPU_CTRL = 1;

	const u32 cpuId = __getCpuId();
	// Tell core 0 we are here
	if(cpuId == 3) REGs_PDN_MPCORE_BOOTCNT[3] = MPCORE_BOOTCNT_RST;
	else           REGs_PDN_MPCORE_BOOTCNT[2] = MPCORE_BOOTCNT_RST;

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

void PDN_core123Init(void)
{
	if(REG_CFG11_SOCINFO & SOCINFO_N3DS_PROTO)
	{
		REG_GIC_CPU_CTRL = 1;
		for(u32 i = 0; i < 4; i++) REGs_GIC_DIST_ENABLE_CLEAR[i] = 0xFFFFFFFFu;
		REGs_GIC_DIST_PENDING_CLEAR[2] = 0x1000000; // Interrupt ID 88
		REGs_GIC_DIST_PRI[22] = 0;
		REGs_GIC_DIST_TARGET[22] = 1;
		REGs_GIC_DIST_ENABLE_SET[2] = 0x1000000;

#ifdef CORE123_INIT
		u16 socmode;
		// If non-prototype SoC use 804 MHz.
		if(REG_CFG11_SOCINFO & SOCINFO_N3DS) socmode = SOCMODE_N3DS_804MHz;
		else                                 socmode = SOCMODE_N3DS_PROTO_536MHz;

		if((REG_PDN_MPCORE_SOCMODE & SOCMODE_MASK) != socmode)
		{
			// No idea what this does
			if(REG_CFG11_SOCINFO & SOCINFO_N3DS) REG_PDN_MPCORE_CNT = PDN_MPCORE_CNT_L2C_E | PDN_MPCORE_CNT_MEM_EXT_E;
			else                                 REG_PDN_MPCORE_CNT = PDN_MPCORE_CNT_MEM_EXT_E;

			// Necessary delay.
			wait(403);

			PDN_setSocmode(socmode);
			REGs_GIC_DIST_PENDING_CLEAR[2] = 0x1000000;
			REG_CFG11_GPU_N3DS_CNT = GPU_N3DS_CNT_TEX_FIX | GPU_N3DS_CNT_N3DS_MODE;
		}
		REG_CFG11_CDMA_PERIPHERALS = CDMA_PERIPHERALS_ALL; // Redirect all to CDMA2.

		if((REG_SCU_CONFIG & 3) == 3)
		{
			// Set core 2/3 to normal mode (running)
			REG_SCU_CPU_STAT &= ~0b11110000;

			const u16 socmode = REG_PDN_MPCORE_SOCMODE & SOCMODE_MASK;
			u16 tmpSocmode;
			if(REG_CFG11_SOCINFO & SOCINFO_N3DS) tmpSocmode = SOCMODE_N3DS_268MHz;
			else                                 tmpSocmode = SOCMODE_N3DS_PROTO_268MHz;

			if(socmode != tmpSocmode)
			{
				PDN_setSocmode(tmpSocmode);
				REGs_GIC_DIST_PENDING_CLEAR[2] = 0x1000000;
			}

			REG_CFG11_BOOTROM_OVERLAY_CNT = BOOTROM_OVERLAY_CNT_E;
			REG_CFG11_BOOTROM_OVERLAY_VAL = (u32)core23Entry;
			// If not already done enable instruction and data overlays
			if(!(REGs_PDN_MPCORE_BOOTCNT[2] & MPCORE_BOOTCNT_RST_STAT))
			{
				REGs_PDN_MPCORE_BOOTCNT[2] = MPCORE_BOOTCNT_D_OVERL_E | MPCORE_BOOTCNT_RST;
			}
			if(!(REGs_PDN_MPCORE_BOOTCNT[3] & MPCORE_BOOTCNT_RST_STAT))
			{
				REGs_PDN_MPCORE_BOOTCNT[3] = MPCORE_BOOTCNT_D_OVERL_E | MPCORE_BOOTCNT_RST;
			}
			// Wait for core 2/3 to jump out of boot11
			while((REGs_PDN_MPCORE_BOOTCNT[2] & (MPCORE_BOOTCNT_RST_STAT | MPCORE_BOOTCNT_D_OVERL_E))
			      != MPCORE_BOOTCNT_RST_STAT);
			while((REGs_PDN_MPCORE_BOOTCNT[3] & (MPCORE_BOOTCNT_RST_STAT | MPCORE_BOOTCNT_D_OVERL_E))
			      != MPCORE_BOOTCNT_RST_STAT);
			REG_CFG11_BOOTROM_OVERLAY_CNT = 0; // Disable all overlays

			// Set clock back to original one
			if(socmode != tmpSocmode) PDN_setSocmode(socmode);
		}

		REGs_GIC_DIST_ENABLE_CLEAR[2] = 0x1000000;

		// Wakeup core 2/3 and let them jump to their entrypoint.
		IRQ_softwareInterrupt(2, 0b0100);
		IRQ_softwareInterrupt(3, 0b1000);
#else
		// Just enables the New3DS FCRAM extension (if not already done).
		if((REG_PDN_MPCORE_SOCMODE & SOCMODE_MASK) != SOCMODE_N3DS_268MHz)
			PDN_setSocmode(SOCMODE_N3DS_268MHz);

		REGs_GIC_DIST_ENABLE_CLEAR[2] = 0x1000000;
#endif
	}

	// Wakeup core 1
	*((vu32*)0x1FFFFFDC) = (u32)_start;  // Core 1 entrypoint
	IRQ_softwareInterrupt(1, 0b0010);
}

void PDN_setSocmode(PdnSocmode socmode)
{
	REG_PDN_MPCORE_SOCMODE = PDN_MPCORE_SOCMODE_ACK | socmode;
	do
	{
		__wfi();
	} while(!(REG_PDN_MPCORE_SOCMODE & PDN_MPCORE_SOCMODE_ACK));
}

void PDN_poweroffCore23(void)
{
	if(REG_CFG11_SOCINFO & SOCINFO_N3DS_PROTO)
	{
		REGs_PDN_MPCORE_BOOTCNT[2] = 0;
		REGs_PDN_MPCORE_BOOTCNT[3] = 0;

		REG_CFG11_CDMA_PERIPHERALS = 0;
		REG_CFG11_GPU_N3DS_CNT = 0;

		REG_PDN_MPCORE_CNT = 0;
		PDN_setSocmode(SOCMODE_N3DS_268MHz);

		REG_SCU_CPU_STAT |= 0b1111<<4;

		PDN_setSocmode(SOCMODE_O3DS_268MHz);
	}
}
