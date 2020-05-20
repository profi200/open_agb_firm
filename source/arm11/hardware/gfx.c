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
#include "hardware/gfx.h"
#include "arm11/hardware/lcd.h"
#include "arm11/hardware/gx.h"
#include "arm11/hardware/gpu_regs.h"
#include "mem_map.h"
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


static struct
{
	u16 lcdIds;            // Bits 0-7 top screen, 8-15 bottom screen.
	bool lcdIdsRead;
	u8 lcdPower;           // 1 = on. Bit 4 top light, bit 2 bottom light, bit 0 LCDs.
	u8 lcdLights[2];       // LCD backlight brightness. Top, bottom.
	bool events[6];
	u32 swap;              // Currently active framebuffer.
	void *framebufs[2][4]; // For each screen A1, A2, B1, B2
	u16 strides[2];        // Top, bottom
	u32 formats[2];        // Top, bottom
} g_gfxState = {0};



static u8 fmt2PixSize(GfxFbFmt fmt);
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
	REG_GX_PSC_VRAM = 0;
	REG_GX_PSC_FILL0_CNT = 0;
	REG_GX_PSC_FILL1_CNT = 0;
	REG_GX_PPF_CNT = 0;

	// LCD framebuffer setup.
	setupDislayController(0);
	setupDislayController(1);
	REG_LCD_PDC0_SWAP = 0; // Select framebuf 0.
	REG_LCD_PDC1_SWAP = 0;
	REG_LCD_PDC0_CNT = PDC_CNT_OUT_E | PDC_CNT_I_MASK_ERR | PDC_CNT_I_MASK_H | PDC_CNT_E; // Start
	REG_LCD_PDC1_CNT = PDC_CNT_OUT_E | PDC_CNT_I_MASK_ERR | PDC_CNT_I_MASK_H | PDC_CNT_E;

	// LCD reg setup.
	REG_LCD_ABL0_FILL = 1u<<24; // Force blackscreen
	REG_LCD_ABL1_FILL = 1u<<24; // Force blackscreen
	REG_LCD_PARALLAX_CNT = 0;
	REG_LCD_PARALLAX_PWM = 0xA390A39;
	REG_LCD_RST = 0;
	REG_LCD_UNK00C = 0x10001;

	// Register IRQ handlers.
	IRQ_registerIsr(IRQ_PSC0, 14, 0, true, gfxIrqHandler);
	IRQ_registerIsr(IRQ_PSC1, 14, 0, true, gfxIrqHandler);
	IRQ_registerIsr(IRQ_PDC0, 14, 0, true, gfxIrqHandler);
	//IRQ_registerIsr(IRQ_PDC1, 14, 0, true, gfxIrqHandler);
	IRQ_registerIsr(IRQ_PPF, 14, 0, true, gfxIrqHandler);
	IRQ_registerIsr(IRQ_P3D, 14, 0, true, gfxIrqHandler);

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

	REG_LCD_RST = 1;
	REG_LCD_UNK00C = 0;
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

	// GPU stuff.
	REG_GX_GPU_CLK = 0x70100;
	*((vu32*)0x10400050) = 0x22221200;
	*((vu32*)0x10400054) = 0xFF2;

	REG_GX_P3D(GPUREG_IRQ_ACK) = 0;
	REG_GX_P3D(GPUREG_IRQ_CMP) = 0x12345678;
	REG_GX_P3D(GPUREG_IRQ_MASK) = 0xFFFFFFF0;
	REG_GX_P3D(GPUREG_IRQ_AUTOSTOP) = 1;

	// This reg needs to be set to 1 (configuration)
	// before running the first cmd list.
	REG_GX_P3D(GPUREG_START_DRAW_FUNC0) = 1;
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
	REG_LCD_UNK00C = 0x10001;
	REG_LCD_RST = 0;
	REG_GX_PSC_VRAM = 0xF00;
	REG_GX_GPU_CLK = 0;
	REG_PDN_GPU_CNT = 0x10001;

	deallocFramebufs();

	IRQ_unregisterIsr(IRQ_PSC0);
	IRQ_unregisterIsr(IRQ_PSC1);
	IRQ_unregisterIsr(IRQ_PDC0);
	//IRQ_unregisterIsr(IRQ_PDC1);
	IRQ_unregisterIsr(IRQ_PPF);
	IRQ_unregisterIsr(IRQ_P3D);
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
	REG_LCD_PDC0_FB_A1  = (u32)g_gfxState.framebufs[0][0];
	REG_LCD_PDC0_FB_A2  = (u32)g_gfxState.framebufs[0][1];
	REG_LCD_PDC0_FB_B1  = (u32)g_gfxState.framebufs[0][2];
	REG_LCD_PDC0_FB_B2  = (u32)g_gfxState.framebufs[0][3];
	REG_LCD_PDC0_STRIDE = g_gfxState.strides[0];
	REG_LCD_PDC0_FMT    = g_gfxState.formats[0];

	REG_LCD_PDC1_FB_A1  = (u32)g_gfxState.framebufs[1][0];
	REG_LCD_PDC1_FB_A2  = (u32)g_gfxState.framebufs[1][1];
	REG_LCD_PDC1_FB_B1  = (u32)g_gfxState.framebufs[1][2];
	REG_LCD_PDC1_FB_B2  = (u32)g_gfxState.framebufs[1][3];
	REG_LCD_PDC1_STRIDE = g_gfxState.strides[1];
	REG_LCD_PDC1_FMT    = g_gfxState.formats[1];

	REG_LCD_ABL0_FILL = 0;
	REG_LCD_ABL1_FILL = 0;
}

static u8 fmt2PixSize(GfxFbFmt fmt)
{
	u8 size;

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
	const u8 topPixSize = fmt2PixSize(fmtTop);
	const u8 botPixSize = fmt2PixSize(fmtBot);
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

	g_gfxState.formats[0] = 0u<<16 | 3u<<8 | 1u<<6 | 0u<<4 | fmtTop;
	g_gfxState.formats[1] = 0u<<16 | 3u<<8 | 0u<<6 | 0u<<4 | fmtBot;
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

	swap |= PDC_SWAP_I_ALL; // Acknowledge IRQs.
	REG_LCD_PDC0_SWAP = swap;
	REG_LCD_PDC1_SWAP = swap;
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
		REG_GX_PSC_FILL0_S_ADDR = (u32)buf0a>>3;
		REG_GX_PSC_FILL0_E_ADDR = ((u32)buf0a + buf0Sz)>>3;
		REG_GX_PSC_FILL0_VAL    = val0;
		REG_GX_PSC_FILL0_CNT    = buf0v | 1u; // Pattern + start
	}

	if(buf1a)
	{
		REG_GX_PSC_FILL1_S_ADDR = (u32)buf1a>>3;
		REG_GX_PSC_FILL1_E_ADDR = ((u32)buf1a + buf1Sz)>>3;
		REG_GX_PSC_FILL1_VAL    = val1;
		REG_GX_PSC_FILL1_CNT    = buf1v | 1u; // Pattern + start
	}
}

// Example: GX_displayTransfer(in, 160u<<16 | 240u, out, 160u<<16 | 240u, 2u<<12 | 2u<<8);
// Copy and unswizzle GBA sized frame in RGB565.
void GX_displayTransfer(const u32 *const in, u32 indim, u32 *out, u32 outdim, u32 flags)
{
	if(!in || !out) return;

	REG_GX_PPF_IN_ADDR = (u32)in>>3;
	REG_GX_PPF_OUT_ADDR = (u32)out>>3;
	REG_GX_PPF_DT_INDIM = indim;
	REG_GX_PPF_DT_OUTDIM = outdim;
	REG_GX_PPF_FlAGS = flags;
	REG_GX_PPF_UNK14 = 0;
	REG_GX_PPF_CNT = 1;
}

// Example: GX_textureCopy(in, (240 * 2)<<12 | (240 * 2)>>4, out, (240 * 2)<<12 | (240 * 2)>>4, 240 * 400);
// Copies every second line of a 240x400 framebuffer.
void GX_textureCopy(const u32 *const in, u32 indim, u32 *out, u32 outdim, u32 size)
{
	if(!in || !out) return;

	REG_GX_PPF_IN_ADDR = (u32)in>>3;
	REG_GX_PPF_OUT_ADDR = (u32)out>>3;
	REG_GX_PPF_FlAGS = 1u<<3;
	REG_GX_PPF_LEN = size;
	REG_GX_PPF_TC_INDIM = indim;
	REG_GX_PPF_TC_OUTDIM = outdim;
	REG_GX_PPF_CNT = 1;
}

void GX_processCommandList(u32 size, const u32 *const cmdList)
{
	REG_GX_P3D(GPUREG_IRQ_ACK) = 0; // Acknowledge last P3D.
	while(REG_GX_PSC_STAT & 1u<<31) wait(0x30);

	REG_GX_P3D(GPUREG_CMDBUF_SIZE0) = size>>3;
	REG_GX_P3D(GPUREG_CMDBUF_ADDR0) = (u32)cmdList>>3;
	REG_GX_P3D(GPUREG_CMDBUF_JUMP0) = 1;
}

// TODO: Sleep mode stuff needs some work.
/*void GFX_enterLowPowerState(void)
{
	REG_LCD_ABL0_FILL = 1u<<24; // Force blackscreen
	REG_LCD_ABL1_FILL = 1u<<24; // Force blackscreen
	GFX_waitForEvent(GFX_EVENT_PDC0, true);

	// Stop PDCs.
	REG_LCD_PDC0_CNT = 0x700; // Stop
	REG_LCD_PDC1_CNT = 0x700; // Stop
	REG_LCD_PDC0_SWAP = 0x70100;
	REG_LCD_PDC1_SWAP = 0x70100;

	REG_GX_PSC_VRAM = 0xF00;
	REG_PDN_GPU_CNT = 0x7F;
}

void GFX_returnFromLowPowerState(void)
{
	REG_PDN_GPU_CNT = 0x1007F;
	REG_GX_PSC_VRAM = 0;
	//REG_GX_GPU_CLK = 0x70100;
	REG_GX_PSC_FILL0_CNT = 0;
	REG_GX_PSC_FILL1_CNT = 0;
	// *((vu32*)0x10400050) = 0x22221200;
	// *((vu32*)0x10400054) = 0xFF2;

	setupDislayController(0);
	setupDislayController(1);
	const u32 swap = 0x70100 | g_gfxState.swap;
	REG_LCD_PDC0_SWAP = swap;
	REG_LCD_PDC1_SWAP = swap;
	REG_LCD_PDC0_CNT = 0x10501; // Start
	REG_LCD_PDC1_CNT = 0x10501; // Start

	REG_LCD_ABL0_FILL = 0;
	REG_LCD_ABL1_FILL = 0;
}*/
