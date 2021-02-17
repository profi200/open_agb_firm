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

#include "arm11/hardware/sha.h"
#include "mmio.h"



void SHA_start(u16 params)
{
	getShaRegs()->cnt = params | SHA_ENABLE;
}

void SHA_update(const u32 *data, u32 size)
{
	Sha *const sha = getShaRegs();
	while(size >= 64)
	{
		*((volatile _u512*)REGs_SHA_FIFO) = *((const _u512*)data);
		data += 64 / 4;
		size -= 64;
		while(sha->cnt & SHA_ENABLE);
	}

	if(size) iomemcpy(REGs_SHA_FIFO, data, size);
}

void SHA_finish(u32 *const hash, u16 endianess)
{
	Sha *const sha = getShaRegs();
	sha->cnt = (sha->cnt & SHA_MODE_MASK) | endianess | SHA_FINAL_ROUND;
	while(sha->cnt & SHA_ENABLE);

	SHA_getState(hash);
}

void SHA_getState(u32 *const out)
{
	Sha *const sha = getShaRegs();
	u32 size;
	switch(sha->cnt & SHA_MODE_MASK)
	{
		case SHA_256_MODE:
			size = 32;
			break;
		case SHA_224_MODE:
			size = 28;
			break;
		case SHA_1_MODE:
		default:           // 2 and 3 are both SHA1
			size = 20;
	}

	iomemcpy(out, sha->hash, size);
}

void sha(const u32 *data, u32 size, u32 *const hash, u16 params, u16 hashEndianess)
{
	SHA_start(params);
	SHA_update(data, size);
	SHA_finish(hash, hashEndianess);
}

// Note: The FIFO is 64 bit capable but 64 bit is slower than 32 bit.
/*SHA CDMA test prog:
# 4 bytes burst with 16 transfers. Total 64 bytes per burst.
# Source incrementing and destination fixed.
# Source and destination unprivileged, non-secure data access.
MOV CCR, SB16 SS32 SAI SP2 DB16 DS32 DAF DP2
MOV SAR, 0xDEADBEEF
MOV DAR, 0x10301000

FLUSHP 11


LP 8 # 128 KiB
	LP 256 # 16 KiB
		LD
		WFP 11, burst
		STPB 11
	LPEND
LPEND
WMB
SEV 1
END

void sha_dma(const u32 *data, u32 size, u32 *const hash, u16 params, u16 hashEndianess)
{
	IRQ_registerIsr(IRQ_CDMA_EVENT1, 14, 0, blah); // TODO: Move this elsewhere.

	// TODO: DMA JIT.

	SHA_start(params | SHA_I_DMA_E);
	do
	{
		__wfi();
	} while(DMA330_status(1)); // TODO: Use events.
	while(sha->cnt & SHA_ENABLE);
	SHA_finish(hash, hashEndianess);
}*/
