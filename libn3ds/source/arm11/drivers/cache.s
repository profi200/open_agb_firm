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
.cpu mpcore
.fpu vfpv2


#define ICACHE_SIZE     (0x4000)
#define DCACHE_SIZE     (0x4000)
#define CACHE_LINE_SIZE	(32)



BEGIN_ASM_FUNC invalidateICache
	mov r0, #0
	mcr p15, 0, r0, c7, c5, 0       @ Invalidate Entire Instruction Cache, also flushes the branch target cache
	@mcr p15, 0, r0, c7, c5, 6       @ Flush Entire Branch Target Cache
	mcr p15, 0, r0, c7, c10, 4      @ Data Synchronization Barrier
	mcr p15, 0, r0, c7, c5, 4       @ Flush Prefetch Buffer
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC invalidateICacheRange
	add r1, r1, r0
	bic r0, r0, #(CACHE_LINE_SIZE - 1)
	mov r2, #0
	invalidateICacheRange_lp:
		mcr p15, 0, r0, c7, c5, 1   @ Invalidate Instruction Cache Line (using MVA)
		add r0, r0, #CACHE_LINE_SIZE
		cmp r0, r1
		blt invalidateICacheRange_lp
	mcr p15, 0, r2, c7, c5, 6       @ Flush Entire Branch Target Cache
	mcr p15, 0, r2, c7, c10, 4      @ Data Synchronization Barrier
	mcr p15, 0, r2, c7, c5, 4       @ Flush Prefetch Buffer
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC cleanDCache
	mov r0, #0
	mcr p15, 0, r0, c7, c10, 0      @ Clean Entire Data Cache
	mcr p15, 0, r0, c7, c10, 4      @ Data Synchronization Barrier
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC flushDCache
	mov r0, #0
	mcr p15, 0, r0, c7, c14, 0      @ Clean and Invalidate Entire Data Cache
	mcr p15, 0, r0, c7, c10, 4      @ Data Synchronization Barrier
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC cleanDCacheRange
	cmp r1, #DCACHE_SIZE
	bhi cleanDCache
	add r1, r1, r0
	bic r0, r0, #(CACHE_LINE_SIZE - 1)
	mov r2, #0
	cleanDCacheRange_lp:
		mcr p15, 0, r0, c7, c10, 1  @ Clean Data Cache Line (using MVA)
		add r0, r0, #CACHE_LINE_SIZE
		cmp r0, r1
		blt cleanDCacheRange_lp
	mcr p15, 0, r2, c7, c10, 4      @ Data Synchronization Barrier
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC flushDCacheRange
	cmp r1, #DCACHE_SIZE
	bhi flushDCache
	add r1, r1, r0
	bic r0, r0, #(CACHE_LINE_SIZE - 1)
	mov r2, #0
	flushDCacheRange_lp:
		mcr p15, 0, r0, c7, c14, 1  @ Clean and Invalidate Data Cache Line (using MVA)
		add r0, r0, #CACHE_LINE_SIZE
		cmp r0, r1
		blt flushDCacheRange_lp
	mcr p15, 0, r2, c7, c10, 4      @ Data Synchronization Barrier
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC invalidateDCache
	mov r0, #0
	mcr p15, 0, r0, c7, c6, 0       @ Invalidate Entire Data Cache
	mcr p15, 0, r0, c7, c10, 4      @ Data Synchronization Barrier
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC invalidateDCacheRange
	cmp r1, #DCACHE_SIZE
	bhi flushDCache
	add r1, r1, r0
	tst r0, #(CACHE_LINE_SIZE - 1)
	mcrne p15, 0, r0, c7, c10, 1    @ Clean Data Cache Line (using MVA)
	tst r1, #(CACHE_LINE_SIZE - 1)
	mcrne p15, 0, r1, c7, c10, 1    @ Clean Data Cache Line (using MVA)
	bic r0, r0, #(CACHE_LINE_SIZE - 1)
	mov r2, #0
	invalidateDCacheRange_lp:
		mcr p15, 0, r0, c7, c6, 1   @ Invalidate Data Cache Line (using MVA)
		add r0, r0, #CACHE_LINE_SIZE
		cmp r0, r1
		blt invalidateDCacheRange_lp
	mcr p15, 0, r2, c7, c10, 4      @ Data Synchronization Barrier
	bx lr
END_ASM_FUNC
