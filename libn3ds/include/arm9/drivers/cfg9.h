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


#define CFG9_REGS_BASE  (IO_MEM_ARM9_ONLY)

typedef struct
{
	vu8 sysprot9;           // 0x00000
	vu8 sysprot11;          // 0x00001
	vu8 unk00002;           // 0x00002 Bit 0 bootrom write enable? Cleared immediately in boot9.
	u8 _0x3;
	vu16 unk00004;          // 0x00004 JTAG related?
	u8 _0x6[2];
	vu8 xdma_req;           // 0x00008 Enable requests for XDMA. Each bit 1 = enabled.
	u8 _0x9[3];
	vu16 cardctl;           // 0x0000C
	u8 _0xe[2];
	vu8 card_power;         // 0x00010
	u8 _0x11;
	vu16 card_insert_delay; // 0x00012 Insert delay in 0x400 cycle units.
	vu16 card_pwroff_delay; // 0x00014 Power off delay in 0x400 cycle units.
	u8 _0x16[0xa];
	vu16 sdmmcctl;          // 0x00020
	u8 _0x22[0xde];
	vu16 unk00100;          // 0x00100 Similar to SCFG_EXT regs on DSi?
	u8 _0x102[0xfe];
	vu8 extmemcnt9;         // 0x00200
	u8 _0x201[0xdfb];
	const vu16 socinfo;     // 0x00FFC Same as REG_CFG11_SOCINFO.
	u8 _0xffe[0xf002];
	vu32 bootenv;           // 0x10000
	u8 _0x10004[0xc];
	const vu8 unitinfo;     // 0x10010
	u8 _0x10011[3];
	vu8 twlunitinfo;        // 0x10014 Writable reg for TWL mode.
	u8 _0x10015[0xb];
	vu8 unk10020;           // 0x10020 Bootrom related?
} Cfg9;
static_assert(offsetof(Cfg9, unk10020) == 0x10020, "Error: Member unk10020 of Cfg9 is not at offset 0x10020!");

ALWAYS_INLINE Cfg9* getCfg9Regs(void)
{
	return (Cfg9*)CFG9_REGS_BASE;
}


// REG_CFG9_SYSPROT9
#define SYSPROT9_ROM_H2_LOCK       (1u)    // Disables access to the second half of the ARM9 bootrom. Also enables FCRAM access.
#define SYSPROT9_OTP_LOCK          (1u<<1) // Disables access to the OTP.

// REG_CFG9_SYSPROT11
#define SYSPROT11_ROM_H2_LOCK      (1u) // Disables access to the second half of the ARM11 bootrom. Also enables FCRAM access.

// REG_CFG9_XDMA_REQ
#define XDMA_REQ_TOSHSD1           (1u)    // Toshsd controller 1 (SD card slot/eMMC).
#define XDMA_REQ_TOSHSD3           (1u<<1) // Toshsd controller 3 (SD card slot).
#define XDMA_REQ_AES_IN            (1u<<2)
#define XDMA_REQ_AES_OUT           (1u<<3)

// REG_CFG9_CARDCTL
#define CARDCTL_NTRCARD            (0u)    // Controller at 0x10164000.
#define CARDCTL_UNK1               (1u)    // Unknown controller/function.
#define CARDCTL_CTRCARD1           (2u)    // Controller at 0x10004000.
#define CARDCTL_CTRCARD2           (3u)    // Controller at 0x10005000.
#define CARDCTL_SPIC_FIFO_MODE     (1u<<4) // TODO: Confirm this. If set use regs at 0x1000D800 otherwise 0x1000D000.
#define CARDCTL_SPIC_SEL           (1u<<8) // If set use regs at 0x1000D000/0x1000D800 otherwise regs at 0x10164000.

// REG_CFG9_CARD_POWER
#define CARD_POWER_EJECTED         (1u)    // No card inserted.
#define CARD_POWER_OFF             (0u<<2) // Slot powered off.
#define CARD_POWER_ON_RESET        (1u<<2) // Powered on and in reset.
#define CARD_POWER_ON              (2u<<2) // Powered on.
#define CARD_POWER_OFF_REQ         (3u<<2) // Power off request.

// REG_CFG9_SDMMCCTL
#define SDMMCCTL_CARD_PWR_OFF      (1u)    // Controller 1/3 port 0 (MMC/SD card slot).
#define SDMMCCTL_eMMC_PWR_OFF      (1u<<1) // Controller 1 port 1.
#define SDMMCCTL_WiFi_PWR_OFF      (1u<<2) // Controller 2 port 0.
#define SDMMCCTL_UNK_PWR_OFF       (1u<<3) // Controller 3 port 1 power off? Set at cold boot.
#define SDMMCCTL_UNK_BIT6          (1u<<6) // Wifi port related? Pull up? Set at cold boot.
#define SDMMCCTL_TOSHSD3_MAP9      (0u)    // Controller 3 mapping ARM9 0x10007000.
#define SDMMCCTL_TOSHSD3_MAP11     (1u<<8) // Controller 3 mapping ARM11 0x10100000.
#define SDMMCCTL_CARD_TOSHSD3_SEL  (0u)    // SD card slot controller select TOSHSD3 0x10007000/0x10100000.
#define SDMMCCTL_CARD_TOSHSD1_SEL  (1u<<9) // SD card slot controller select TOSHSD1 0x10006000.

// REG_CFG9_EXTMEMCNT9
#define EXTMEMCNT9_WRAM_EXT_E      (1u)    // Enables extra WRAM aka. ARM9 mem extension.

// REG_CFG9_SOCINFO
#define SOCINFO_CTR                (1u)    // Also set on New 3DS.
#define SOCINFO_LGR1               (1u<<1) // Never saw the daylight? Set on retail N3DS (LGR2).
#define SOCINFO_LGR2               (1u<<2) // Set on New 3DS.

// REG_CFG9_BOOTENV
#define BOOTENV_COLD_BOOT          (0u)
#define BOOTENV_NATIVE_FIRM        (1u)
#define BOOTENV_TWL_FIRM           (3u)
#define BOOTENV_AGB_FIRM           (7u)

// REG_CFG9_UNITINFO
// TODO: Collect all the possible values for this reg.
#define UNITINFO_RETAIL            (0u)
