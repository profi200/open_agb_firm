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

#include "types.h"
#include "mem_map.h"



#define GX_REGS_BASE             (IO_MEM_ARM11_ONLY + 0x200000)
#define REG_GX_GPU_CLK           *((vu32*)(GX_REGS_BASE + 0x0004)) // ?

// PSC (memory fill) regs.
#define REG_GX_PSC_FILL0_S_ADDR  *((vu32*)(GX_REGS_BASE + 0x0010)) // Start address
#define REG_GX_PSC_FILL0_E_ADDR  *((vu32*)(GX_REGS_BASE + 0x0014)) // End address
#define REG_GX_PSC_FILL0_VAL     *((vu32*)(GX_REGS_BASE + 0x0018)) // Fill value
#define REG_GX_PSC_FILL0_CNT     *((vu32*)(GX_REGS_BASE + 0x001C))

#define REG_GX_PSC_FILL1_S_ADDR  *((vu32*)(GX_REGS_BASE + 0x0020))
#define REG_GX_PSC_FILL1_E_ADDR  *((vu32*)(GX_REGS_BASE + 0x0024))
#define REG_GX_PSC_FILL1_VAL     *((vu32*)(GX_REGS_BASE + 0x0028))
#define REG_GX_PSC_FILL1_CNT     *((vu32*)(GX_REGS_BASE + 0x002C))

#define REG_GX_PSC_VRAM          *((vu32*)(GX_REGS_BASE + 0x0030)) // gsp mudule only changes bit 8-11.
#define REG_GX_PSC_STAT          *((vu32*)(GX_REGS_BASE + 0x0034))

// PDC0/1 regs see lcd.h.

// PPF (transfer engine) regs.
#define REG_GX_PPF_IN_ADDR       *((vu32*)(GX_REGS_BASE + 0x0C00))
#define REG_GX_PPF_OUT_ADDR      *((vu32*)(GX_REGS_BASE + 0x0C04))
#define REG_GX_PPF_DT_OUTDIM     *((vu32*)(GX_REGS_BASE + 0x0C08)) // Display transfer output dimensions.
#define REG_GX_PPF_DT_INDIM      *((vu32*)(GX_REGS_BASE + 0x0C0C)) // Display transfer input dimensions.
#define REG_GX_PPF_FlAGS         *((vu32*)(GX_REGS_BASE + 0x0C10))
#define REG_GX_PPF_UNK14         *((vu32*)(GX_REGS_BASE + 0x0C14)) // Transfer interval?
#define REG_GX_PPF_CNT           *((vu32*)(GX_REGS_BASE + 0x0C18))
#define REG_GX_PPF_IRQ_POS       *((vu32*)(GX_REGS_BASE + 0x0C1C)) // ?
#define REG_GX_PPF_LEN           *((vu32*)(GX_REGS_BASE + 0x0C20)) // Texture copy size in bytes.
#define REG_GX_PPF_TC_INDIM      *((vu32*)(GX_REGS_BASE + 0x0C24)) // Texture copy input width and gap in 16 byte units.
#define REG_GX_PPF_TC_OUTDIM     *((vu32*)(GX_REGS_BASE + 0x0C28)) // Texture copy output width and gap in 16 byte units.

// P3D (GPU internal) regs. See gpu_regs.h.
#define REG_GX_P3D(reg)          *((vu32*)(GX_REGS_BASE + 0x1000 + ((reg) * 4)))
