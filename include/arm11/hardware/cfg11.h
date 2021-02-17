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


#define CFG11_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x40000)

typedef struct
{
	vu8 sharedwram_32k_code[8];
	vu8 sharedwram_32k_data[8];
	u8 _0x10[0xf0];
	vu32 nullpage_cnt;
	vu8 fiq_mask;
	vu8 unk105;                 // Debug related? Mask?
	u8 _0x106[2];
	vu8 unk108;                 // LGY gamecard related?
	u8 _0x109[3];
	vu8 cdma_cnt;
	u8 _0x10d[3];
	vu8 unk110;                 // VRAM related?
	u8 _0x111[0x2f];
	vu16 gpuprot;
	u8 _0x142[0x3e];
	vu8 wifi_power;             // Used for flight mode?
	u8 _0x181[0x3f];
	vu16 spi_cnt;
	u8 _0x1c2[0x3e];
	vu32 unk200;                // GPIO3 related? 8x4 bits.
	u8 _0x204[0x1fc];
	vu8 gpu_n3ds_cnt;           // New3DS-only.
	u8 _0x401[0xf];
	vu32 cdma_peripherals;      // New3DS-only.
	u8 _0x414[0xc];
	vu8 bootrom_overlay_cnt;    // New3DS-only.
	u8 _0x421[3];
	vu32 bootrom_overlay_val;   // New3DS-only.
	vu8 unk428;                 // New3DS-only. 1 bit. Enable CPU core 1 access to overlay regs?
	u8 _0x429[0xbd3];
	const vu16 socinfo;
} Cfg11;
static_assert(offsetof(Cfg11, socinfo) == 0xFFC, "Error: Member socinfo of Cfg11 is not at offset 0xFFC!");

ALWAYS_INLINE Cfg11* getCfg11Regs(void)
{
	return (Cfg11*)CFG11_REGS_BASE;
}


// REG_CFG11_NULLPAGE_CNT
#define NULLPAGE_CNT_FAULT_E    (1u)     // All data accesses to 0x0000-0x1000 generate faults.
#define NULLPAGE_CNT_UNKBIT16   (1u<<16)

// REG_CFG11_FIQ_MASK
#define FIQ_MASK_CPU0           (1u)
#define FIQ_MASK_CPU1           (1u<<1)
#define FIQ_MASK_CPU2           (1u<<2) // New3DS-only.
#define FIQ_MASK_CPU3           (1u<<3) // New3DS-only.

// REG_CFG11_CDMA_CNT
#define CDMA_CNT_MIC_E          (1u)
#define CDMA_CNT_NTRCARD_E      (1u<<1)
#define CDMA_CNT_CAM1_E         (1u<<2)
#define CDMA_CNT_CAM2_E         (1u<<3)
#define CDMA_CNT_SDIO2_E        (1u<<4) // WiFi
#define CDMA_CNT_SDIO3_E        (1u<<5)

// REG_CFG11_GPUPROT
// TODO

// REG_CFG11_WIFI_POWER
#define WIFI_POWER_ON           (1u)

// REG_CFG11_SPI_CNT
#define SPI_CNT_SPI1_NEW_IF     (1u)    // New interface (NSPI).
#define SPI_CNT_SPI2_NEW_IF     (1u<<1)
#define SPI_CNT_SPI3_NEW_IF     (1u<<2)

// REG_CFG11_GPU_N3DS_CNT
#define GPU_N3DS_CNT_N3DS_MODE  (1u)    // Enable access to mem extensions.
#define GPU_N3DS_CNT_TEX_FIX    (1u<<1) // Fixes some texture glitches in New3DS mode.

// REG_CFG11_CDMA_PERIPHERALS
#define CDMA_PERIPHERALS_ALL    (0x3FFFFu)

// REG_CFG11_BOOTROM_OVERLAY_CNT
#define BOOTROM_OVERLAY_CNT_E   (1u)

// REG_CFG11_SOCINFO
#define SOCINFO_CTR             (1u)    // Also set on New 3DS.
#define SOCINFO_LGR1            (1u<<1) // Never saw the daylight? Set on retail N3DS (LGR2).
#define SOCINFO_LGR2            (1u<<2) // Set on New 3DS.
