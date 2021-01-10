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
#include "mem_map.h"
#include "arm11/hardware/hash.h"
#include "mmio.h"



//////////////////////////////////
//             HASH             //
//////////////////////////////////

#define HASH_REGS_BASE    (IO_MEM_ARM9_ARM11 + 0x1000)
#define REG_HASH_CNT      *((vu32*)(HASH_REGS_BASE + 0x00))
#define REG_HASH_BLKCNT   *((vu32*)(HASH_REGS_BASE + 0x04))
#define REGs_HASH_HASH     ((vu32*)(HASH_REGS_BASE + 0x40))
#define REGs_HASH_INFIFO   ((vu32*)(IO_MEM_ARM11_ONLY + 0x101000)) // INFIFO is in the DMA region


void HASH_start(u8 params)
{
	REG_HASH_CNT = params | HASH_IN_DMA_ENABLE | HASH_ENABLE;
}

void HASH_update(const u32 *data, u32 size)
{
	while(size >= 64)
	{
		*((volatile _u512*)REGs_HASH_INFIFO) = *((const _u512*)data);
		data += 64 / 4;
		size -= 64;
		while(REG_HASH_CNT & HASH_ENABLE);
	}

	if(size) iomemcpy(REGs_HASH_INFIFO, data, size);
}

void HASH_finish(u32 *const hash, u8 endianess)
{
	REG_HASH_CNT = (REG_HASH_CNT & HASH_MODE_MASK) | endianess | HASH_FINAL_ROUND;
	while(REG_HASH_CNT & HASH_ENABLE);

	HASH_getState(hash);
}

void HASH_getState(u32 *const out)
{
	u32 size;
	switch(REG_HASH_CNT & HASH_MODE_MASK)
	{
		case HASH_MODE_256:
			size = 32;
			break;
		case HASH_MODE_224:
			size = 28;
			break;
		case HASH_MODE_1:
		default:           // 2 and 3 are both SHA1
			size = 20;
	}

	iomemcpy(out, REGs_HASH_HASH, size);
}

void hash(const u32 *data, u32 size, u32 *const hash, u8 params, u8 hashEndianess)
{
	HASH_start(params);
	HASH_update(data, size);
	HASH_finish(hash, hashEndianess);
}
