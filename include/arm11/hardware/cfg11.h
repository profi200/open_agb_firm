#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2018 derrek, profi200
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

#include "mem_map.h"


#define CFG11_REGS_BASE                (IO_MEM_ARM9_ARM11 + 0x40000)
#define REG_CFG11_FIQ_MASK             *((       vu8*)(CFG11_REGS_BASE + 0x104))
#define REG_CFG11_UNK105               *((       vu8*)(CFG11_REGS_BASE + 0x105)) // Debug related? Mask?
#define REG_CFG11_UNK108               *((       vu8*)(CFG11_REGS_BASE + 0x108)) // LGY gamecard related?
#define REG_CFG11_CDMA_CNT             *((       vu8*)(CFG11_REGS_BASE + 0x10C))
#define REG_CFG11_UNK110               *((       vu8*)(CFG11_REGS_BASE + 0x110)) // VRAM related?
#define REG_CFG11_GPUPROT              *((      vu16*)(CFG11_REGS_BASE + 0x140))
#define REG_CFG11_WIFI_POWER           *((       vu8*)(CFG11_REGS_BASE + 0x180)) // Used for flight mode?
#define REG_CFG11_SPI_CNT              *((      vu16*)(CFG11_REGS_BASE + 0x1C0))
#define REG_CFG11_UNK200               *((      vu32*)(CFG11_REGS_BASE + 0x200)) // GPIO3 related? 8x4 bits.
#define REG_CFG11_GPU_N3DS_CNT         *((       vu8*)(CFG11_REGS_BASE + 0x400)) // New3DS-only.
#define REG_CFG11_CDMA_PERIPHERALS     *((      vu32*)(CFG11_REGS_BASE + 0x410)) // New3DS-only.
#define REG_CFG11_BOOTROM_OVERLAY_CNT  *((       vu8*)(CFG11_REGS_BASE + 0x420)) // New3DS-only.
#define REG_CFG11_BOOTROM_OVERLAY_VAL  *((      vu32*)(CFG11_REGS_BASE + 0x424)) // New3DS-only.
#define REG_CFG11_UNK428               *((       vu8*)(CFG11_REGS_BASE + 0x428)) // New3DS-only. 1 bit. Enable CPU core 1 access to overlay regs?
#define REG_CFG11_SOCINFO              *((const vu16*)(CFG11_REGS_BASE + 0xFFC))


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
#define SOCINFO_O3DS            (1u)    // Also set on New3DS.
#define SOCINFO_N3DS_PROTO      (1u<<1) // Never saw the daylight?
#define SOCINFO_N3DS            (1u<<2) // Set on New3DS.
