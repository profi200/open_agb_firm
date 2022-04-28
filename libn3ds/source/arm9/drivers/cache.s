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

#include "asm_macros.h"

.syntax unified
.cpu arm946e-s
.fpu softvfp


#define ICACHE_SIZE     (0x2000)
#define DCACHE_SIZE     (0x1000)
#define CACHE_LINE_SIZE	(32)



BEGIN_ASM_FUNC invalidateICache
	mov r0, #0
	mcr p15, 0, r0, c7, c5, 0       @ "Flush instruction cache"
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC invalidateICacheRange
	add r1, r1, r0
	bic r0, r0, #(CACHE_LINE_SIZE - 1)
	invalidateICacheRange_lp:
		mcr p15, 0, r0, c7, c5, 1   @ "Flush instruction cache single entry Address"
		add r0, r0, #CACHE_LINE_SIZE
		cmp r0, r1
		blt invalidateICacheRange_lp
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC cleanDCache
	mov r1, #0
	cleanDCache_outer_lp:
		mov r0, #0
		cleanDCache_inner_lp:
			orr r2, r1, r0             @ Generate segment and line address
			add r0, r0, #CACHE_LINE_SIZE
			mcr p15, 0, r2, c7, c10, 2 @ "Clean data cache entry Index and segment"
			cmp r0, #(DCACHE_SIZE / 4)
			bne cleanDCache_inner_lp
		add r1, r1, #0x40000000
		cmp r1, #0
		bne cleanDCache_outer_lp
	mcr p15, 0, r1, c7, c10, 4         @ Drain write buffer
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC flushDCache
	mov r1, #0
	flushDCache_outer_lp:
		mov r0, #0
		flushDCache_inner_lp:
			orr r2, r1, r0             @ Generate segment and line address
			add r0, r0, #CACHE_LINE_SIZE
			mcr p15, 0, r2, c7, c14, 2 @ Clean and flush data cache entry Index and segment
			cmp r0, #(DCACHE_SIZE / 4)
			bne flushDCache_inner_lp
		add r1, r1, #0x40000000
		cmp r1, #0
		bne flushDCache_outer_lp
	mcr p15, 0, r1, c7, c10, 4         @ Drain write buffer
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC cleanDCacheRange
	cmp r1, #DCACHE_SIZE
	bhi cleanDCache
	add r1, r1, r0
	bic r0, r0, #(CACHE_LINE_SIZE - 1)
	mov r2, #0
	cleanDCacheRange_lp:
		mcr p15, 0, r0, c7, c10, 1  @ Clean data cache entry Address
		add r0, r0, #CACHE_LINE_SIZE
		cmp r0, r1
		blt cleanDCacheRange_lp
	mcr p15, 0, r2, c7, c10, 4      @ Drain write buffer
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC flushDCacheRange
	cmp r1, #DCACHE_SIZE
	bhi flushDCache
	add r1, r1, r0
	bic r0, r0, #(CACHE_LINE_SIZE - 1)
	mov r2, #0
	flushDCacheRange_lp:
		mcr p15, 0, r0, c7, c14, 1  @ Clean and flush data cache entry Address
		add r0, r0, #CACHE_LINE_SIZE
		cmp r0, r1
		blt flushDCacheRange_lp
	mcr p15, 0, r2, c7, c10, 4      @ Drain write buffer
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC invalidateDCache
	mov r0, #0
	mcr p15, 0, r0, c7, c6, 0       @ "Flush data cache"
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC invalidateDCacheRange
	cmp r1, #DCACHE_SIZE
	bhi flushDCache
	add r1, r1, r0
	tst r0, #(CACHE_LINE_SIZE - 1)
	mcrne p15, 0, r0, c7, c10, 1    @ Clean data cache entry Address
	tst r1, #(CACHE_LINE_SIZE - 1)
	mcrne p15, 0, r1, c7, c10, 1    @ Clean data cache entry Address
	bic r0, r0, #(CACHE_LINE_SIZE - 1)
	mov r2, #0
	invalidateDCacheRange_lp:
		mcr p15, 0, r0, c7, c6, 1   @ Flush data cache single entry Address
		add r0, r0, #CACHE_LINE_SIZE
		cmp r0, r1
		blt invalidateDCacheRange_lp
	mcr p15, 0, r2, c7, c10, 4      @ Drain write buffer
	bx lr
END_ASM_FUNC
