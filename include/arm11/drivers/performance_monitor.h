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

// Based on ARM11 MPCore™ Processor Revision: r2p0 Technical Reference Manual (DDI0360F_arm11_mpcore_r2p0_trm.pdf).

#if !__ASSEMBLER__
#include "types.h"
#endif // #if !__ASSEMBLER__


// Performance Monitor Control Register (PMNC).
#define PM_EN           (1u)     // All three counters enabled.
#define PM_PMN01_RST    (1u<<1)  // Reset both Count Registers to 0x0.
#define PM_CCNT_RST     (1u<<2)  // Reset the Cycle Counter Register to 0x0.
#define PM_CCNT_NODIV   (0u)     // Cycle Counter Register counts every processor clock cycle.
#define PM_CCNT_DIV64   (1u<<3)  // Cycle Counter Register counts every 64th processor clock cycle.
#define PM_PMN0_IRQ_EN  (1u<<4)  // Count Register 0 interrupt enable.
#define PM_PMN1_IRQ_EN  (1u<<5)  // Count Register 1 interrupt enable.
#define PM_CCNT_IRQ_EN  (1u<<6)  // Cycle Counter interrupt enable.
#define PM_PMN0_IRQ     (1u<<8)  // Count Register 0 overflow flag. Write 1 to clear.
#define PM_PMN1_IRQ     (1u<<9)  // Count Register 1 overflow flag. Write 1 to clear.
#define PM_CCNT_IRQ     (1u<<10) // Cycle Counter Register overflow flag. Write 1 to clear.
#define PM_EVT(pmn1, pmn0)  ((pmn0)<<20 | (pmn1)<<12) // Set what events PMN0/1 count. See events below.

// Performance monitor events.
#define PM_EVT_ICACHE_MISS        (0x00u) // Instruction cache miss to a cachable location requires fetch from external memory.
#define PM_EVT_INST_BUF_STALL     (0x01u) // Stall because instruction buffer cannot deliver an instruction. This can indicate an instruction cache miss or an instruction MicroTLB miss. This event occurs every cycle where the condition is present.
#define PM_EVT_DATA_DEP_STALL     (0x02u) // Stall because of a data dependency. This event occurs every cycle where the condition is present.
#define PM_EVT_INST_MICROTLB_MISS (0x03u) // Instruction MicroTLB miss.
#define PM_EVT_DATA_MICROTLB_MISS (0x04u) // Data MicroTLB miss.
#define PM_EVT_BRANCH_EXEC        (0x05u) // Branch instruction executed, branch might or might not have changed program flow.
#define PM_EVT_BRANCH_NOT_PRED    (0x06u) // Branch not predicted.
#define PM_EVT_BRANCH_MISPRED     (0x07u) // Branch mispredicted.
#define PM_EVT_INST_EXEC          (0x08u) // Instruction executed.
#define PM_EVT_FOLD_INST_EXEC     (0x09u) // Folded instruction executed.
#define PM_EVT_DCACHE_RD          (0x0Au) // Data cache read access, not including cache operations. This event occurs for each non-sequential access to a cache line.
#define PM_EVT_DCACHE_RD_MISS     (0x0Bu) // Data cache read miss, not including cache operations.
#define PM_EVT_DCACHE_WR          (0x0Cu) // Data cache write access.
#define PM_EVT_DCACHE_WR_MISS     (0x0Du) // Data cache write miss.
#define PM_EVT_DCACHE_LINE_EVICT  (0x0Eu) // Data cache line eviction, not including cache operations.
#define PM_EVT_PC_CHANGE_NOT_MODE (0x0Fu) // Software changed the PC and there is not a mode change.
#define PM_EVT_TLB_MISS           (0x10u) // Main TLB miss.
#define PM_EVT_EXT_MEM_REQ        (0x11u) // External memory request (cache refill, noncachable, write-back).
#define PM_EVT_LD_ST_UNIT_STALL   (0x12u) // Stall because of Load Store Unit request queue being full.
#define PM_EVT_ST_BUF_DRAIN       (0x13u) // The number of times the Store buffer was drained because of LSU ordering constraints or CP15 operations.
#define PM_EVT_BUF_WR_MERGED      (0x14u) // Buffered write merged in a store buffer slot.
#define PM_EVT_CYCLE              (0xFFu) // An increment each cycle.



#if !__ASSEMBLER__
// Write Performance Monitor Control Register.
ALWAYS_INLINE void __setPmnc(u32 val)
{
	__asm__ volatile("mcr p15, 0, %0, c15, c12, 0" : : "r" (val) : "memory");
}

// Write Cycle Counter Register.
ALWAYS_INLINE void __setCcnt(u32 val)
{
	__asm__ volatile("mcr p15, 0, %0, c15, c12, 1" : : "r" (val) : "memory");
}

// Read Count Register 0.
ALWAYS_INLINE void __setPmn0(u32 val)
{
	__asm__ volatile("mcr p15, 0, %0, c15, c12, 2" : : "r" (val) : "memory");
}

// Read Count Register 1.
ALWAYS_INLINE void __setPmn1(u32 val)
{
	__asm__ volatile("mcr p15, 0, %0, c15, c12, 3" : : "r" (val) : "memory");
}

// Read Performance Monitor Control Register.
ALWAYS_INLINE u32 __getPmnc(void)
{
	u32 tmp;
	__asm__ volatile("mrc p15, 0, %0, c15, c12, 0" : "=r" (tmp) : : "memory");
	return tmp;
}

// Read Cycle Counter Register.
ALWAYS_INLINE u32 __getCcnt(void)
{
	u32 tmp;
	__asm__ volatile("mrc p15, 0, %0, c15, c12, 1" : "=r" (tmp) : : "memory");
	return tmp;
}

// Write Count Register 0.
ALWAYS_INLINE u32 __getPmn0(void)
{
	u32 tmp;
	__asm__ volatile("mrc p15, 0, %0, c15, c12, 2" : "=r" (tmp) : : "memory");
	return tmp;
}

// Write Count Register 1.
ALWAYS_INLINE u32 __getPmn1(void)
{
	u32 tmp;
	__asm__ volatile("mrc p15, 0, %0, c15, c12, 3" : "=r" (tmp) : : "memory");
	return tmp;
}


// Helpers.
// Note: Make sure the performance monitor is off
//       before starting to count cycles to avoid
//       misleading counter results.
ALWAYS_INLINE void perfMonitorCountCycles(void)
{
	__setPmnc(PM_EVT(PM_EVT_INST_EXEC, PM_EVT_ICACHE_MISS) | PM_CCNT_IRQ | PM_PMN1_IRQ |
	          PM_PMN0_IRQ | PM_CCNT_NODIV | PM_CCNT_RST | PM_PMN01_RST | PM_EN);
}
#endif // #if !__ASSEMBLER__
