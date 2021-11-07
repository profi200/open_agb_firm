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

#define MAKE_INTR_NO_INOUT(isVolatile, name, inst, ...)  \
ALWAYS_INLINE void __##name(void)                        \
{                                                        \
	if(isVolatile == 1)                                  \
		__asm__ volatile(inst : : : __VA_ARGS__);        \
	else                                                 \
		__asm__(inst : : : __VA_ARGS__);                 \
}

#define MAKE_INTR_GET_REG(isVolatile, name, inst) \
ALWAYS_INLINE u32 __##name(void)                  \
{                                                 \
	u32 reg;                                      \
	if(isVolatile == 1)                           \
		__asm__ volatile(inst : "=r" (reg) : : ); \
	else                                          \
		__asm__(inst : "=r" (reg) : : );          \
	return reg;                                   \
}

#define MAKE_INTR_SET_REG_ZERO(isVolatile, name, inst, ...) \
ALWAYS_INLINE void __##name(void)                           \
{                                                           \
	if(isVolatile == 1)                                     \
		__asm__ volatile(inst : : "r" (0) : __VA_ARGS__);   \
	else                                                    \
		__asm__(inst : : "r" (0) : __VA_ARGS__);            \
}

#define MAKE_INTR_SET_REG(isVolatile, name, inst, ...)      \
ALWAYS_INLINE void __##name(u32 reg)                        \
{                                                           \
	if(isVolatile == 1)                                     \
		__asm__ volatile(inst : : "r" (reg) : __VA_ARGS__); \
	else                                                    \
		__asm__(inst : : "r" (reg) : __VA_ARGS__);          \
}


#define __bkpt(val) __asm__ volatile("bkpt #" #val : : : )

#if !__thumb__
// Program status register
MAKE_INTR_GET_REG(1, getCpsr, "mrs %0, cpsr")
MAKE_INTR_SET_REG(1, setCpsr_c, "msr cpsr_c, %0", "memory")
MAKE_INTR_SET_REG(1, setCpsr, "msr cpsr_cxsf, %0", "memory")
MAKE_INTR_GET_REG(1, getSpsr, "mrs %0, spsr")
MAKE_INTR_SET_REG(1, setSpsr_c, "msr spsr_c, %0", "memory")
MAKE_INTR_SET_REG(1, setSpsr, "msr spsr_cxsf, %0", "memory")

// Control Register
MAKE_INTR_GET_REG(1, getCr, "mrc p15, 0, %0, c1, c0, 0")
MAKE_INTR_SET_REG(1, setCr, "mcr p15, 0, %0, c1, c0, 0", "memory")
#endif // if !__thumb__


#ifdef ARM11
#define __cpsid(flags) __asm__ volatile("cpsid " #flags : : : "memory")
#define __cpsie(flags) __asm__ volatile("cpsie " #flags : : : "memory")
#define __setend(end) __asm__ volatile("setend " #end : : : "memory")

MAKE_INTR_NO_INOUT(1, nop, "nop")
MAKE_INTR_NO_INOUT(1, wfi, "wfi", "memory")
MAKE_INTR_NO_INOUT(1, wfe, "wfe", "memory")
MAKE_INTR_NO_INOUT(1, sev, "sev")

#if !__thumb__
ALWAYS_INLINE u8 __ldrexb(vu8 *addr)
{
	u8 res;
	__asm__ volatile("ldrexb %0, %1" : "=r" (res) : "Q" (*addr) : );
	return res;
}

ALWAYS_INLINE u16 __ldrexh(vu16 *addr)
{
	u16 res;
	__asm__ volatile("ldrexh %0, %1" : "=r" (res) : "Q" (*addr) : );
	return res;
}

ALWAYS_INLINE u32 __ldrex(vu32 *addr)
{
	u32 res;
	__asm__ volatile("ldrex %0, %1" : "=r" (res) : "Q" (*addr) : );
	return res;
}

/*ALWAYS_INLINE u64 __ldrexd(vu64 *addr)
{
	union
	{
		u32 r32[2];
		u64 r64;
	} r;

	// TODO: "Error: even register required -- `ldrexd r3,r2,[r0]'"
#ifndef __ARMEB__ // Little endian
	__asm__ volatile("ldrexd %0, %1, %2" : "=r" (r.r32[0]), "=r" (r.r32[1]) : "Q" (*addr) : );
#else             // Big endian
	__asm__ volatile("ldrexd %0, %1, %2" : "=r" (r.r32[1]), "=r" (r.r32[0]) : "Q" (*addr) : );
#endif
	return r.r64;
}*/

ALWAYS_INLINE u32 __strexb(vu8 *addr, u8 val)
{
	u32 res;
	__asm__ volatile("strexb %0, %2, %1" : "=&r" (res), "=Q" (*addr) : "r" (val) : );
	return res;
}

ALWAYS_INLINE u32 __strexh(vu16 *addr, u16 val)
{
	u32 res;
	__asm__ volatile("strexh %0, %2, %1" : "=&r" (res), "=Q" (*addr) : "r" (val) : );
	return res;
}

ALWAYS_INLINE u32 __strex(vu32 *addr, u32 val)
{
	u32 res;
	__asm__ volatile("strex %0, %2, %1" : "=&r" (res), "=Q" (*addr) : "r" (val) : );
	return res;
}

/*ALWAYS_INLINE u32 __strexd(vu64 *addr, u64 val)
{
	union
	{
		u32 r32[2];
		u64 r64;
	} r;
	r.r64 = val;

	// TODO: "Error: even register required -- `strexd r0,r3,r2,[r1]'"
	u32 res;
#ifndef __ARMEB__ // Little endian
	__asm__ volatile("strexd %0, %2, %3, %1" : "=&r" (res), "=Q" (*addr) : "r" (r.r32[0]), "r" (r.r32[1]) : );
#else             // Big endian
	__asm__ volatile("strexd %0, %2, %3, %1" : "=&r" (res), "=Q" (*addr) : "r" (r.r32[1]), "r" (r.r32[0]) : );
#endif
	return res;
}*/

MAKE_INTR_NO_INOUT(1, clrex, "clrex", "memory")

// Debug ID Register
MAKE_INTR_GET_REG(0, getDidr, "mrc p14, 0, %0, c0, c0, 0")

// Debug Status and Control Register
MAKE_INTR_GET_REG(1, getDscr, "mrc p14, 0, %0, c0, c1, 0")
MAKE_INTR_SET_REG(1, setDscr, "mcr p14, 0, %0, c0, c1, 0", "memory")

// Data Transfer Register

// Vector Catch Register
MAKE_INTR_GET_REG(1, getVcr, "mrc p14, 0, %0, c0, c7, 0")
MAKE_INTR_SET_REG(1, setVcr, "mcr p14, 0, %0, c0, c7, 0", "memory")

// Breakpoint Value Register 0-5
MAKE_INTR_GET_REG(1, getBvr0, "mrc p14, 0, %0, c0, c0, 4")
MAKE_INTR_SET_REG(1, setBvr0, "mcr p14, 0, %0, c0, c0, 4", "memory")
MAKE_INTR_GET_REG(1, getBvr1, "mrc p14, 0, %0, c0, c1, 4")
MAKE_INTR_SET_REG(1, setBvr1, "mcr p14, 0, %0, c0, c1, 4", "memory")
MAKE_INTR_GET_REG(1, getBvr2, "mrc p14, 0, %0, c0, c2, 4")
MAKE_INTR_SET_REG(1, setBvr2, "mcr p14, 0, %0, c0, c2, 4", "memory")
MAKE_INTR_GET_REG(1, getBvr3, "mrc p14, 0, %0, c0, c3, 4")
MAKE_INTR_SET_REG(1, setBvr3, "mcr p14, 0, %0, c0, c3, 4", "memory")
MAKE_INTR_GET_REG(1, getBvr4, "mrc p14, 0, %0, c0, c4, 4")
MAKE_INTR_SET_REG(1, setBvr4, "mcr p14, 0, %0, c0, c4, 4", "memory")
MAKE_INTR_GET_REG(1, getBvr5, "mrc p14, 0, %0, c0, c5, 4")
MAKE_INTR_SET_REG(1, setBvr5, "mcr p14, 0, %0, c0, c5, 4", "memory")

// Breakpoint Control Register 0-5
MAKE_INTR_GET_REG(1, getBcr0, "mrc p14, 0, %0, c0, c0, 5")
MAKE_INTR_SET_REG(1, setBcr0, "mcr p14, 0, %0, c0, c0, 5", "memory")
MAKE_INTR_GET_REG(1, getBcr1, "mrc p14, 0, %0, c0, c1, 5")
MAKE_INTR_SET_REG(1, setBcr1, "mcr p14, 0, %0, c0, c1, 5", "memory")
MAKE_INTR_GET_REG(1, getBcr2, "mrc p14, 0, %0, c0, c2, 5")
MAKE_INTR_SET_REG(1, setBcr2, "mcr p14, 0, %0, c0, c2, 5", "memory")
MAKE_INTR_GET_REG(1, getBcr3, "mrc p14, 0, %0, c0, c3, 5")
MAKE_INTR_SET_REG(1, setBcr3, "mcr p14, 0, %0, c0, c3, 5", "memory")
MAKE_INTR_GET_REG(1, getBcr4, "mrc p14, 0, %0, c0, c4, 5")
MAKE_INTR_SET_REG(1, setBcr4, "mcr p14, 0, %0, c0, c4, 5", "memory")
MAKE_INTR_GET_REG(1, getBcr5, "mrc p14, 0, %0, c0, c5, 5")
MAKE_INTR_SET_REG(1, setBcr5, "mcr p14, 0, %0, c0, c5, 5", "memory")

// Watchpoint Value Register 0-1
MAKE_INTR_GET_REG(1, getWvr0, "mrc p14, 0, %0, c0, c0, 6")
MAKE_INTR_SET_REG(1, setWvr0, "mcr p14, 0, %0, c0, c0, 6", "memory")
MAKE_INTR_GET_REG(1, getWvr1, "mrc p14, 0, %0, c0, c1, 6")
MAKE_INTR_SET_REG(1, setWvr1, "mcr p14, 0, %0, c0, c1, 6", "memory")

// Watchpoint Control Register 0-1
MAKE_INTR_GET_REG(1, getWcr0, "mrc p14, 0, %0, c0, c0, 7")
MAKE_INTR_SET_REG(1, setWcr0, "mcr p14, 0, %0, c0, c0, 7", "memory")
MAKE_INTR_GET_REG(1, getWcr1, "mrc p14, 0, %0, c0, c1, 7")
MAKE_INTR_SET_REG(1, setWcr1, "mcr p14, 0, %0, c0, c1, 7", "memory")

ALWAYS_INLINE u32 __getCpuId(void)
{
	u32 cpuId;
	__asm__("mrc p15, 0, %0, c0, c0, 5" : "=r" (cpuId) : );
	return cpuId & 3;
}

// Auxiliary Control Register
MAKE_INTR_GET_REG(1, getAcr, "mrc p15, 0, %0, c1, c0, 1")
MAKE_INTR_SET_REG(1, setAcr, "mcr p15, 0, %0, c1, c0, 1", "memory")

// Translation Table Base Register 0
MAKE_INTR_GET_REG(1, getTtbr0, "mrc p15, 0, %0, c2, c0, 0")
MAKE_INTR_SET_REG(1, setTtbr0, "mcr p15, 0, %0, c2, c0, 0", "memory")

// Translation Table Base Register 1
MAKE_INTR_GET_REG(1, getTtbr1, "mrc p15, 0, %0, c2, c0, 1")
MAKE_INTR_SET_REG(1, setTtbr1, "mcr p15, 0, %0, c2, c0, 1", "memory")

// Translation Table Base Control Register
MAKE_INTR_GET_REG(1, getTtbcr, "mrc p15, 0, %0, c2, c0, 2")
MAKE_INTR_SET_REG(1, setTtbcr, "mcr p15, 0, %0, c2, c0, 2", "memory")

// Domain Access Control Register
MAKE_INTR_GET_REG(1, getDacr, "mrc p15, 0, %0, c3, c0, 0")
MAKE_INTR_SET_REG(1, setDacr, "mcr p15, 0, %0, c3, c0, 0", "memory")

// Data Fault Status Register
MAKE_INTR_GET_REG(1, getDfsr, "mrc p15, 0, %0, c5, c0, 0")
MAKE_INTR_SET_REG(1, setDfsr, "mcr p15, 0, %0, c5, c0, 0", "memory")

// Instruction Fault Status Register
MAKE_INTR_GET_REG(1, getIfsr, "mrc p15, 0, %0, c5, c0, 1")
MAKE_INTR_SET_REG(1, setIfsr, "mcr p15, 0, %0, c5, c0, 1", "memory")

// Fault Address Register
MAKE_INTR_GET_REG(1, getFar, "mrc p15, 0, %0, c6, c0, 0")
MAKE_INTR_SET_REG(1, setFar, "mcr p15, 0, %0, c6, c0, 0", "memory")

// Watchpoint Fault Address Register
MAKE_INTR_GET_REG(1, getWfar, "mrc p15, 0, %0, c6, c0, 1")
MAKE_INTR_SET_REG(1, setWfar, "mcr p15, 0, %0, c6, c0, 1", "memory")

// Flush Prefetch Buffer
// Data Synchronization Barrier
// Data Memory Barrier
MAKE_INTR_SET_REG_ZERO(1, isb, "mcr p15, 0, %0, c7, c5, 4", "memory")
MAKE_INTR_SET_REG_ZERO(1, dsb, "mcr p15, 0, %0, c7, c10, 4", "memory")
MAKE_INTR_SET_REG_ZERO(1, dmb, "mcr p15, 0, %0, c7, c10, 5", "memory")

// FCSE PID Register
MAKE_INTR_GET_REG(1, getFcsepidr, "mrc p15, 0, %0, c13, c0, 0")
MAKE_INTR_SET_REG(1, setFcsepidr, "mcr p15, 0, %0, c13, c0, 0", "memory")

// Context ID Register
MAKE_INTR_GET_REG(1, getCidr, "mrc p15, 0, %0, c13, c0, 1")
MAKE_INTR_SET_REG(1, setCidr, "mcr p15, 0, %0, c13, c0, 1", "memory")
#endif // if !__thumb__

#elif ARM9

#if !__thumb__
MAKE_INTR_NO_INOUT(1, wfi, "mcr p15, 0, r0, c7, c0, 4", "memory")
#endif // if !__thumb__
#endif // ifdef ARM11

#undef MAKE_INTR_NO_INOUT
#undef MAKE_INTR_GET_REG
#undef MAKE_INTR_SET_REG_ZERO
#undef MAKE_INTR_SET_REG

#endif // if !__ASSEMBLER__
