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

#include "kevent.h"


// REG_LGYFB_CNT
#define LGYFB_ENABLE             (1u)
#define LGYFB_VSCALE_E           (1u<<1)
#define LGYFB_HSCALE_E           (1u<<2)
#define LGYFB_SPATIAL_DITHER_E   (1u<<4) // Unset behaves like weight 0xCCCC in both pattern regs.
#define LGYFB_TEMPORAL_DITHER_E  (1u<<5) // Unset behaves like weight 0xCCCC in both pattern regs.
#define LGYFB_OUT_FMT_8888       (0u)
#define LGYFB_OUT_FMT_8880       (1u<<8)
#define LGYFB_OUT_FMT_5551       (2u<<8)
#define LGYFB_OUT_FMT_5650       (3u<<8)
#define LGYFB_ROT_NONE           (0u)
#define LGYFB_ROT_90CW           (1u<<10)
#define LGYFB_ROT_180CW          (2u<<10)
#define LGYFB_ROT_270CW          (3u<<10)
#define LGYFB_OUT_SWIZZLE        (1u<<12)
#define LGYFB_DMA_E              (1u<<15)
#define LGYFB_IN_FMT             (1u<<16) // Use input format but this bit does nothing?

// REG_LGYFB_SIZE width and hight
#define LGYFB_SIZE(w, h)     (((h) - 1)<<16 | ((w) - 1))

// REG_LGYFB_STAT and REG_LGYFB_IRQ
#define LGYFB_IRQ_DMA_REQ    (1u)
#define LGYFB_IRQ_BUF_ERR    (1u<<1) // FIFO overrun?
#define LGYFB_IRQ_VBLANK     (1u<<2)
#define LGYFB_IRQ_MASK       (LGYFB_IRQ_VBLANK | LGYFB_IRQ_BUF_ERR | LGYFB_IRQ_DMA_REQ)
#define LGYFB_OUT_LINE(reg)  ((reg)>>16) // STAT only



void LGYFB_init(KEvent *frameReadyEvent);
void LGYFB_deinit(void);

#ifndef NDEBUG
void LGYFB_dbgDumpFrame(void);
#endif
