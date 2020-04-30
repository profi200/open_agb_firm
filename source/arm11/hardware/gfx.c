/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
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

#include <string.h>
#include <stdatomic.h>
#include "types.h"
#include "mem_map.h"
#include "hardware/gfx.h"
#include "mmio.h"
#include "arm11/hardware/i2c.h"
#include "arm11/hardware/mcu.h"
#include "arm11/debug.h"
#include "arm11/hardware/interrupt.h"
#include "arm11/hardware/timer.h"
#include "arm.h"
#include "util.h"
#include "arm11/allocator/vram.h"


#define PDN_REGS_BASE            (IO_MEM_ARM9_ARM11 + 0x40000)
#define REG_PDN_GPU_CNT          *((vu32*)(PDN_REGS_BASE + 0x1200))

// LCD/ABL regs.
#define LCD_REGS_BASE            (IO_MEM_ARM11_ONLY + 0x2000)
#define REG_LCD_ABL0_CNT         *((vu32*)(LCD_REGS_BASE + 0x200)) // Bit 0 enables ABL aka power saving mode.
#define REG_LCD_ABL0_FILL        *((vu32*)(LCD_REGS_BASE + 0x204))
#define REG_LCD_ABL0_LIGHT       *((vu32*)(LCD_REGS_BASE + 0x240))
#define REG_LCD_ABL0_LIGHT_PWM   *((vu32*)(LCD_REGS_BASE + 0x244))

#define REG_LCD_ABL1_CNT         *((vu32*)(LCD_REGS_BASE + 0xA00)) // Bit 0 enables ABL aka power saving mode.
#define REG_LCD_ABL1_FILL        *((vu32*)(LCD_REGS_BASE + 0xA04))
#define REG_LCD_ABL1_LIGHT       *((vu32*)(LCD_REGS_BASE + 0xA40))
#define REG_LCD_ABL1_LIGHT_PWM   *((vu32*)(LCD_REGS_BASE + 0xA44))

#define GX_REGS_BASE             (IO_MEM_ARM11_ONLY + 0x200000)
#define REG_GX_GPU_CLK           *((vu32*)(GX_REGS_BASE + 0x0004)) // ?

// PSC (memory fill) regs.
#define REG_GX_PSC_FILL0_S_ADR   *((vu32*)(GX_REGS_BASE + 0x0010)) // Start address
#define REG_GX_PSC_FILL0_E_ADR   *((vu32*)(GX_REGS_BASE + 0x0014)) // End address
#define REG_GX_PSC_FILL0_VAL     *((vu32*)(GX_REGS_BASE + 0x0018)) // Fill value
#define REG_GX_PSC_FILL0_CNT     *((vu32*)(GX_REGS_BASE + 0x001C))

#define REG_GX_PSC_FILL1_S_ADR   *((vu32*)(GX_REGS_BASE + 0x0020))
#define REG_GX_PSC_FILL1_E_ADR   *((vu32*)(GX_REGS_BASE + 0x0024))
#define REG_GX_PSC_FILL1_VAL     *((vu32*)(GX_REGS_BASE + 0x0028))
#define REG_GX_PSC_FILL1_CNT     *((vu32*)(GX_REGS_BASE + 0x002C))

#define REG_GX_PSC_UNK           *((vu32*)(GX_REGS_BASE + 0x0030)) // ? gsp mudule only changes bit 8-11.
#define REG_GX_PSC_STAT          *((vu32*)(GX_REGS_BASE + 0x0034))

// PDC0 (top screen display controller) regs.
#define REG_LCD_PDC0_G_TBL_IDX   *((vu32*)(GX_REGS_BASE + 0x0480)) // Gamma table index.
#define REG_LCD_PDC0_G_TBL_FIFO  *((vu32*)(GX_REGS_BASE + 0x0484)) // Gamma table FIFO.

// PDC1 (bottom screen display controller) regs.
#define REG_LCD_PDC1_G_TBL_IDX   *((vu32*)(GX_REGS_BASE + 0x0580)) // Gamma table index.
#define REG_LCD_PDC1_G_TBL_FIFO  *((vu32*)(GX_REGS_BASE + 0x0584)) // Gamma table FIFO.

// PPF (transfer engine) regs.
#define REG_GX_PPF_IN_ADR        *((vu32*)(GX_REGS_BASE + 0x0C00))
#define REG_GX_PPF_OUT_ADR       *((vu32*)(GX_REGS_BASE + 0x0C04))
#define REG_GX_PPF_DT_OUTDIM     *((vu32*)(GX_REGS_BASE + 0x0C08)) // Display transfer output dimensions.
#define REG_GX_PPF_DT_INDIM      *((vu32*)(GX_REGS_BASE + 0x0C0C)) // Display transfer input dimensions.
#define REG_GX_PPF_CFG           *((vu32*)(GX_REGS_BASE + 0x0C10))
#define REG_GX_PPF_UNK14         *((vu32*)(GX_REGS_BASE + 0x0C14)) // Transfer interval?
#define REG_GX_PPF_CNT           *((vu32*)(GX_REGS_BASE + 0x0C18))
#define REG_GX_PPF_UNK1C         *((vu32*)(GX_REGS_BASE + 0x0C1C)) // ?
#define REG_GX_PPF_LEN           *((vu32*)(GX_REGS_BASE + 0x0C20)) // Texture copy size in bytes.
#define REG_GX_PPF_TC_INDIM      *((vu32*)(GX_REGS_BASE + 0x0C24)) // Texture copy input width and gap in 16 byte units.
#define REG_GX_PPF_TC_OUTDIM     *((vu32*)(GX_REGS_BASE + 0x0C28)) // Texture copy output width and gap in 16 byte units.

// P3D (GPU) regs.
#define REG_GX_P3D_UNK           *((vu32*)(GX_REGS_BASE + 0x1000)) // GSP writes 0 before running a cmd list.
#define REG_GX_P3D_LIST_SIZE     *((vu32*)(GX_REGS_BASE + 0x18E0)) // cmd list length in 8 bytes units. Must be aligned to 16.
#define REG_GX_P3D_LIST_ADR      *((vu32*)(GX_REGS_BASE + 0x18E8)) // cmd list address / 8. Must be aligned to 16.
#define REG_GX_P3D_LIST_RUN      *((vu32*)(GX_REGS_BASE + 0x18F0)) // Start list processing by writing 1.


static struct
{
	u16 lcdIds;            // Bits 0-7 top screen, 8-15 bottom screen.
	bool lcdIdsRead;
	u8 lcdPower;           // 1 = on. Bit 4 top light, bit 2 bottom light, bit 0 LCDs.
	u8 lcdLights[2];       // LCD backlight brightness. Top, bottom.
	bool events[6];
	u32 swap;              // Currently active framebuffer.
	void *framebufs[2][4]; // For each screen A1, A2, B1, B2
	u32 strides[2];        // Top, bottom
	u32 formats[2];        // Top, bottom
} g_gfxState = {0};



static u32 fmt2PixSize(GfxFbFmt fmt);
static void setupFramebufs(GfxFbFmt fmtTop, GfxFbFmt fmtBot);
static void deallocFramebufs(void);
static void setupDislayController(u8 lcd);
static void resetLcdsMaybe(void);
static void waitLcdsReady(void);
static void gfxIrqHandler(u32 intSource);

void GFX_init(GfxFbFmt fmtTop, GfxFbFmt fmtBot)
{
	setupFramebufs(fmtTop, fmtBot);

	*((vu32*)0x10140140) = 0; // REG_CFG11_GPUPROT

	// Reset
	REG_PDN_GPU_CNT = 0x10000;
	wait(12);
	REG_PDN_GPU_CNT = 0x1007F;
	REG_GX_GPU_CLK = 0x100;
	REG_GX_PSC_UNK = 0;
	REG_GX_PSC_FILL0_CNT = 0;
	REG_GX_PSC_FILL1_CNT = 0;
	REG_GX_PPF_CNT = 0;

	// LCD framebuffer setup.
	setupDislayController(0);
	setupDislayController(1);
	*((vu32*)0x10400478) = 0x70100; // Framebuffer select 0.
	*((vu32*)0x10400578) = 0x70100; // Framebuffer select 0.
	*((vu32*)0x10400474) = 0x10501; // Start
	*((vu32*)0x10400574) = 0x10501; // Start

	// LCD reg setup.
	REG_LCD_ABL0_FILL = 1u<<24; // Force blackscreen
	REG_LCD_ABL1_FILL = 1u<<24; // Force blackscreen
	*((vu32*)0x10202000) = 0;
	*((vu32*)0x10202004) = 0xA390A39;
	*((vu32*)0x10202014) = 0;
	*((vu32*)0x1020200C) = 0x10001;

	// Register IRQ handlers.
	IRQ_registerHandler(IRQ_PSC0, 14, 0, true, gfxIrqHandler);
	IRQ_registerHandler(IRQ_PSC1, 14, 0, true, gfxIrqHandler);
	IRQ_registerHandler(IRQ_PDC0, 14, 0, true, gfxIrqHandler);
	//IRQ_registerHandler(IRQ_PDC1, 14, 0, true, gfxIrqHandler);
	IRQ_registerHandler(IRQ_PPF, 14, 0, true, gfxIrqHandler);
	IRQ_registerHandler(IRQ_P3D, 14, 0, true, gfxIrqHandler);

	// Clear entire VRAM.
	GX_memoryFill((u32*)VRAM_BANK0, 1u<<9, VRAM_SIZE / 2, 0,
	              (u32*)VRAM_BANK1, 1u<<9, VRAM_SIZE / 2, 0);

	// Backlight and other stuff.
	REG_LCD_ABL0_LIGHT = 0;
	REG_LCD_ABL0_CNT = 0;
	REG_LCD_ABL0_LIGHT_PWM = 0;
	REG_LCD_ABL1_LIGHT = 0;
	REG_LCD_ABL1_CNT = 0;
	REG_LCD_ABL1_LIGHT_PWM = 0;

	*((vu32*)0x10202014) = 1;
	*((vu32*)0x1020200C) = 0;
	TIMER_sleepMs(10);
	resetLcdsMaybe();
	MCU_controlLCDPower(2u); // Power on LCDs.
	if(MCU_waitEvents(0x3Fu<<24) != 2u<<24) panic();

	// The transfer engine is (sometimes) borked on screen init.
	// Doing a dummy texture copy fixes it.
	// TODO: Proper fix.
	//GX_textureCopy((u32*)RENDERBUF_TOP, 0, (u32*)RENDERBUF_BOT, 0, 16);

	waitLcdsReady();
	REG_LCD_ABL0_LIGHT_PWM = 0x1023E;
	REG_LCD_ABL1_LIGHT_PWM = 0x1023E;
	MCU_controlLCDPower(0x28u); // Power on backlights.
	if(MCU_waitEvents(0x3Fu<<24) != 0x28u<<24) panic();
	g_gfxState.lcdPower = 0x15; // All on.

	// Make sure the fills finished.
	GFX_waitForEvent(GFX_EVENT_PSC0, false);
	GFX_waitForEvent(GFX_EVENT_PSC1, false);
	REG_LCD_ABL0_FILL = 0;
	REG_LCD_ABL1_FILL = 0;
}

void GFX_deinit(void)
{
	const u8 power = g_gfxState.lcdPower;
	if(power & ~1u) // Poweroff backlights if on.
	{
		MCU_controlLCDPower(power & ~1u);
		if(MCU_waitEvents(0x3Fu<<24) != (power & ~1u)<<24) panic();
	}
	if(power & 1u) // Poweroff LCDs if on.
	{
		MCU_controlLCDPower(1u);
		if(MCU_waitEvents(0x3Fu<<24) != 1u<<24) panic();
	}
	GFX_setBrightness(0, 0);
	REG_LCD_ABL0_LIGHT_PWM = 0;
	REG_LCD_ABL1_LIGHT_PWM = 0;
	*((vu32*)0x1020200C) = 0x10001;
	*((vu32*)0x10202014) = 0;
	REG_GX_PSC_UNK = 0xF00;
	REG_GX_GPU_CLK = 0;
	REG_PDN_GPU_CNT = 0x10001;

	deallocFramebufs();

	IRQ_unregisterHandler(IRQ_PSC0);
	IRQ_unregisterHandler(IRQ_PSC1);
	IRQ_unregisterHandler(IRQ_PDC0);
	//IRQ_unregisterHandler(IRQ_PDC1);
	IRQ_unregisterHandler(IRQ_PPF);
	IRQ_unregisterHandler(IRQ_P3D);
}

void GFX_gpuInit(void)
{
	REG_GX_GPU_CLK = 0x70100;
	*((vu32*)0x10400050) = 0x22221200;
	*((vu32*)0x10400054) = 0xFF2;

	*((vu32*)0x10401000) = 0;
	*((vu32*)0x10401080) = 0x12345678;
	*((vu32*)0x104010C0) = 0xFFFFFFF0;
	*((vu32*)0x104010D0) = 1;

	// GPUREG_START_DRAW_FUNC0
	// This reg needs to be set to 1 (configuration)
	// before running the first cmd list.
	*((vu32*)0x10401914) = 1;
}

void GFX_setFramebufFmt(GfxFbFmt fmtTop, GfxFbFmt fmtBot)
{
	REG_LCD_ABL0_FILL = 1u<<24; // Force blackscreen
	REG_LCD_ABL1_FILL = 1u<<24; // Force blackscreen

	if(fmtTop < (g_gfxState.formats[0] & 7u) || fmtBot < (g_gfxState.formats[1] & 7u))
	{
		deallocFramebufs();
		setupFramebufs(fmtTop, fmtBot);
	}

	// Update PDC regs.
	*((vu32*)0x10400468) = (u32)g_gfxState.framebufs[0][0];
	*((vu32*)0x1040046C) = (u32)g_gfxState.framebufs[0][1];
	*((vu32*)0x10400494) = (u32)g_gfxState.framebufs[0][2];
	*((vu32*)0x10400498) = (u32)g_gfxState.framebufs[0][3];
	*((vu32*)0x10400490) = g_gfxState.strides[0];
	*((vu32*)0x10400470) = g_gfxState.formats[0];

	*((vu32*)0x10400568) = (u32)g_gfxState.framebufs[1][0];
	*((vu32*)0x1040056C) = (u32)g_gfxState.framebufs[1][1];
	*((vu32*)0x10400594) = (u32)g_gfxState.framebufs[1][2];
	*((vu32*)0x10400598) = (u32)g_gfxState.framebufs[1][3];
	*((vu32*)0x10400590) = g_gfxState.strides[1];
	*((vu32*)0x10400570) = g_gfxState.formats[1];

	REG_LCD_ABL0_FILL = 0;
	REG_LCD_ABL1_FILL = 0;
}

static u32 fmt2PixSize(GfxFbFmt fmt)
{
	u32 size;

	switch(fmt)
	{
		case GFX_RGBA8:
			size = 4;
			break;
		case GFX_BGR8:
			size = 3;
			break;
		default: // 2 = RGB565, 3 = RGB5A1, 4 = RGBA4
			size = 2;
	}

	return size;
}

static void setupFramebufs(GfxFbFmt fmtTop, GfxFbFmt fmtBot)
{
	const u32 topPixSize = fmt2PixSize(fmtTop);
	const u32 botPixSize = fmt2PixSize(fmtBot);
	g_gfxState.strides[0] = 240u * topPixSize; // No gap.
	g_gfxState.strides[1] = 240u * botPixSize; // No gap.

	const u32 topSize = 400 * 240 * topPixSize;
	const u32 botSize = 320 * 240 * botPixSize;
	g_gfxState.framebufs[0][0] = vramAlloc(topSize); // Top A1 (3D left eye)
	void *botPtr = vramAlloc(botSize);
	g_gfxState.framebufs[1][0] = botPtr;             // Bottom A1
	g_gfxState.framebufs[1][2] = botPtr;             // Bottom B1 (unused)
	g_gfxState.framebufs[0][2] = vramAlloc(topSize); // Top B1 (3D right eye)

	g_gfxState.framebufs[0][1] = vramAlloc(topSize); // Top A2 (3D left eye)
	botPtr = vramAlloc(botSize);
	g_gfxState.framebufs[1][1] = botPtr;             // Bottom A2
	g_gfxState.framebufs[1][3] = botPtr;             // Bottom B2 (unused)
	g_gfxState.framebufs[0][3] = vramAlloc(topSize); // Top B2 (3D right eye)

	g_gfxState.formats[0] = 0<<16 | 3<<8 | 1<<6 | 0<<4 | fmtTop;
	g_gfxState.formats[1] = 0<<16 | 3<<8 | 0<<6 | 0<<4 | fmtBot;
}

static void deallocFramebufs(void)
{
	vramFree(g_gfxState.framebufs[0][3]);
	vramFree(g_gfxState.framebufs[1][1]);
	vramFree(g_gfxState.framebufs[0][1]);
	vramFree(g_gfxState.framebufs[0][2]);
	vramFree(g_gfxState.framebufs[1][0]);
	vramFree(g_gfxState.framebufs[0][0]);
}

static void setupDislayController(u8 lcd)
{
	if(lcd > 1) return;

	static const u32 displayCfgs[2][24] =
	{
		{
			// PDC0 regs 0-0x4C.
			450, 209, 449, 449, 0, 207, 209, 453<<16 | 449,
			1<<16 | 0, 413, 2, 402, 402, 402, 1, 2,
			406<<16 | 402, 0, 0<<4 | 0, 0<<16 | 0xFF<<8 | 0,
			// PDC0 regs 0x5C-0x64.
			400<<16 | 240, // Width and height.
			449<<16 | 209,
			402<<16 | 2,
			// PDC0 reg 0x9C.
			0<<16 | 0
		},
		{
			// PDC1 regs 0-0x4C.
			450, 209, 449, 449, 205, 207, 209, 453<<16 | 449,
			1<<16 | 0, 413, 82, 402, 402, 79, 80, 82,
			408<<16 | 404, 0, 1<<4 | 1, 0<<16 | 0<<8 | 0xFF,
			// PDC1 regs 0x5C-0x64.
			320<<16 | 240, // Width and height.
			449<<16 | 209,
			402<<16 | 82,
			// PDC1 reg 0x9C.
			0<<16 | 0
		}
	};

	const u32 *const cfg = displayCfgs[lcd];
	vu32 *const regs = (vu32*)(GX_REGS_BASE + 0x400 + (0x100u * lcd));

	iomemcpy(regs, cfg, 0x50);          // PDC regs 0-0x4C.
	iomemcpy(regs + 23, &cfg[20], 0xC); // PDC regs 0x5C-0x64.
	regs[36] = g_gfxState.strides[lcd]; // PDC reg 0x90 stride.
	regs[39] = cfg[23];                 // PDC reg 0x9C.


	// PDC regs 0x68, 0x6C, 0x94, 0x98 and 0x70.
	regs[26] = (u32)g_gfxState.framebufs[lcd][0]; // Framebuffer A first address.
	regs[27] = (u32)g_gfxState.framebufs[lcd][1]; // Framebuffer A second address.
	regs[37] = (u32)g_gfxState.framebufs[lcd][2]; // Framebuffer B first address.
	regs[38] = (u32)g_gfxState.framebufs[lcd][3]; // Framebuffer B second address.
	regs[28] = g_gfxState.formats[lcd];           // Format


	regs[32] = 0; // Gamma table index 0.
	for(u32 i = 0; i < 256; i++) regs[33] = 0x10101u * i;
}

static u16 getLcdIds(void)
{
	u16 ids;

	if(!g_gfxState.lcdIdsRead)
	{
		g_gfxState.lcdIdsRead = true;

		u16 top, bot;
		I2C_writeReg(I2C_DEV_LCD0, 0x40, 0xFF);
		I2C_readRegBuf(I2C_DEV_LCD0, 0x40, (u8*)&top, 2);
		I2C_writeReg(I2C_DEV_LCD1, 0x40, 0xFF);
		I2C_readRegBuf(I2C_DEV_LCD1, 0x40, (u8*)&bot, 2);

		ids = top>>8;
		ids |= bot & 0xFF00u;
		g_gfxState.lcdIds = ids;
	}
	else ids = g_gfxState.lcdIds;

	return ids;
}

static void resetLcdsMaybe(void)
{
	const u16 ids = getLcdIds();

	// Top screen
	if(ids & 0xFFu) I2C_writeReg(I2C_DEV_LCD0, 0xFE, 0xAA);
	else
	{
		I2C_writeReg(I2C_DEV_LCD0, 0x11, 0x10);
		I2C_writeReg(I2C_DEV_LCD0, 0x50, 1);
	}

	// Bottom screen
	if(ids>>8) I2C_writeReg(I2C_DEV_LCD1, 0xFE, 0xAA);
	else       I2C_writeReg(I2C_DEV_LCD1, 0x11, 0x10);

	I2C_writeReg(I2C_DEV_LCD0, 0x60, 0);
	I2C_writeReg(I2C_DEV_LCD1, 0x60, 0);
	I2C_writeReg(I2C_DEV_LCD0, 1, 0x10);
	I2C_writeReg(I2C_DEV_LCD1, 1, 0x10);
}

static void waitLcdsReady(void)
{
	const u16 ids = getLcdIds();

	if((ids & 0xFFu) == 0 || (ids>>8) == 0) // Unknown LCD?
	{
		TIMER_sleepMs(150);
	}
	else
	{
		u32 i = 0;
		do
		{
			u16 top, bot;
			I2C_writeReg(I2C_DEV_LCD0, 0x40, 0x62);
			I2C_readRegBuf(I2C_DEV_LCD0, 0x40, (u8*)&top, 2);
			I2C_writeReg(I2C_DEV_LCD1, 0x40, 0x62);
			I2C_readRegBuf(I2C_DEV_LCD1, 0x40, (u8*)&bot, 2);

			if((top>>8) == 1 && (bot>>8) == 1) break;

			TIMER_sleepTicks(TIMER_FREQ(1, 1000) * 33.333f);
			i++;
		} while(i < 10);
	}
}

void GFX_setBrightness(u8 top, u8 bot)
{
	if(top > 64 || bot > 64) return;

	g_gfxState.lcdLights[0] = top;
	g_gfxState.lcdLights[1] = bot;
	REG_LCD_ABL0_LIGHT = top;
	REG_LCD_ABL1_LIGHT = bot;
}

void GFX_setForceBlack(bool top, bool bot)
{
	REG_LCD_ABL0_FILL = top<<24; // Force blackscreen
	REG_LCD_ABL1_FILL = bot<<24; // Force blackscreen
}

void* GFX_getFramebuffer(u8 screen)
{
	return g_gfxState.framebufs[screen][g_gfxState.swap ^ 1u];
}

void GFX_swapFramebufs(void)
{
	u32 swap = g_gfxState.swap;
	swap ^= 1u;
	g_gfxState.swap = swap;

	swap |= 0x70000u; // Acknowledge IRQs.
	*((vu32*)0x10400478) = swap;
	*((vu32*)0x10400578) = swap;
}

void GFX_waitForEvent(GfxEvent event, bool discard)
{
	bool *const events = g_gfxState.events;

	if(discard) atomic_store_explicit(&events[event], false, memory_order_relaxed);
	while(!atomic_load_explicit(&events[event], memory_order_relaxed)) __wfe();
	atomic_store_explicit(&events[event], false, memory_order_relaxed);
}

static void gfxIrqHandler(u32 intSource)
{
	bool *const events = g_gfxState.events;

	atomic_store_explicit(&events[intSource - IRQ_PSC0], true, memory_order_relaxed);
}

void GX_memoryFill(u32 *buf0a, u32 buf0v, u32 buf0Sz, u32 val0, u32 *buf1a, u32 buf1v, u32 buf1Sz, u32 val1)
{
	if(buf0a)
	{
		REG_GX_PSC_FILL0_S_ADR = (u32)buf0a>>3;
		REG_GX_PSC_FILL0_E_ADR = ((u32)buf0a + buf0Sz)>>3;
		REG_GX_PSC_FILL0_VAL   = val0;
		REG_GX_PSC_FILL0_CNT   = buf0v | 1u; // Pattern + start
	}

	if(buf1a)
	{
		REG_GX_PSC_FILL1_S_ADR = (u32)buf1a>>3;
		REG_GX_PSC_FILL1_E_ADR = ((u32)buf1a + buf1Sz)>>3;
		REG_GX_PSC_FILL1_VAL   = val1;
		REG_GX_PSC_FILL1_CNT   = buf1v | 1u; // Pattern + start
	}
}

// Example: GX_displayTransfer(in, 160u<<16 | 240u, out, 160u<<16 | 240u, 2u<<12 | 2u<<8);
// Copy and unswizzle GBA sized frame in RGB565.
void GX_displayTransfer(const u32 *const in, u32 indim, u32 *out, u32 outdim, u32 flags)
{
	if(!in || !out) return;

	REG_GX_PPF_IN_ADR = (u32)in>>3;
	REG_GX_PPF_OUT_ADR = (u32)out>>3;
	REG_GX_PPF_DT_INDIM = indim;
	REG_GX_PPF_DT_OUTDIM = outdim;
	REG_GX_PPF_CFG = flags;
	REG_GX_PPF_UNK14 = 0;
	REG_GX_PPF_CNT = 1;
}

// Example: GX_textureCopy(in, (240 * 2)<<12 | (240 * 2)>>4, out, (240 * 2)<<12 | (240 * 2)>>4, 240 * 400);
// Copies every second line of a 240x400 framebuffer.
void GX_textureCopy(const u32 *const in, u32 indim, u32 *out, u32 outdim, u32 size)
{
	if(!in || !out) return;

	REG_GX_PPF_IN_ADR = (u32)in>>3;
	REG_GX_PPF_OUT_ADR = (u32)out>>3;
	REG_GX_PPF_CFG = 1u<<3;
	REG_GX_PPF_LEN = size;
	REG_GX_PPF_TC_INDIM = indim;
	REG_GX_PPF_TC_OUTDIM = outdim;
	REG_GX_PPF_CNT = 1;
}

void GX_processCommandList(u32 size, const u32 *const cmdList)
{
	REG_GX_P3D_UNK = 0; // Acknowledge last P3D?
	while(REG_GX_PSC_STAT & 1u<<31) wait(0x30);

	REG_GX_P3D_LIST_SIZE = size>>3;
	REG_GX_P3D_LIST_ADR = (u32)cmdList>>3;
	REG_GX_P3D_LIST_RUN = 1;
}

// TODO: Sleep mode stuff needs some work.
/*void GFX_enterLowPowerState(void)
{
	REG_LCD_ABL0_FILL = 1u<<24; // Force blackscreen
	REG_LCD_ABL1_FILL = 1u<<24; // Force blackscreen
	GFX_waitForEvent(GFX_EVENT_PDC0, true);

	// Stop PDCs.
	*((vu32*)0x10400474) = 0x700; // Stop
	*((vu32*)0x10400574) = 0x700; // Stop
	*((vu32*)0x10400478) = 0x70100;
	*((vu32*)0x10400578) = 0x70100;

	REG_GX_PSC_UNK = 0xF00;
	REG_PDN_GPU_CNT = 0x7F;
}

void GFX_returnFromLowPowerState(void)
{
	REG_PDN_GPU_CNT = 0x1007F;
	REG_GX_PSC_UNK = 0;
	//REG_GX_GPU_CLK = 0x70100;
	REG_GX_PSC_FILL0_CNT = 0;
	REG_GX_PSC_FILL1_CNT = 0;
	// *((vu32*)0x10400050) = 0x22221200;
	// *((vu32*)0x10400054) = 0xFF2;

	setupDislayController(0);
	setupDislayController(1);
	const u32 swap = 0x70100 | g_gfxState.swap;
	*((vu32*)0x10400478) = swap;
	*((vu32*)0x10400578) = swap;
	*((vu32*)0x10400474) = 0x10501; // Start
	*((vu32*)0x10400574) = 0x10501; // Start

	REG_LCD_ABL0_FILL = 0;
	REG_LCD_ABL1_FILL = 0;
}*/
