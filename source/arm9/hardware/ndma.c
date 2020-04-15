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

#include "types.h"
#include "fb_assert.h"
#include "arm9/hardware/ndma.h"
#include "arm9/hardware/interrupt.h"
#include "arm.h"



void NDMA_init(void)
{
	for(u32 i = 0; i < 8; i++)
	{
		REG_NDMA_CNT(i) = (REG_NDMA_CNT(i)<<1)>>1;
	}

	REG_NDMA_GLOBAL_CNT = NDMA_ROUND_ROBIN(32);

	IRQ_registerHandler(IRQ_DMAC_1_7, NULL);
}

void NDMA_copyAsync(u32 *dest, const u32 *source, u32 size)
{
	fb_assert(((u32)dest >= ITCM_BOOT9_MIRROR + ITCM_SIZE) && (((u32)dest < DTCM_BASE) || ((u32)dest >= DTCM_BASE + DTCM_SIZE)));
	fb_assert(((u32)source >= ITCM_BOOT9_MIRROR + ITCM_SIZE) && (((u32)source < DTCM_BASE) || ((u32)source >= DTCM_BASE + DTCM_SIZE)));

	REG_NDMA7_SRC_ADDR = (u32)source;
	REG_NDMA7_DST_ADDR = (u32)dest;
	REG_NDMA7_LOG_BLK_CNT = size / 4;
	REG_NDMA7_INT_CNT = NDMA_INT_SYS_FREQ;
	REG_NDMA7_CNT = NDMA_ENABLE | NDMA_IRQ_ENABLE | NDMA_IMMEDIATE_MODE | NDMA_BURST_WORDS(1) |
	                NDMA_SRC_UPDATE_INC | NDMA_DST_UPDATE_INC;
}

void NDMA_copy(u32 *dest, const u32 *source, u32 size)
{
	NDMA_copyAsync(dest, source, size);

	do
	{
		__wfi();
	} while(REG_NDMA7_CNT & NDMA_ENABLE);
}

void NDMA_fillAsync(u32 *dest, u32 value, u32 size)
{
	fb_assert(((u32)dest >= ITCM_BOOT9_MIRROR + ITCM_SIZE) && (((u32)dest < DTCM_BASE) || ((u32)dest >= DTCM_BASE + DTCM_SIZE)));

	REG_NDMA7_DST_ADDR = (u32)dest;
	REG_NDMA7_LOG_BLK_CNT = size / 4;
	REG_NDMA7_INT_CNT = NDMA_INT_SYS_FREQ;
	REG_NDMA7_FILL_DATA = value;
	REG_NDMA7_CNT = NDMA_ENABLE | NDMA_IRQ_ENABLE | NDMA_IMMEDIATE_MODE | NDMA_BURST_WORDS(1) |
	                NDMA_SRC_UPDATE_FILL | NDMA_DST_UPDATE_INC;
}

void NDMA_fill(u32 *dest, u32 value, u32 size)
{
	NDMA_fillAsync(dest, value, size);

	do
	{
		__wfi();
	} while(REG_NDMA7_CNT & NDMA_ENABLE);
}
