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


#define SCU_REGS_BASE  (MPCORE_PRIV_REG_BASE)

typedef struct
{
	vu32 ctrl;         // 0x00 SCU Control Register.
	const vu32 config; // 0x04 SCU Configuration Register.
	vu32 cpu_stat;     // 0x08 SCU CPU Status Register.
	vu32 inval_all;    // 0x0C SCU Invalidate All Register. (write-only)
	vu32 pmc;          // 0x10 Performance Monitor Control Register.
	vu32 pme0;         // 0x14 Performance monitor event register 0.
	vu32 pme1;         // 0x18 Performance monitor event register 1.
	vu32 mn0;          // 0x1C Count register 0.
	vu32 mn1;          // 0x20 Count register 1.
	vu32 mn2;          // 0x24 Count register 2.
	vu32 mn3;          // 0x28 Count register 3.
	vu32 mn4;          // 0x2C Count register 4.
	vu32 mn5;          // 0x30 Count register 5.
	vu32 mn6;          // 0x34 Count register 6.
	vu32 mn7;          // 0x38 Count register 7.
} Scu;
static_assert(offsetof(Scu, mn7) == 0x38, "Error: Member mn7 of Scu is not at offset 0x38!");

ALWAYS_INLINE Scu* getScuRegs(void)
{
	return (Scu*)SCU_REGS_BASE;
}


// REG_SCU_CTRL
#define SCU_EN                  (1u)            // SCU is enabled, coherency is maintained between MP11 CPUs Level 1 data side caches.
#define SCU_ACS(n)              (1u<<((n) + 1)) // CPUn can write to SCU-specific registers.
#define SCU_INT_ALIAS_ACS(n)    (1u<<((n) + 5)) // CPUn can access aliased interrupt interface registers in the offset range 0x0200 to 0x050FF of the MPCore private memory region.
#define SCU_TMR_ALIAS_ACS(n)    (1u<<((n) + 9)) // CPUn can access aliased timer and watchdog registers in the offset range 0x0700 to 0x0A0FF of the MPCore private memory region.
#define SCU_PARITY_CHK_EN       (1u<<13)        // SCU parity checking enable bit.
#define SCU_CTRL_RST_VAL        (SCU_TMR_ALIAS_ACS(3u) | SCU_TMR_ALIAS_ACS(2u) | \
                                 SCU_TMR_ALIAS_ACS(1u) | SCU_TMR_ALIAS_ACS(0u) | \
                                 SCU_INT_ALIAS_ACS(3u) | SCU_INT_ALIAS_ACS(2u) | \
                                 SCU_INT_ALIAS_ACS(1u) | SCU_INT_ALIAS_ACS(0u) | \
                                 SCU_ACS(3u) | SCU_ACS(2u) | SCU_ACS(1u) | SCU_ACS(0u))

// REG_SCU_CONFIG
#define SCU_CPU_NUM_1           (0u)                // 1 MP11 CPU, CPU0.
#define SCU_CPU_NUM_2           (1u)                // 2 MP11 CPUs, CPU0-CPU1.
#define SCU_CPU_NUM_3           (2u)                // 3 MP11 CPUs, CPU0-CPU2.
#define SCU_CPU_NUM_4           (3u)                // 4 MP11 CPUs, CPU0-CPU3.
#define SCU_CPU_NUM_MASK        (SCU_CPU_NUM_4)
#define SCU_SMP(n)              (1u<<((n) + 4))     // MP11 CPUn is in SMP mode taking part in coherency.
#define SCU_TRAM_16KB_64I(n)    (0u)                // CPUn 16KB cache, 64 indexes per tag RAM.
#define SCU_TRAM_32KB_128I(n)   (1u<<((n) * 2 + 8)) // CPUn 32KB cache, 128 indexes per tag RAM.
#define SCU_TRAM_64KB_256I(n)   (2u<<((n) * 2 + 8)) // CPUn 64KB cache, 256 indexes per tag RAM.

// REG_SCU_CPU_STAT
#define SCU_STAT_NORMAL(n)      (0u)            // CPUn Normal mode (Default).
// 1 reserved.
#define SCU_STAT_DORMANT(n)     (2u<<((n) * 2)) // CPUn is about to enter (or is in) dormant mode. No CCB request is sent to the CPU.
#define SCU_STAT_PWROFF(n)      (3u<<((n) * 2)) // CPUn is about to enter (or is in) powered-off mode, or is nonpresent. No CCB request is sent to the CPU.
#define SCU_STAT_MASK(n)        (SCU_STAT_PWROFF(n))

// REG_SCU_INVAL_ALL
#define SCU_WAY_0(n)            (1u<<((n) * 4)) // CPUn invalidate way 0.
#define SCU_WAY_1(n)            (2u<<((n) * 4)) // CPUn invalidate way 1.
#define SCU_WAY_2(n)            (4u<<((n) * 4)) // CPUn invalidate way 2.
#define SCU_WAY_3(n)            (8u<<((n) * 4)) // CPUn invalidate way 3.
#define SCU_WAY_ALL             (SCU_WAY_3(3u) | SCU_WAY_2(3u) | SCU_WAY_1(3u) | \
                                 SCU_WAY_0(3u) | SCU_WAY_3(2u) | SCU_WAY_2(2u) | \
                                 SCU_WAY_1(2u) | SCU_WAY_0(2u) | SCU_WAY_3(1u) | \
                                 SCU_WAY_2(1u) | SCU_WAY_1(1u) | SCU_WAY_0(1u) | \
                                 SCU_WAY_3(0u) | SCU_WAY_2(0u) | SCU_WAY_1(0u) | \
                                 SCU_WAY_0(0u))

// TODO: SCU performance monitor bits.
