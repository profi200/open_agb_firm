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


#define PDN_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x41000)

typedef struct
{
	vu16 cnt;            // 0x000
	u8 _0x2[6];
	vu32 wake_enable;    // 0x008
	vu32 wake_reason;    // 0x00C Write 1 to acknowledge and 0 to clear?
	// Some LGY regs are located inbetween. See lgy.h/c.
	u8 _0x10[0x1f0];
	vu32 gpu_cnt;        // 0x200
	vu8  vram_cnt;       // 0x204 This reg doesn't seem to exist on retail hardware.
	u8 _0x205[3];
	vu8  lcd_cnt;        // 0x208 This reg doesn't seem to exist on retail hardware.
	u8 _0x209[7];
	vu8  fcram_cnt;      // 0x210
	u8 _0x211[0xf];
	vu8  i2s_cnt;        // 0x220
	u8 _0x221[3];
	vu8  cam_cnt;        // 0x224
	u8 _0x225[0xb];
	vu8  dsp_cnt;        // 0x230
	u8 _0x231[0xF];
	vu8  g1_cnt;         // 0x240 Hantro G1 decoder aka MVD.
	u8 _0x241[0xbf];
	vu16 lgr_socmode;    // 0x300
	u8 _0x302[2];
	vu16 lgr_cnt;        // 0x304 Is this reg actually only vu8?
	u8 _0x306[0xa];
	vu8  lgr_cpu_cnt[4]; // 0x310
} Pdn;
static_assert(offsetof(Pdn, lgr_cpu_cnt[3]) == 0x313, "Error: Member lgr_cpu_cnt[3] of Pdn is not at offset 0x313!");

ALWAYS_INLINE Pdn* getPdnRegs(void)
{
	return (Pdn*)PDN_REGS_BASE;
}


// REG_PDN_CNT
#define PDN_CNT_SLEEP                   (1u)     // Set this bit to enter sleep mode.
#define PDN_CNT_VRAM_OFF                (1u<<15) // Set when VRAM is powered off.

// REG_PDN_WAKE_ENABLE and REG_PDN_WAKE_REASON
#define PDN_WAKE_PADCNT                 (1u)
#define PDN_WAKE_SHELL_OPENED           (1u<<3)
#define PDN_WAKE_HEADPH_NOT_PLUGGED_IN  (1u<<4) // Really?
#define PDN_WAKE_UNK6                   (1u<<6) // DSi mode related.
#define PDN_WAKE_SDIO1                  (1u<<7)
#define PDN_WAKE_SDIO2                  (1u<<8)
#define PDN_WAKE_SDIO3                  (1u<<16)
// 17-28 maybe GPIO3 0-11?
#define PDN_WAKE_GAMECARD_INSERT        (1u<<29) // ?
#define PDN_WAKE_TOUCHPEN_DOWN          (1u<<30)
#define PDN_WAKE_UNK31                  (1u<<31) // Also shell related?

// REG_PDN_GPU_CNT
// Note: The resets are active low.
#define PDN_GPU_CNT_NORST_REGS          (1u)    // And more?
#define PDN_GPU_CNT_NORST_PSC           (1u<<1) // ?
#define PDN_GPU_CNT_NORST_GEOSHADER     (1u<<2) // ?
#define PDN_GPU_CNT_NORST_RASTERIZER    (1u<<3) // ?
#define PDN_GPU_CNT_NORST_PPF           (1u<<4)
#define PDN_GPU_CNT_NORST_PDC           (1u<<5) // ?
#define PDN_GPU_CNT_NORST_PDC2          (1u<<6) // Maybe pixel pipeline or so?
#define PDN_GPU_CNT_NORST_ALL           ((PDN_GPU_CNT_NORST_PDC2<<1) - 1)
#define PDN_GPU_CNT_CLK_EN              (1u<<16)

// REG_PDN_VRAM_CNT
#define PDN_VRAM_CNT_CLK_EN             (1u)

// REG_PDN_LCD_CNT
#define PDN_LCD_CNT_PWR_MGR_OFF         (1u) // Power management off?

// REG_PDN_FCRAM_CNT
// Note: Reset is active low.
#define PDN_FCRAM_CNT_NORST             (1u)
#define PDN_FCRAM_CNT_CLK_EN            (1u<<1)
#define PDN_FCRAM_CNT_CLK_EN_ACK        (1u<<2) // Gets set or unset depending on CLK_E.

// REG_PDN_I2S_CNT
#define PDN_I2S_CNT_I2S_CLK1_EN         (1u)    // ? Unused?
#define PDN_I2S_CNT_I2S_CLK2_EN         (1u<<1)

// REG_PDN_CAM_CNT
#define PDN_CAM_CNT_CLK_EN              (1u)

// REG_PDN_DSP_CNT
// Note: Reset is active low.
#define PDN_DSP_CNT_NORST               (1u)
#define PDN_DSP_CNT_CLK_EN              (1u<<1)

// REG_PDN_G1_CNT
// TODO: Active low or high?
#define PDN_G1_CNT_NORST                (1u)

// REG_PDN_LGR_SOCMODE
typedef enum
{
	SOCMODE_CTR_268MHZ  = 0u,
	SOCMODE_LGR2_268MHZ = 1u, // Also enables FCRAM extension.
	SOCMODE_LGR1_268MHZ = 2u, // Also enables FCRAM extension?
	SOCMODE_LGR1_536MHZ = 3u, // Also enables FCRAM extension?
	SOCMODE_LGR2_804MHZ = 5u, // Also enables FCRAM extension.

	SOCMODE_MASK        = 7u
} PdnSocmode;

#define PDN_LGR_SOCMODE_ACK             (1u<<15)

// REG_PDN_LGR_CNT
#define PDN_LGR_CNT_WRAM_EXT_EN         (1u)    // QTM WRAM enable.
#define PDN_LGR_CNT_L2C_EN              (1u<<8) // L2C L2 cache enable.

// REGs_PDN_LGR_CPU_CNT
// Note: Reset is active low.
#define LGR_CPU_CNT_NORST               (1u)    // Core 2/3 only. Reset and instruction overlay enable.
#define LGR_CPU_CNT_D_OVERL_EN          (1u<<1) // Core 2/3 only. Data overlay enable. Also used to signal a core booted.
#define LGR_CPU_CNT_RST_STAT            (1u<<4) // Reset status.
#define LGR_CPU_CNT_UNK                 (1u<<5) // Something ready?



void PDN_core123Init(void);
void PDN_setSocmode(PdnSocmode socmode);
void PDN_poweroffCore23(void);
