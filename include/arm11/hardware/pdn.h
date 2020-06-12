#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2020 derrek, profi200
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


#define PDN_REGS_BASE                (IO_MEM_ARM9_ARM11 + 0x41000)
#define REG_PDN_CNT                  *((vu16*)(PDN_REGS_BASE + 0x000))
#define REG_PDN_WAKE_ENABLE          *((vu32*)(PDN_REGS_BASE + 0x008))
#define REG_PDN_WAKE_REASON          *((vu32*)(PDN_REGS_BASE + 0x00C)) // Write 1 to acknowledge and 0 to clear?
#define REG_PDN_GPU_CNT              *((vu32*)(PDN_REGS_BASE + 0x200))
#define REG_PDN_VRAM_CNT             *((vu8* )(PDN_REGS_BASE + 0x204)) // This reg doesn't seem to exist on retail hardware.
#define REG_PDN_LCD_CNT              *((vu8* )(PDN_REGS_BASE + 0x208)) // This reg doesn't seem to exist on retail hardware.
#define REG_PDN_FCRAM_CNT            *((vu8* )(PDN_REGS_BASE + 0x210))
#define REG_PDN_I2S_CNT              *((vu8* )(PDN_REGS_BASE + 0x220))
#define REG_PDN_CAM_CNT              *((vu8* )(PDN_REGS_BASE + 0x224))
#define REG_PDN_DSP_CNT              *((vu8* )(PDN_REGS_BASE + 0x230))
#define REG_PDN_G1_CNT               *((vu8* )(PDN_REGS_BASE + 0x240)) // Hantro G1 decoder.
#define REG_PDN_MPCORE_SOCMODE       *((vu16*)(PDN_REGS_BASE + 0x300))
#define REG_PDN_MPCORE_CNT           *((vu16*)(PDN_REGS_BASE + 0x304)) // Is this reg actually only vu8?
#define REGs_PDN_MPCORE_BOOTCNT       ((vu8* )(PDN_REGS_BASE + 0x310))


// REG_PDN_CNT
#define PDN_CNT_SLEEP             (1u)     // Set this bit to enter sleep mode.
#define PDN_CNT_VRAM_OFF          (1u<<15) // Set when VRAM is powered off.

// REG_PDN_WAKE_ENABLE and REG_PDN_WAKE_REASON
enum
{
	PDN_WAKE_PADCNT                = 1u,
	PDN_WAKE_SHELL_OPENED          = 1u<<3,
	PDN_WAKE_HEADPH_NOT_PLUGGED_IN = 1u<<4, // Really?
	PDN_WAKE_UNK6                  = 1u<<6, // DSi mode related.
	PDN_WAKE_SDIO1                 = 1u<<7,
	PDN_WAKE_SDIO2                 = 1u<<8,
	PDN_WAKE_SDIO3                 = 1u<<16,
	// 17-28 maybe GPIO3 0-11?
	PDN_WAKE_GAMECARD_INSERT       = 1u<<29, // ?
	PDN_WAKE_TOUCHPEN_DOWN         = 1u<<30,
	PDN_WAKE_UNK31                 = 1u<<31  // Also shell related?
};

// REG_PDN_GPU_CNT
// Note: The resets are active low.
enum
{
	PDN_GPU_CNT_RST_REGS           = 1u,    // And more?
	PDN_GPU_CNT_RST_PSC            = 1u<<1, // ?
	PDN_GPU_CNT_RST_GEOSHADER      = 1u<<2, // ?
	PDN_GPU_CNT_RST_RASTERIZER     = 1u<<3, // ?
	PDN_GPU_CNT_RST_PPF            = 1u<<4,
	PDN_GPU_CNT_RST_PDC            = 1u<<5, // ?
	PDN_GPU_CNT_RST_PDC2           = 1u<<6, // Maybe pixel pipeline or so?

	PDN_GPU_CNT_RST_ALL            = (PDN_GPU_CNT_RST_PDC2<<1) - 1
};

#define PDN_GPU_CNT_CLK_E         (1u<<16)

// REG_PDN_VRAM_CNT
#define PDN_VRAM_CNT_CLK_E        (1u)

// REG_PDN_LCD_CNT
#define PDN_LCD_CNT_PWR_MGR_OFF   (1u) // Power management off?

// REG_PDN_FCRAM_CNT
// Note: Reset is active low.
#define PDN_FCRAM_CNT_RST         (1u)
#define PDN_FCRAM_CNT_CLK_E       (1u<<1)
#define PDN_FCRAM_CNT_CLK_E_ACK   (1u<<2) // Gets set or unset depending on CLK_E.

// REG_PDN_I2S_CNT
#define PDN_I2S_CNT_I2S_CLK1_E    (1u)    // ? Unused?
#define PDN_I2S_CNT_I2S_CLK2_E    (1u<<1)

// REG_PDN_CAM_CNT
#define PDN_CAM_CNT_CLK_E         (1u)

// REG_PDN_DSP_CNT
// Note: Reset is active low.
#define PDN_DSP_CNT_RST           (1u)
#define PDN_DSP_CNT_CLK_E         (1u<<1)

// REG_PDN_G1_CNT
// TODO: Active low or high?
#define PDN_G1_CNT_RST            (1u)

// REG_PDN_MPCORE_SOCMODE
typedef enum
{
	SOCMODE_O3DS_268MHz            = 0u,
	SOCMODE_N3DS_268MHz            = 1u, // Also enables FCRAM extension.
	SOCMODE_N3DS_PROTO_268MHz      = 2u, // Also enables FCRAM extension?
	SOCMODE_N3DS_PROTO_536MHz      = 3u, // Also enables FCRAM extension?
	SOCMODE_N3DS_804MHz            = 5u, // Also enables FCRAM extension.

	SOCMODE_MASK                   = 7u
} PdnSocmode;

#define PDN_MPCORE_SOCMODE_ACK    (1u<<15)

// REG_PDN_MPCORE_CNT
#define PDN_MPCORE_CNT_MEM_EXT_E  (1u)    // Does it actually affect all mem extensions or just QTM?
#define PDN_MPCORE_CNT_L2C_E      (1u<<8)

// REGs_PDN_MPCORE_BOOTCNT
// Note: Reset is active low.
#define MPCORE_BOOTCNT_RST        (1u)    // Core 2/3 only. Reset and instruction overlay enable.
#define MPCORE_BOOTCNT_D_OVERL_E  (1u<<1) // Core 2/3 only. Data overlay enable. Also used to signal a core booted.
#define MPCORE_BOOTCNT_RST_STAT   (1u<<4)
#define MPCORE_BOOTCNT_UNK        (1u<<5)



void PDN_core123Init(void);
void PDN_setSocmode(PdnSocmode socmode);
void PDN_poweroffCore23(void);
