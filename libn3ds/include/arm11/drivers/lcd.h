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
#include "arm11/drivers/gx.h"



// LCD/ABL regs.
#define LCD_REGS_BASE            (IO_MEM_ARM11_ONLY + 0x2000)
#define REG_LCD_PARALLAX_CNT     *((vu32*)(LCD_REGS_BASE + 0x000)) // Controls PWM for the parallax barrier?
#define REG_LCD_PARALLAX_PWM     *((vu32*)(LCD_REGS_BASE + 0x004)) // Frequency/other PWM stuff maybe?
#define REG_LCD_UNK00C           *((vu32*)(LCD_REGS_BASE + 0x00C)) // Wtf is "FIX"?
#define REG_LCD_RST              *((vu32*)(LCD_REGS_BASE + 0x014)) // Reset active low.

#define REG_LCD_ABL0_CNT         *((vu32*)(LCD_REGS_BASE + 0x200)) // Bit 0 enables ABL aka power saving mode.
#define REG_LCD_ABL0_FILL        *((vu32*)(LCD_REGS_BASE + 0x204))
#define REG_LCD_ABL0_LIGHT       *((vu32*)(LCD_REGS_BASE + 0x240))
#define REG_LCD_ABL0_LIGHT_PWM   *((vu32*)(LCD_REGS_BASE + 0x244))

#define REG_LCD_ABL1_CNT         *((vu32*)(LCD_REGS_BASE + 0xA00)) // Bit 0 enables ABL aka power saving mode.
#define REG_LCD_ABL1_FILL        *((vu32*)(LCD_REGS_BASE + 0xA04))
#define REG_LCD_ABL1_LIGHT       *((vu32*)(LCD_REGS_BASE + 0xA40))
#define REG_LCD_ABL1_LIGHT_PWM   *((vu32*)(LCD_REGS_BASE + 0xA44))


// Technically these regs belong in gx.h but they are used for LCD configuration so...
// Pitfall warning: The 3DS LCDs are physically rotated 90° CCW.

// PDC0 (top screen display controller) regs.
#define REG_LCD_PDC0_HTOTAL      *((vu32*)(GX_REGS_BASE + 0x400))
#define REG_LCD_PDC0_VTOTAL      *((vu32*)(GX_REGS_BASE + 0x424))
#define REG_LCD_PDC0_HPOS        *((const vu32*)(GX_REGS_BASE + 0x450))
#define REG_LCD_PDC0_VPOS        *((const vu32*)(GX_REGS_BASE + 0x454))
#define REG_LCD_PDC0_FB_A1       *((vu32*)(GX_REGS_BASE + 0x468))
#define REG_LCD_PDC0_FB_A2       *((vu32*)(GX_REGS_BASE + 0x46C))
#define REG_LCD_PDC0_FMT         *((vu32*)(GX_REGS_BASE + 0x470))
#define REG_LCD_PDC0_CNT         *((vu32*)(GX_REGS_BASE + 0x474))
#define REG_LCD_PDC0_SWAP        *((vu32*)(GX_REGS_BASE + 0x478))
#define REG_LCD_PDC0_STAT        *((const vu32*)(GX_REGS_BASE + 0x47C))
#define REG_LCD_PDC0_GTBL_IDX    *((vu32*)(GX_REGS_BASE + 0x480)) // Gamma table index.
#define REG_LCD_PDC0_GTBL_FIFO   *((vu32*)(GX_REGS_BASE + 0x484)) // Gamma table FIFO.
#define REG_LCD_PDC0_STRIDE      *((vu32*)(GX_REGS_BASE + 0x490))
#define REG_LCD_PDC0_FB_B1       *((vu32*)(GX_REGS_BASE + 0x494))
#define REG_LCD_PDC0_FB_B2       *((vu32*)(GX_REGS_BASE + 0x498))

// PDC1 (bottom screen display controller) regs.
#define REG_LCD_PDC1_HTOTAL      *((vu32*)(GX_REGS_BASE + 0x500))
#define REG_LCD_PDC1_VTOTAL      *((vu32*)(GX_REGS_BASE + 0x524))
#define REG_LCD_PDC1_HPOS        *((const vu32*)(GX_REGS_BASE + 0x550))
#define REG_LCD_PDC1_VPOS        *((const vu32*)(GX_REGS_BASE + 0x554))
#define REG_LCD_PDC1_FB_A1       *((vu32*)(GX_REGS_BASE + 0x568))
#define REG_LCD_PDC1_FB_A2       *((vu32*)(GX_REGS_BASE + 0x56C))
#define REG_LCD_PDC1_FMT         *((vu32*)(GX_REGS_BASE + 0x570))
#define REG_LCD_PDC1_CNT         *((vu32*)(GX_REGS_BASE + 0x574))
#define REG_LCD_PDC1_SWAP        *((vu32*)(GX_REGS_BASE + 0x578))
#define REG_LCD_PDC1_STAT        *((const vu32*)(GX_REGS_BASE + 0x57C))
#define REG_LCD_PDC1_GTBL_IDX    *((vu32*)(GX_REGS_BASE + 0x580)) // Gamma table index.
#define REG_LCD_PDC1_GTBL_FIFO   *((vu32*)(GX_REGS_BASE + 0x584)) // Gamma table FIFO.
#define REG_LCD_PDC1_STRIDE      *((vu32*)(GX_REGS_BASE + 0x590))
#define REG_LCD_PDC1_FB_B1       *((vu32*)(GX_REGS_BASE + 0x594))
#define REG_LCD_PDC1_FB_B2       *((vu32*)(GX_REGS_BASE + 0x598))


// REG_LCD_PDC_CNT
#define PDC_CNT_E           (1u)
#define PDC_CNT_I_MASK_H    (1u<<8)  // Disables H(Blank?) IRQs.
#define PDC_CNT_I_MASK_V    (1u<<9)  // Disables VBlank IRQs.
#define PDC_CNT_I_MASK_ERR  (1u<<10) // Disables error IRQs. What kind of errors?
#define PDC_CNT_I_MASK_ALL  (PDC_CNT_I_MASK_ERR | PDC_CNT_I_MASK_V | PDC_CNT_I_MASK_H)
#define PDC_CNT_OUT_E       (1u<<16) // Output enable?

// REG_LCD_PDC_SWAP
// Masks
#define PDC_SWAP_NEXT       (1u)     // Next framebuffer.
#define PDC_SWAP_CUR        (1u<<4)  // Currently displaying framebuffer?
// Bits
#define PDC_SWAP_RST_FIFO   (1u<<8)  // Which FIFO?
#define PDC_SWAP_I_H        (1u<<16) // H(Blank?) IRQ bit.
#define PDC_SWAP_I_V        (1u<<17) // VBlank IRQ bit.
#define PDC_SWAP_I_ERR      (1u<<18) // Error IRQ bit?
#define PDC_SWAP_I_ALL      (PDC_SWAP_I_ERR | PDC_SWAP_I_V | PDC_SWAP_I_H)


// LCD I2C regs.
typedef enum
{
	LCD_I2C_REG_POWER      = 0x01u,
	LCD_I2C_REG_UNK11      = 0x11u,
	LCD_I2C_REG_READ_ADDR  = 0x40u,
	LCD_I2C_REG_HS_SERIAL  = 0x50u, // Highspeed serial for upper LCD only.
	LCD_I2C_REG_UNK54      = 0x54u, // Checksum on/off?
	LCD_I2C_REG_UNK55      = 0x55u, // Checksum status?
	LCD_I2C_REG_STATUS     = 0x60u, // Initially 0x01.
	LCD_I2C_REG_BL_STATUS  = 0x62u, // Backlight status.
	LCD_I2C_REG_RST_STATUS = 0xFEu, // Reset status. Initially 0x00.
	LCD_I2C_REG_REVISION   = 0xFFu, // Revision/vendor infos.
} LcdI2cReg;

// LCD_I2C_REG_POWER
#define LCD_REG_POWER_BLACK      (0x11u) // Force blackscreen.
#define LCD_REG_POWER_ON         (0x10u) // Normal operation.
#define LCD_REG_POWER_OFF        (0x00u) // LCD powered off.

// LCD_I2C_REG_UNK11
#define LCD_REG_UNK11_UNK10      (0x10u) // Written on init.

// LCD_I2C_REG_HS_SERIAL
#define LCD_REG_HS_SERIAL_ON     (0x01u) // Enable highspeed serial.

// LCD_I2C_REG_UNK54

// LCD_I2C_REG_UNK55

// LCD_I2C_REG_STATUS
#define LCD_REG_STATUS_OK        (0x00u)
#define LCD_REG_STATUS_ERR       (0x01u)

// LCD_I2C_REG_BL_STATUS
#define LCD_REG_BL_STATUS_OFF    (0x00u)
#define LCD_REG_BL_STATUS_ON     (0x01u)

// LCD_I2C_REG_RST_STATUS
#define LCD_REG_RST_STATUS_NONE  (0xAAu)
#define LCD_REG_RST_STATUS_RST   (0x00u)



u8 LCDI2C_readReg(u8 lcd, LcdI2cReg reg);
void LCDI2C_writeReg(u8 lcd, LcdI2cReg reg, u8 data);
void LCDI2C_init(void);
void LCDI2C_waitBacklightsOn(void);
u16 LCDI2C_getRevisions(void);
