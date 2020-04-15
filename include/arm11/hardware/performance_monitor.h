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

// Refer to http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0360f/index.html
// (comprocessor regs c15) for documentation.

#include "types.h"



static inline void startProfiling(u16 pmnEvents, u8 intMask, bool ccntDiv64, u8 reset)
{
	const u32 tmp = pmnEvents<<12 | 7u<<8 | intMask<<4 | ccntDiv64<<3 | reset<<1 | 1u;
	__asm__ volatile("mcr p15, 0, %0, c15, c12, 0" : : "r" (tmp) : "memory");
}

static inline void stopProfiling(void)
{
	__asm__ volatile("mcr p15, 0, %0, c15, c12, 0" : : "r" (7u<<8) : "memory");
}

static inline void setCcnt(u32 val)
{
	__asm__ volatile("mcr p15, 0, %0, c15, c12, 1" : : "r" (val) : "memory");
}

static inline void setPmn0(u32 val)
{
	__asm__ volatile("mcr p15, 0, %0, c15, c12, 2" : : "r" (val) : "memory");
}

static inline void setPmn1(u32 val)
{
	__asm__ volatile("mcr p15, 0, %0, c15, c12, 3" : : "r" (val) : "memory");
}

static inline u32 getCcnt(void)
{
	u32 tmp;
	__asm__ volatile("mrc p15, 0, %0, c15, c12, 1" : "=r" (tmp) : : "memory");
	return tmp;
}

static inline u32 getPmn0(void)
{
	u32 tmp;
	__asm__ volatile("mrc p15, 0, %0, c15, c12, 2" : "=r" (tmp) : : "memory");
	return tmp;
}

static inline u32 getPmn1(void)
{
	u32 tmp;
	__asm__ volatile("mrc p15, 0, %0, c15, c12, 3" : "=r" (tmp) : : "memory");
	return tmp;
}
