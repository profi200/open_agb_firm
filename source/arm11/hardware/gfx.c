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

/*
 * Based on code from https://github.com/AuroraWright/Luma3DS
 * for compatibility.
 * 
 * Credits go to the Luma3DS devs and derrek for reverse engineering boot11.
*/

#include <stdatomic.h>
#include "types.h"
#include "mem_map.h"
#include "hardware/gfx.h"
#include "mmio.h"
#include "arm11/hardware/i2c.h"
#include "arm11/hardware/mcu.h"
#include "arm11/hardware/interrupt.h"
#include "arm11/hardware/timer.h"
#include "arm.h"
#include "util.h"


#define PDN_REGS_BASE            (IO_MEM_ARM9_ARM11 + 0x40000)
#define REG_PDN_GPU_CNT          *((vu32*)(PDN_REGS_BASE + 0x1200))

// LCD/ABL regs.
#define LCD_REGS_BASE            (IO_MEM_ARM11_ONLY + 0x2000)
#define REG_LCD_FILL_TOP         *((vu32*)(LCD_REGS_BASE + 0x204))
#define REG_LCD_FILL_BOT         *((vu32*)(LCD_REGS_BASE + 0xA04))
#define REG_LCD_LIGHT_TOP        *((vu32*)(LCD_REGS_BASE + 0x240))
#define REG_LCD_LIGHT_PWM_TOP    *((vu32*)(LCD_REGS_BASE + 0x244))
#define REG_LCD_LIGHT_BOT        *((vu32*)(LCD_REGS_BASE + 0xA40))
#define REG_LCD_LIGHT_PWM_BOT    *((vu32*)(LCD_REGS_BASE + 0xA44))

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
#define REG_GX_P3D_LIST_SIZE     *((vu32*)(GX_REGS_BASE + 0x18E0)) // cmd list length in 16 bytes units.
#define REG_GX_P3D_LIST_ADR      *((vu32*)(GX_REGS_BASE + 0x18E8)) // cmd list address / 64.
#define REG_GX_P3D_LIST_RUN      *((vu32*)(GX_REGS_BASE + 0x18F0)) // Start list processing by writing 1.


static struct
{
	u16 lcdIds;            // Bits 0-7 top screen, 8-15 bottom screen.
	bool lcdIdsRead;
	bool events[6];
	u32 activeFb;
	void *framebufs[2][4]; // A1, A2, B1, B2
	u32 formats[2];        // Top, bottom
} g_gfxState =
{
	0,
	false,
	{false, false, false, false, false, false},
	0,
	{
		{FRAMEBUF_TOP_A_1, FRAMEBUF_TOP_A_2, FRAMEBUF_TOP_A_1, FRAMEBUF_TOP_A_2},
		{FRAMEBUF_BOT_A_1, FRAMEBUF_BOT_A_2, FRAMEBUF_BOT_A_1, FRAMEBUF_BOT_A_2}
	},
	{8<<16 | 3<<8 | 1<<6 | 0<<4 | 3, 8<<16 | 3<<8 | 0<<6 | 0<<4 | 3}
};



static u8 fmt2PixSize(u32 format)
{
	u8 size;

	switch(format & 7u)
	{
		// Pretend RGBA8 doesn't exist because it makes no sense for framebuffers.
		//case 0:  // RGBA8888
		//	size = 4;
		//	break;
		case 1:  // RGBA8880
			size = 3;
			break;
		default: // 2 = RGBA5650, 3 = RGBA5551, 4 = RGBA4444
			size = 2;
	}

	return size;
}

static void gfxSetupFramebuf(u8 lcd)
{
	if(lcd > 1) return;

	static const u32 framebufCfgs[2][24] =
	{
		{
			// 0-0x4C
			450, 209, 449, 449, 0, 207, 209, 453<<16 | 449,
			1<<16 | 0, 413, 2, 402, 402, 402, 1, 2,
			406<<16 | 402, 0, 0<<4 | 0, 0<<16 | 0xFF<<8 | 0,
			// 0x5C-0x64
			400<<16 | 240, // Width and height
			449<<16 | 209,
			402<<16 | 2,
			// 0x9C
			0<<16 | 0
		},
		{
			// 0-0x4C
			450, 209, 449, 449, 205, 207, 209, 453<<16 | 449,
			1<<16 | 0, 413, 82, 402, 402, 79, 80, 82,
			408<<16 | 404, 0, 1<<4 | 1, 0<<16 | 0<<8 | 0xFF,
			// 0x5C-0x64
			320<<16 | 240, // Width and height
			449<<16 | 209,
			402<<16 | 82,
			// 0x9C
			0<<16 | 0
		}
	};

	const u32 *const cfg = framebufCfgs[lcd];
	vu32 *const regs = (vu32*)(GX_REGS_BASE + 0x400 + (0x100u * lcd));

	const u32 format = g_gfxState.formats[lcd];
	iomemcpy(regs, cfg, 0x50);             // 0-0x4C
	iomemcpy(regs + 23, &cfg[20], 0xC);    // 0x5C-0x64
	regs[36] = 240u * fmt2PixSize(format); // 0x90 stride without gap.
	regs[39] = cfg[23];                    // 0x9C


	// 0x68, 0x6C, 0x94, 0x98 and 0x70
	regs[26] = (u32)g_gfxState.framebufs[lcd][0]; // Framebuffer A first address.
	regs[27] = (u32)g_gfxState.framebufs[lcd][1]; // Framebuffer A second address.
	regs[37] = (u32)g_gfxState.framebufs[lcd][2]; // Framebuffer B first address.
	regs[38] = (u32)g_gfxState.framebufs[lcd][3]; // Framebuffer B second address.
	regs[28] = format;                            // Format


	regs[32] = 0; // Gamma table index 0.
	for(u32 i = 0; i < 256; i++) regs[33] = 0x10101u * i;
}

void GX_memoryFill(u64 *buf0a, u32 buf0v, u32 buf0Sz, u32 val0, u64 *buf1a, u32 buf1v, u32 buf1Sz, u32 val1)
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
void GX_displayTransfer(u64 *in, u32 indim, u64 *out, u32 outdim, u32 flags)
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
void GX_textureCopy(u64 *in, u32 indim, u64 *out, u32 outdim, u32 size)
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

void GFX_setBrightness(u32 top, u32 sub)
{
	REG_LCD_LIGHT_TOP = top;
	REG_LCD_LIGHT_BOT = sub;
}

void* GFX_getFramebuffer(u8 screen)
{
	return g_gfxState.framebufs[screen][g_gfxState.activeFb ^ 1u];
}

void GFX_swapFramebufs(void)
{
	u32 fb = g_gfxState.activeFb;
	fb ^= 1u;
	g_gfxState.activeFb = fb;

	fb |= 0x70000u; // Acknowledge IRQs.
	*((vu32*)0x10400478) = fb;
	*((vu32*)0x10400578) = fb;
}

static void gfxIrqHandler(u32 intSource)
{
	bool *const events = g_gfxState.events;

	atomic_store_explicit(&events[intSource - IRQ_PSC0], true, memory_order_relaxed);
}

void GFX_waitForEvent(GfxEvent event, bool discard)
{
	bool *const events = g_gfxState.events;

	if(discard) atomic_store_explicit(&events[event], false, memory_order_relaxed);
	while(!atomic_load_explicit(&events[event], memory_order_relaxed)) __wfe();
	atomic_store_explicit(&events[event], false, memory_order_relaxed);
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

void GFX_init(void)
{
	if(REG_PDN_GPU_CNT == 0x1007F) GFX_deinit(false);

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
	gfxSetupFramebuf(0);
	gfxSetupFramebuf(1);
	*((vu32*)0x10400478) = 0x70100; // Framebuffer select 0.
	*((vu32*)0x10400578) = 0x70100; // Framebuffer select 0.
	*((vu32*)0x10400474) = 0x10501; // Start
	*((vu32*)0x10400574) = 0x10501; // Start

	// LCD reg setup.
	REG_LCD_FILL_TOP = 1u<<24; // Force blackscreen
	REG_LCD_FILL_BOT = 1u<<24; // Force blackscreen
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

	// Backlight and other stuff.
	REG_LCD_LIGHT_TOP = 64;
	*((vu32*)0x10202200) = 0;
	REG_LCD_LIGHT_PWM_TOP = 0;
	REG_LCD_LIGHT_BOT = 64;
	*((vu32*)0x10202A00) = 0;
	REG_LCD_LIGHT_PWM_BOT = 0;

	GFX_setBrightness(DEFAULT_BRIGHTNESS, DEFAULT_BRIGHTNESS);
	*((vu32*)0x10202014) = 1;
	*((vu32*)0x1020200C) = 0;
	TIMER_sleepMs(10);
	resetLcdsMaybe();
	MCU_powerOnLCDs(); // Power on LCDs.
	MCU_waitEvents(0x3Fu<<24); // Wait for MCU event 25. 24 on poweroff.

	GX_memoryFill((u64*)FRAMEBUF_TOP_A_1, 1u<<9, SCREEN_SIZE_TOP + SCREEN_SIZE_BOT, 0,
	              (u64*)FRAMEBUF_TOP_A_2, 1u<<9, SCREEN_SIZE_TOP + SCREEN_SIZE_BOT, 0);
	GFX_waitForEvent(GFX_EVENT_PSC1, true);
	GX_memoryFill((u64*)RENDERBUF_TOP, 1u<<9, SCREEN_SIZE_TOP, 0, (u64*)RENDERBUF_BOT, 1u<<9, SCREEN_SIZE_BOT, 0);
	GFX_waitForEvent(GFX_EVENT_PSC0, true);

	// The transfer engine is (sometimes) borked on screen init.
	// Doing a dummy texture copy fixes it.
	// TODO: Proper fix.
	GX_textureCopy((u64*)RENDERBUF_TOP, 0, (u64*)RENDERBUF_BOT, 0, 16);

	waitLcdsReady();
	REG_LCD_LIGHT_PWM_TOP = 0x1023E;
	REG_LCD_LIGHT_PWM_BOT = 0x1023E;
	MCU_powerOnLcdLights();
	MCU_waitEvents(0x3Fu<<24); // Wait for MCU event 29 (top) and 27 (bot). 28 and 26 on poweroff.
	// Weird VBlank wait?

	REG_LCD_FILL_TOP = 0;
	REG_LCD_FILL_BOT = 0;
}

void GFX_gpuInit(void)
{
	*((vu32*)0x10140140) = 0; // REG_CFG11_GPUPROT
	//*((vu32*)0x10141204) = 1;

	REG_GX_GPU_CLK = 0x70100;
	*((vu32*)0x10400050) = 0x22221200;
	*((vu32*)0x10400054) = 0xFF2;

	*((vu32*)0x10401000) = 0;
	*((vu32*)0x10401080) = 0x12345678;
	*((vu32*)0x104010C0) = 0xFFFFFFF0;
	*((vu32*)0x104010D0) = 1;


	// Works.
	/*alignas(16) static const u32 cmdList[4] = {0x12345678, 0xF0010, 0x12345678, 0xF0010};
	GX_processCommandList(16, cmdList);
	GFX_waitForEvent(GFX_EVENT_P3D, false);*/
}

void GX_processCommandList(u32 size, const u32 *const cmdList)
{
	REG_GX_P3D_UNK = 0; // Acknowledge last P3D?
	REG_GX_P3D_LIST_SIZE = size>>3;
	REG_GX_P3D_LIST_ADR = (u32)cmdList>>3;
	REG_GX_P3D_LIST_RUN = 1;
}

// TODO: Sleep mode stuff needs some work.
/*void GFX_enterLowPowerState(void)
{
	REG_LCD_FILL_TOP = 1u<<24; // Force blackscreen
	REG_LCD_FILL_BOT = 1u<<24; // Force blackscreen
	GFX_waitForEvent(GFX_EVENT_PDC0, true);
	GFX_deinit(false); // REG_PDN_GPU_CNT = 0x7F;
}

void GFX_returnFromLowPowerState(void)
{
	GFX_init();
}*/

void GFX_deinit(bool keepLcdsOn)
{
	if(keepLcdsOn)
	{
		GX_memoryFill((u64*)FRAMEBUF_TOP_A_1, 1u<<9, SCREEN_SIZE_TOP + SCREEN_SIZE_BOT + 0x2A300, 0,
		              (u64*)FRAMEBUF_TOP_A_2, 1u<<9, SCREEN_SIZE_TOP + SCREEN_SIZE_BOT + 0x2A300, 0);
		GFX_waitForEvent(GFX_EVENT_PSC1, true);
		*((vu32*)(0x10400400+0x70)) = 0x80341;                         // Format GL_RGB8_OES.
		*((vu32*)(0x10400400+0x78)) = 0;                               // Select first framebuffer.
		*((vu32*)(0x10400400+0x90)) = SCREEN_HEIGHT_TOP * 3;           // Stride (no gap).
		*((vu32*)(0x10400500+0x68)) = (u32)FRAMEBUF_BOT_A_1 + 0x17700; // Bottom framebuffer first address.
		*((vu32*)(0x10400500+0x6C)) = (u32)FRAMEBUF_BOT_A_2 + 0x17700; // Bottom framebuffer second address.
		*((vu32*)(0x10400500+0x70)) = 0x80301;                         // Format GL_RGB8_OES.
		*((vu32*)(0x10400500+0x78)) = 0;                               // Select first framebuffer.
		*((vu32*)(0x10400500+0x90)) = SCREEN_HEIGHT_BOT * 3;           // Stride (no gap).
	}
	else
	{
		MCU_powerOffLCDs();
		MCU_waitEvents(0x3Fu<<24);
		GFX_setBrightness(0, 0);
		REG_LCD_LIGHT_PWM_TOP = 0;
		REG_LCD_LIGHT_PWM_BOT = 0;
		*((vu32*)0x1020200C) = 0x10001;
		*((vu32*)0x10202014) = 0;
		REG_GX_PSC_UNK = 0xF00;
		REG_PDN_GPU_CNT = 0x10001;
	}

	IRQ_disable(IRQ_PSC0);
	IRQ_disable(IRQ_PSC1);
	IRQ_disable(IRQ_PDC0);
	//IRQ_disable(IRQ_PDC1);
	IRQ_disable(IRQ_PPF);
	//IRQ_disable(IRQ_P3D);
}
