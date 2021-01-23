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

#include "mem_map.h"


#define CFG9_REGS_BASE              (IO_MEM_ARM9_ONLY)
#define REG_CFG9_SYSPROT9           *((      vu8* )(CFG9_REGS_BASE + 0x00000))
#define REG_CFG9_SYSPROT11          *((      vu8* )(CFG9_REGS_BASE + 0x00001))
#define REG_CFG9_UNK00002           *((      vu8* )(CFG9_REGS_BASE + 0x00002)) // Bit 0 bootrom write enable? Cleared immediately in boot9.
#define REG_CFG9_UNK00004           *((      vu16*)(CFG9_REGS_BASE + 0x00004)) // JTAG related?
#define REG_CFG9_XDMA_PERIPHALS     *((      vu8* )(CFG9_REGS_BASE + 0x00008)) // DMA request each bit 0 = NDMA, 1 = XDMA.
#define REG_CFG9_CARDCTL            *((      vu16*)(CFG9_REGS_BASE + 0x0000C))
#define REG_CFG9_CARD_POWER         *((      vu8* )(CFG9_REGS_BASE + 0x00010))
#define REG_CFG9_CARD_INSERT_DELAY  *((      vu16*)(CFG9_REGS_BASE + 0x00012)) // Insert delay in 0x400 cycle units.
#define REG_CFG9_CARD_PWROFF_DELAY  *((      vu16*)(CFG9_REGS_BASE + 0x00014)) // Power off delay in 0x400 cycle units.
#define REG_CFG9_SDMMCCTL           *((      vu16*)(CFG9_REGS_BASE + 0x00020))
#define REG_CFG9_UNK00100           *((      vu16*)(CFG9_REGS_BASE + 0x00100)) // Similar to SCFG_EXT regs on DSi?
#define REG_CFG9_EXTMEMCNT9         *((      vu8* )(CFG9_REGS_BASE + 0x00200))
#define REG_CFG9_SOCINFO            *((const vu16*)(CFG9_REGS_BASE + 0x00FFC)) // Same as REG_CFG11_SOCINFO.
#define REG_CFG9_BOOTENV            *((      vu32*)(CFG9_REGS_BASE + 0x10000))
#define REG_CFG9_UNITINFO           *((const vu8* )(CFG9_REGS_BASE + 0x10010))
#define REG_CFG9_TWLUNITINFO        *((      vu8* )(CFG9_REGS_BASE + 0x10014)) // Writable reg for TWL mode.
#define REG_CFG9_UNK10020           *((      vu8* )(CFG9_REGS_BASE + 0x10020)) // Bootrom related?


// REG_CFG9_SYSPROT9
#define SYSPROT9_ROM_H2_LOCK    (1u)    // Disables access to the second half of the ARM9 bootrom. Also enables FCRAM access.
#define SYSPROT9_OTP_LOCK       (1u<<1) // Disables access to the OTP.

// REG_CFG9_SYSPROT11
#define SYSPROT11_ROM_H2_LOCK   (1u) // Disables access to the second half of the ARM11 bootrom. Also enables FCRAM access.

// REG_CFG9_XDMA_PERIPHALS
#define XDMA_PERIPHALS_TMIO1    (1u)    // TMIO controller 1 (SD/eMMC).
#define XDMA_PERIPHALS_TMIO3    (1u<<1) // TMIO controller 3.
#define XDMA_PERIPHALS_AES_IN   (1u<<2)
#define XDMA_PERIPHALS_AES_OUT  (1u<<3)

// REG_CFG9_CARDCTL
#define CARDCTL_NTRCARD         (0u)    // Controller at 0x10164000.
#define CARDCTL_UNK1            (1u)    // Unknown controller/function.
#define CARDCTL_CTRCARD1        (2u)    // Controller at 0x10004000.
#define CARDCTL_CTRCARD2        (3u)    // Controller at 0x10005000.
#define CARDCTL_NSPI_FIFO_MODE  (1u<<4) // TODO: Confirm this. If set use regs at 0x1000D800 otherwise 0x1000D000.
#define CARDCTL_NSPI_SEL        (1u<<8) // If set use regs at 0x1000D000/0x1000D800 otherwise regs at 0x10164000.

// REG_CFG9_CARD_POWER
#define CARD_POWER_EJECTED      (1u)    // No card inserted.
#define CARD_POWER_OFF          (0u<<2) // Slot powered off.
#define CARD_POWER_ON_RESET     (1u<<2) // Powered on and in reset.
#define CARD_POWER_ON           (2u<<2) // Powered on.
#define CARD_POWER_OFF_REQ      (3u<<2) // Power off request.

// REG_CFG9_SDMMCCTL
#define SDMMCCTL_SD_PWR_OFF     (1u)    // Controller 1/3 port 0.
#define SDMMCCTL_eMMC_PWR_OFF   (1u<<1) // Controller 1 port 1.
#define SDMMCCTL_WiFi_PWR_OFF   (1u<<2) // Controller 2 port 0.
#define SDMMCCTL_UNK_PWR_OFF    (1u<<3) // Controller 3 port 1 power off? Set at cold boot.
#define SDMMCCTL_UNKBIT6        (1u<<6) // Wifi port related? Pull up? Set at cold boot.
#define SDMMCCTL_TMIO3_MAP11    (1u<<8) // Controller 3 mapping (0=ARM9 0x10007000, 1=ARM11 0x10100000).
#define SDMMCCTL_SD_TMIO1_SEL   (1u<<9) // SD card controller select (0=TMIO3 0x10007000/0x10100000, 1=TMIO1 0x10006000).

// REG_CFG9_EXTMEMCNT9
#define EXTMEMCNT9_WRAM_EXT_E   (1u) // Enables extra WRAM aka. ARM9 mem extension.

// REG_CFG9_SOCINFO
#define SOCINFO_CTR             (1u)    // Also set on New 3DS.
#define SOCINFO_LGR1            (1u<<1) // Never saw the daylight? Set on retail N3DS (LGR2).
#define SOCINFO_LGR2            (1u<<2) // Set on New 3DS.

// REG_CFG9_BOOTENV
#define BOOTENV_COLD_BOOT       (0u)
#define BOOTENV_NATIVE_FIRM     (1u)
#define BOOTENV_TWL_FIRM        (3u)
#define BOOTENV_AGB_FIRM        (7u)

// REG_CFG9_UNITINFO
// TODO: Collect all the possible values for this reg.
#define UNITINFO_RETAIL         (0u)
