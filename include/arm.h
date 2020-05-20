#pragma once

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

#if !__ASSEMBLER__
	#include "types.h"
#endif


// Program status register (CPSR/SPSR)
#define PSR_USER_MODE   (16)
#define PSR_FIQ_MODE    (17)
#define PSR_IRQ_MODE    (18)
#define PSR_SVC_MODE    (19)
#define PSR_ABORT_MODE  (23)
#define PSR_UNDEF_MODE  (27)
#define PSR_SYS_MODE    (31)
#define PSR_MODE_MASK   (PSR_SYS_MODE)

#define PSR_T           (1<<5)          // Thumb mode
#define PSR_F           (1<<6)          // Interrupts (FIQ) disable flag
#define PSR_I           (1<<7)          // Interrupts (IRQ) disable flag
#define PSR_A           (1<<8)          // Imprecise aborts disable flag
#define PSR_E           (1<<9)          // Big endian
#define PSR_J           (1<<24)         // Jazelle mode
#define PSR_Q           (1<<27)
#define PSR_V           (1<<28)         // Overflow flag
#define PSR_C           (1<<29)         // Carry flag
#define PSR_Z           (1<<30)         // Zero flag
#define PSR_N           (1<<31)         // Negative flag
#define PSR_INT_OFF     (PSR_I | PSR_F) // IRQ and FIQ disabled flags



#if !__ASSEMBLER__

#define MAKE_STANDALONE(name)               \
static inline void __##name(void)           \
{                                           \
	__asm__ volatile(#name : : : "memory"); \
}

#define MAKE_GET_REG(name, inst)            \
static inline u32 __##name(void)            \
{                                           \
	u32 reg;                                \
	__asm__ volatile(inst : "=r" (reg) : ); \
	return reg;                             \
}

#define MAKE_SET_REG_ZERO(name, inst)              \
static inline void __##name(void)                  \
{                                                  \
	__asm__ volatile(inst : : "r" (0) : "memory"); \
}

#define MAKE_SET_REG(name, inst)                     \
static inline void __##name(u32 reg)                 \
{                                                    \
	__asm__ volatile(inst : : "r" (reg) : "memory"); \
}


#if !__thumb__
// Program status register
MAKE_GET_REG(getCpsr, "mrs %0, cpsr")
MAKE_SET_REG(setCpsr_c, "msr cpsr_c, %0")
MAKE_SET_REG(setCpsr, "msr cpsr_cxsf, %0")
MAKE_GET_REG(getSpsr, "mrs %0, spsr")
MAKE_SET_REG(setSpsr_c, "msr spsr_c, %0")
MAKE_SET_REG(setSpsr, "msr spsr_cxsf, %0")

// Control Register
MAKE_GET_REG(getCr, "mrc p15, 0, %0, c1, c0, 0")
MAKE_SET_REG(setCr, "mcr p15, 0, %0, c1, c0, 0")
#endif // if !__thumb__


#ifdef ARM11
#define __cpsid(flags) __asm__ volatile("cpsid " #flags : : : "memory")
#define __cpsie(flags) __asm__ volatile("cpsie " #flags : : : "memory")
#define __setend(end) __asm__ volatile("setend " #end : : : "memory")

MAKE_STANDALONE(wfi)
MAKE_STANDALONE(wfe)
MAKE_STANDALONE(sev)

#if !__thumb__
static inline u32 __getCpuId(void)
{
	u32 cpuId;
	__asm__("mrc p15, 0, %0, c0, c0, 5" : "=r" (cpuId) : );
	return cpuId & 3;
}

// Flush Prefetch Buffer
// Data Synchronization Barrier
// Data Memory Barrier
MAKE_SET_REG_ZERO(isb, "mcr p15, 0, %0, c7, c5, 4")
MAKE_SET_REG_ZERO(dsb, "mcr p15, 0, %0, c7, c10, 4")
MAKE_SET_REG_ZERO(dmb, "mcr p15, 0, %0, c7, c10, 5")

// Auxiliary Control Register
MAKE_GET_REG(getAcr, "mrc p15, 0, %0, c1, c0, 1")
MAKE_SET_REG(setAcr, "mcr p15, 0, %0, c1, c0, 1")

// Translation Table Base Register 0
MAKE_GET_REG(getTtbr0, "mrc p15, 0, %0, c2, c0, 0")
MAKE_SET_REG(setTtbr0, "mcr p15, 0, %0, c2, c0, 0")

// Translation Table Base Register 1
MAKE_GET_REG(getTtbr1, "mrc p15, 0, %0, c2, c0, 1")
MAKE_SET_REG(setTtbr1, "mcr p15, 0, %0, c2, c0, 1")

// Translation Table Base Control Register
MAKE_GET_REG(getTtbcr, "mrc p15, 0, %0, c2, c0, 2")
MAKE_SET_REG(setTtbcr, "mcr p15, 0, %0, c2, c0, 2")

// Domain Access Control Register
MAKE_GET_REG(getDacr, "mrc p15, 0, %0, c3, c0, 0")
MAKE_SET_REG(setDacr, "mcr p15, 0, %0, c3, c0, 0")

// FCSE PID Register
MAKE_GET_REG(getFcsepidr, "mrc p15, 0, %0, c13, c0, 0")
MAKE_SET_REG(setFcsepidr, "mcr p15, 0, %0, c13, c0, 0")

// Context ID Register
MAKE_GET_REG(getCidr, "mrc p15, 0, %0, c13, c0, 1")
MAKE_SET_REG(setCidr, "mcr p15, 0, %0, c13, c0, 1")
#endif // if !__thumb__

#elif ARM9

#if !__thumb__
MAKE_SET_REG_ZERO(wfi, "mcr p15, 0, %0, c7, c0, 4")
#endif // if !__thumb__
#endif // ifdef ARM11

#undef MAKE_STANDALONE
#undef MAKE_GET_REG
#undef MAKE_SET_REG_ZERO
#undef MAKE_SET_REG

#endif // if !__ASSEMBLER__
