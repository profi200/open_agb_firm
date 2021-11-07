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

#include "types.h"
#include "fb_assert.h"
#include "arm9/drivers/ndma.h"
#include "arm9/drivers/interrupt.h"
#include "arm.h"


#define MAX_BURST_WORDS  (5u) // In log2. 5 is 32 words (128 bytes).



static_assert(IRQ_DMAC_1_7 == 7, "Error: IRQ number for NDMA channel 7 is not 7!");
void NDMA_init(void)
{
	for(u32 i = 0; i < 8; i++)
	{
		getNdmaChRegs(i)->cnt = 0;

		// Channel and IRQ numbers are in order so we can get away with this.
		IRQ_registerIsr(IRQ_DMAC_1_0 + i, NULL);
	}

	// Note: The readback bit is not supported in DSi mode.
	// For ARM7 NDMA in DSi mode usually 16 cycles is used.
	REG_NDMA_GCNT = NDMA_ROUND_ROBIN(32) | NDMA_REG_READBACK;
}

void NDMA_copy(u32 *dest, const u32 *source, u32 size)
{
	fb_assert(((u32)dest >= ITCM_BOOT9_MIRROR + ITCM_SIZE) && (((u32)dest < DTCM_BASE) || ((u32)dest >= DTCM_BASE + DTCM_SIZE)));
	fb_assert(((u32)source >= ITCM_BOOT9_MIRROR + ITCM_SIZE) && (((u32)source < DTCM_BASE) || ((u32)source >= DTCM_BASE + DTCM_SIZE)));

	size /= 4; // Sizes need to be in words.

	// TODO: Test iomemcpy() with struct vs. setting the regs manually.
	NdmaCh *const ndmaCh = getNdmaChRegs(7);
	ndmaCh->sad  = (u32)source;
	ndmaCh->dad  = (u32)dest;
	ndmaCh->wcnt = size;
	ndmaCh->bcnt = NDMA_FASTEST;

	u32 burst = __builtin_ctzl(size); // 31u - __builtin_clzl(-(s32)size & size);
	if(burst > MAX_BURST_WORDS) burst = MAX_BURST_WORDS;
	ndmaCh->cnt  = NDMA_EN | NDMA_IRQ_EN | NDMA_START_IMMEDIATE |
	               burst<<NDMA_BURST_SHIFT | NDMA_SAD_INC | NDMA_DAD_INC;

	do
	{
		__wfi();
	} while(ndmaCh->cnt & NDMA_EN);
}

void NDMA_fill(u32 *dest, u32 value, u32 size)
{
	fb_assert(((u32)dest >= ITCM_BOOT9_MIRROR + ITCM_SIZE) && (((u32)dest < DTCM_BASE) || ((u32)dest >= DTCM_BASE + DTCM_SIZE)));

	size /= 4; // Sizes need to be in words.

	NdmaCh *const ndmaCh = getNdmaChRegs(7);
	ndmaCh->dad   = (u32)dest;
	ndmaCh->wcnt  = size;
	ndmaCh->bcnt  = NDMA_FASTEST;
	ndmaCh->fdata = value;

	u32 burst = __builtin_ctzl(size); // 31u - __builtin_clzl(-(s32)size & size);
	if(burst > MAX_BURST_WORDS) burst = MAX_BURST_WORDS;
	ndmaCh->cnt   = NDMA_EN | NDMA_IRQ_EN | NDMA_START_IMMEDIATE |
	                burst<<NDMA_BURST_SHIFT | NDMA_SAD_FILL | NDMA_DAD_INC;

	do
	{
		__wfi();
	} while(ndmaCh->cnt & NDMA_EN);
}
