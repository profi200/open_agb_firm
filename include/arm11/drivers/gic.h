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


// Most register names from: https://github.com/torvalds/linux/blob/master/include/linux/irqchip/arm-gic.h
#define GIC_CPU_REGS_BASE   (MPCORE_PRIV_REG_BASE + 0x100)
#define GIC_DIST_REGS_BASE  (MPCORE_PRIV_REG_BASE + 0x1000)

typedef struct
{
	vu32 ctrl;             // 0x00 Control Register.
	vu32 primask;          // 0x04 Priority Mask Register.
	vu32 binpoint;         // 0x08 Binary Point Register.
	const vu32 intack;     // 0x0C Interrupt Acknowledge Register.
	vu32 eoi;              // 0x10 End of Interrupt Register.
	const vu32 runningpri; // 0x14 Running Priority Register.
	const vu32 highpri;    // 0x18 Highest Pending Interrupt Register.
} GicCpu;
static_assert(offsetof(GicCpu, highpri) == 0x18, "Error: Member highpri of GicCpu is not at offset 0x18!");

typedef struct
{
	vu32 ctrl;                // 0x000 Interrupt Distributor Control Register.
	const vu32 ctr;           // 0x004 Interrupt Controller Type Register.
	u8 _0x8[0xf8];
	vu32 enable_set[8];       // 0x100 Interrupt Enable set Registers.
	u8 _0x120[0x60];
	vu32 enable_clear[8];     // 0x180 Interrupt Enable clear Registers.
	u8 _0x1a0[0x60];
	vu32 pending_set[8];      // 0x200 Interrupt Pending set Registers.
	u8 _0x220[0x60];
	vu32 pending_clear[8];    // 0x280 Interrupt Pending clear Registers.
	u8 _0x2a0[0x60];
	const vu32 active_set[8]; // 0x300 Interrupt Active Bit Registers.
	u8 _0x320[0xe0];
	vu32 pri[64];             // 0x400 Interrupt Priority Registers.
	u8 _0x500[0x300];
	vu32 target[64];          // 0x800 Interrupt CPU targets Registers.
	u8 _0x900[0x300];
	vu32 config[16];          // 0xC00 Interrupt Configuration Registers.
	u8 _0xc40[0xc0];
	const vu32 line_level[8]; // 0xD00 Interrupt Line Level Registers.
	u8 _0xd20[0x1e0];
	vu32 softint;             // 0xF00 Software Interrupt Register.
	u8 _0xf04[0xdc];
	const vu32 periph_ident0; // 0xFE0 Periphal Identification Register 0.
	const vu32 periph_ident1; // 0xFE4 Periphal Identification Register 1.
	const vu32 periph_ident2; // 0xFE8 Periphal Identification Register 2.
	const vu32 periph_ident3; // 0xFEC Periphal Identification Register 3.
	const vu32 primecell0;    // 0xFF0 PrimeCell Identification Register 0.
	const vu32 primecell1;    // 0xFF4 PrimeCell Identification Register 1.
	const vu32 primecell2;    // 0xFF8 PrimeCell Identification Register 2.
	const vu32 primecell3;    // 0xFFC PrimeCell Identification Register 3.
} GicDist;
static_assert(offsetof(GicDist, primecell3) == 0xFFC, "Error: Member primecell3 of GicDist is not at offset 0xFFC!");

ALWAYS_INLINE GicCpu* getGicCpuRegs(void)
{
	return (GicCpu*)GIC_CPU_REGS_BASE;
}

ALWAYS_INLINE GicDist* getGicDistRegs(void)
{
	return (GicDist*)GIC_DIST_REGS_BASE;
}
