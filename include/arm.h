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

#ifdef ARM11

#define __cpsid(flags) __asm__ volatile("cpsid " #flags : : : "memory")
#define __cpsie(flags) __asm__ volatile("cpsie " #flags : : : "memory")
#define __setend(end) __asm__ volatile("setend " #end : : : "memory")

static inline void __wfi(void)
{
	__asm__ volatile("wfi" : : : "memory");
}

static inline void __wfe(void)
{
	__asm__ volatile("wfe" : : : "memory");
}

static inline void __sev(void)
{
	__asm__ volatile("sev" : : : "memory");
}

static inline void __isb(void)
{
	// Flush Prefetch Buffer
	__asm__ volatile("mcr p15, 0, %0, c7, c5, 4" : : "r" (0) : "memory");
}

static inline void __dsb(void)
{
	__asm__ volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory");
}

static inline void __dmb(void)
{
	__asm__ volatile("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory");
}

static inline u32 __getCpuId(void)
{
	u32 cpuId;
	__asm__("mrc p15, 0, %0, c0, c0, 5" : "=r" (cpuId) : );
	return cpuId & 3;
}

// Auxiliary Control Register
static inline u32 __getAcr(void)
{
	u32 acr;
	__asm__("mrc p15, 0, %0, c1, c0, 1" : "=r" (acr) : );
	return acr;
}

static inline void __setAcr(u32 acr)
{
	__asm__ volatile("mcr p15, 0, %0, c1, c0, 1" : : "r" (acr) : "memory");
}

// Translation Table Base Register 0
static inline u32 __getTtbr0(void)
{
	u32 ttb0;
	__asm__("mrc p15, 0, %0, c2, c0, 0" : "=r" (ttb0) : );
	return ttb0;
}

static inline void __setTtbr0(u32 ttb0)
{
	__asm__ volatile("mcr p15, 0, %0, c2, c0, 0" : : "r" (ttb0) : "memory");
}

// Translation Table Base Register 1
static inline u32 __getTtbr1(void)
{
	u32 ttb1;
	__asm__("mrc p15, 0, %0, c2, c0, 1" : "=r" (ttb1) : );
	return ttb1;
}

static inline void __setTtbr1(u32 ttb1)
{
	__asm__ volatile("mcr p15, 0, %0, c2, c0, 1" : : "r" (ttb1) : "memory");
}

// Translation Table Base Control Register
static inline u32 __getTtbcr(void)
{
	u32 ttbcr;
	__asm__("mrc p15, 0, %0, c2, c0, 2" : "=r" (ttbcr) : );
	return ttbcr;
}

static inline void __setTtbcr(u32 ttbcr)
{
	__asm__ volatile("mcr p15, 0, %0, c2, c0, 2" : : "r" (ttbcr) : "memory");
}

// Domain Access Control Register
static inline u32 __getDacr(void)
{
	u32 dacr;
	__asm__("mrc p15, 0, %0, c3, c0, 0" : "=r" (dacr) : );
	return dacr;
}

static inline void __setDacr(u32 dacr)
{
	__asm__ volatile("mcr p15, 0, %0, c3, c0, 0" : : "r" (dacr) : "memory");
}

// FCSE PID Register
static inline u32 __getFcsepidr(void)
{
	u32 fcsePid;
	__asm__("mrc p15, 0, %0, c13, c0, 0" : "=r" (fcsePid) : );
	return fcsePid;
}

static inline void __setFcsepidr(u32 fcsePid)
{
	__asm__ volatile("mcr p15, 0, %0, c13, c0, 0" : : "r" (fcsePid) : "memory");
}

// Context ID Register
static inline u32 __getCidr(void)
{
	u32 cidr;
	__asm__("mrc p15, 0, %0, c13, c0, 1" : "=r" (cidr) : );
	return cidr;
}

static inline void __setCidr(u32 cidr)
{
	__asm__ volatile("mcr p15, 0, %0, c13, c0, 1" : : "r" (cidr) : "memory");
}

#elif ARM9

static inline void __wfi(void)
{
	__asm__ volatile("mcr p15, 0, %0, c7, c0, 4" : : "r" (0) : "memory");
}

#endif // ifdef ARM11

static inline u32 __getCpsr(void)
{
	u32 cpsr;
	__asm__("mrs %0, cpsr" : "=r" (cpsr) : );
	return cpsr;
}

static inline void __setCpsr_c(u32 cpsr)
{
	__asm__ volatile("msr cpsr_c, %0" : : "r" (cpsr) : "memory");
}

static inline void __setCpsr(u32 cpsr)
{
	__asm__ volatile("msr cpsr_cxsf, %0" : : "r" (cpsr) : "memory");
}

static inline u32 __getSpsr(void)
{
	u32 spsr;
	__asm__("mrs %0, spsr" : "=r" (spsr) : );
	return spsr;
}

static inline void __setSpsr_c(u32 spsr)
{
	__asm__ volatile("msr spsr_c, %0" : : "r" (spsr) : "memory");
}

static inline void __setSpsr(u32 spsr)
{
	__asm__ volatile("msr spsr_cxsf, %0" : : "r" (spsr) : "memory");
}

// Control Register
static inline u32 __getCr(void)
{
	u32 cr;
	__asm__("mrc p15, 0, %0, c1, c0, 0" : "=r" (cr) : );
	return cr;
}

static inline void __setCr(u32 cr)
{
	__asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r" (cr) : "memory");
}

#endif // if !__ASSEMBLER__
