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

#include <assert.h>
#include "types.h"
#include "mem_map.h"
#include "kevent.h"


#define LGYFB_TOP_REGS_BASE      (IO_MEM_ARM9_ARM11 + 0x11000)
#define LGYFB_BOT_REGS_BASE      (IO_MEM_ARM9_ARM11 + 0x10000)

#define LGYFB_TOP_FIFO           *((const vu32*)(0x10311000))
#define LGYFB_BOT_FIFO           *((const vu32*)(0x10310000))

typedef struct
{
	vu32 len;          // 0x00
	vu32 patt;         // 0x04
	u8 _0x8[0x38];
	vu32 matrix[6][8]; // 0x40
} LgyFbScaler;

typedef struct
{
	vu32 cnt;          // 0x000
	vu32 size;         // 0x004
	vu32 stat;         // 0x008
	vu32 irq;          // 0x00C
	vu32 flush;        // 0x010 Write 0 to flush LgyFb FIFO.
	u8 _0x14[0xc];
	vu32 alpha;        // 0x020 8 bit alpha for all pixels.
	u8 _0x24[0xcc];
	vu32 unkF0;        // 0x0F0
	u8 _0xf4[0xc];
	vu32 dithpatt0[2]; // 0x100 2 u32 regs with 4x2 pattern bits (mask 0xCCCC) each.
	vu32 dithpatt1[2]; // 0x108 2 u32 regs with 4x2 pattern bits (mask 0xCCCC) each.
	u8 _0x110[0xf0];
	LgyFbScaler v;     // 0x200
	LgyFbScaler h;     // 0x300
} LgyFb;
static_assert(offsetof(LgyFb, h.matrix[5][7]) == 0x3FC, "Error: Member h.matrix[5][7] of LgyFb is not at offset 0x3FC!");

ALWAYS_INLINE LgyFb* getLgyFbRegs(bool top)
{
	return (LgyFb*)(top ? LGYFB_TOP_REGS_BASE : LGYFB_BOT_REGS_BASE);
}


// REG_LGYFB_CNT
#define LGYFB_EN                 (1u)
#define LGYFB_VSCALE_EN          (1u<<1)
#define LGYFB_HSCALE_EN          (1u<<2)
#define LGYFB_SPATIAL_DITHER_EN  (1u<<4) // Unset behaves like weight 0xCCCC in both pattern regs.
#define LGYFB_TEMPORAL_DITHER_EN (1u<<5) // Unset behaves like weight 0xCCCC in both pattern regs.
#define LGYFB_OUT_FMT_8888       (0u)
#define LGYFB_OUT_FMT_8880       (1u<<8)
#define LGYFB_OUT_FMT_5551       (2u<<8)
#define LGYFB_OUT_FMT_5650       (3u<<8)
#define LGYFB_ROT_NONE           (0u)
#define LGYFB_ROT_90CW           (1u<<10)
#define LGYFB_ROT_180CW          (2u<<10)
#define LGYFB_ROT_270CW          (3u<<10)
#define LGYFB_OUT_SWIZZLE        (1u<<12)
#define LGYFB_DMA_EN             (1u<<15)
#define LGYFB_IN_FMT             (1u<<16) // Use input format but this bit does nothing?

// REG_LGYFB_SIZE width and hight
#define LGYFB_SIZE(w, h)     (((h) - 1)<<16 | ((w) - 1))

// REG_LGYFB_STAT and REG_LGYFB_IRQ
#define LGYFB_IRQ_DMA_REQ    (1u)
#define LGYFB_IRQ_BUF_ERR    (1u<<1) // FIFO overrun?
#define LGYFB_IRQ_VBLANK     (1u<<2)
#define LGYFB_IRQ_MASK       (LGYFB_IRQ_VBLANK | LGYFB_IRQ_BUF_ERR | LGYFB_IRQ_DMA_REQ)
#define LGYFB_OUT_LINE(reg)  ((reg)>>16) // STAT only



void LGYFB_init(KHandle frameReadyEvent, u8 scaler);
void LGYFB_deinit(void);
void LGYFB_stop(void);
void LGYFB_start(void);

#ifndef NDEBUG
void LGYFB_dbgDumpFrame(void);
#endif
