/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2024 profi200
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

#include <math.h>
#include <string.h>
#include "types.h"
#include "arm11/config.h"
#include "arm11/drivers/gx.h"
#include "drivers/cache.h"
#include "util.h"
#include "oaf_error_codes.h"
#include "arm11/drivers/lgycap.h"
#include "arm11/bitmap.h"
#include "drivers/gfx.h"
#include "arm11/drivers/mcu.h"
#include "arm11/fmt.h"
#include "fsutil.h"
#include "kernel.h"
#include "kevent.h"
#include "arm11/drivers/hid.h"
#include "arm11/drivers/interrupt.h"
#include "arm11/gpu_cmd_lists.h"
#include "system.h"
#include "arm11/fast_frame_convert.h"


#define COLOR_LUT_ADDR (0x1FF00000u)


static KHandle g_convFinishedEvent = 0;
static const u32 g_topLcdCurveCorrect[73] =
{
	// Curve correction from 3DS top LCD gamma to 2.2 gamma for all channels.
	// Unfortunately this doesn't fix color temperature but that varies wildly
	// for all 2/3DS consoles on the market anyway.
	0x01000000, 0x03020102, 0x00060406, 0x01060507,
	0x03080608, 0x020B090C, 0x010E0B0F, 0x010F0D10,
	0x01110E12, 0x01121014, 0x00141116, 0x00151216,
	0x01151317, 0x01171419, 0x0118161B, 0x011A171C,
	0x021B191E, 0x001E1B21, 0x011E1C22, 0x01201E23,
	0x03211F25, 0x01242329, 0x0226242B, 0x0028272E,
	0x0229282E, 0x002B2B31, 0x042C2B32, 0x04303037,
	0x0034353C, 0x0535353D, 0x073A3B43, 0x0A41434B,
	0x034B4E56, 0x084F525B, 0x0D575B64, 0x03656973,
	0x12686D77, 0x017B818A, 0x167C838C, 0x03939AA4,
	0x0E979FA8, 0x01A6AFB7, 0x06A9B1B9, 0x01B0B9C0,
	0x00B2BBC3, 0x04B4BCC4, 0x00B9C2C9, 0x04BBC3CA,
	0x00C1C8CF, 0x00C2CAD0, 0x02C3CBD2, 0x01C7CED5,
	0x00C9D1D7, 0x03CBD2D8, 0x00D0D7DC, 0x01D1D8DE,
	0x01D4DAE0, 0x00D6DDE2, 0x02D8DEE3, 0x00DCE1E6,
	0x01DDE3E7, 0x02E0E5EA, 0x01E4E9ED, 0x02E7EBEF,
	0x01EBEFF2, 0x01EEF1F4, 0x00F1F3F6, 0x01F2F5F7,
	0x01F5F7F9, 0x01F8F9FB, 0x00FAFCFD, 0x01FCFDFD,
	0x00FFFFFF
};



// TODO: Reimplement contrast and brightness in color lut below.
static void adjustGammaTableForGba(void)
{
	// Credits for this algo go to Extrems.
	/*const float targetGamma = g_oafConfig.gbaGamma;
	const float lcdGamma    = 1.f / g_oafConfig.lcdGamma;
	const float contrast    = g_oafConfig.contrast;
	const float brightness  = g_oafConfig.brightness / contrast;
	const float contrastInTargetGamma = powf(contrast, targetGamma);
	vu32 *const color_lut_data = &getGxRegs()->pdc0.color_lut_data;
	for(u32 i = 0; i < 256; i++)
	{
		// Adjust i with brightness and convert to target gamma.
		const float adjusted = powf((float)i / 255 + brightness, targetGamma);

		// Apply contrast, convert to LCD gamma, round to nearest and clamp.
		const u32 res = clamp_s32(lroundf(powf(contrastInTargetGamma * adjusted, lcdGamma) * 255), 0, 255);

		// Same adjustment for red/green/blue.
		*color_lut_data = res<<16 | res<<8 | res;
	}*/

	// Very simple gamma table expansion code.
	// Code + hardcoded tables are way smaller than hardcoding the uncompressed tables.
	const u32 *encTable = g_topLcdCurveCorrect;
	vu32 *const color_lut_data = &getGxRegs()->pdc0.color_lut_data;
	u32 decoded = 0;
	do
	{
		// Get table entry and extract the number of linearly increasing entries.
		u32 entry = *encTable++;
		u32 steps = (entry>>24) + 1;

		// Keep track of how many table entries we generated.
		decoded += steps;
		do
		{
			// Set gamma table entry and increment.
			// Note: Bits 24-31 don't matter so we don't need to mask.
			*color_lut_data = entry;
			entry += 0x010101;
		} while(--steps != 0);
	} while(decoded < 256);
}

typedef struct
{
	float targetGamma;
	float lum;
	float  r, gr, br;
	float rg,  g, bg;
	float rb, gb,  b;
	float displayGamma;
} ColorProfile;

static const ColorProfile g_colorProfiles[3] =
{
	{ // libretro GBA color (sRGB). Credits: hunterk and Pokefan531.
		2.f + 0.5f,
		0.93f,
		0.8f,   0.275f, -0.075f,
		0.135f, 0.64f,   0.225f,
		0.195f, 0.155f,  0.65f,
		1.f / 2.f
	},
	{ // libretro DS phat (sRGB). Credits: hunterk and Pokefan531.
		2.f,
		1.f,
		0.705f,  0.235f,  -0.075f,
		0.09f,   0.585f,   0.24f,
		0.1075f, 0.1725f,  0.72f,
		1.f / 2.f
	},
	{ // libretro DS phat white (sRGB). Credits: hunterk and Pokefan531.
		2.f,
		0.915f,
		0.815f,  0.275f,  -0.09f,
		0.1f,    0.64f,    0.26f,
		0.1075f, 0.1725f,  0.72f,
		1.f / 2.f
	}
};

ALWAYS_INLINE float clamp_float(const float x, const float min, const float max)
{
	return (x < min ? min : (x > max ? max : x));
}

static void makeColorLut(const ColorProfile *const p)
{
	u32 *colorLut = (u32*)COLOR_LUT_ADDR;
	for(u32 i = 0; i < 32768; i++)
	{
		// Convert to 8-bit and normalize.
		float b = (float)rgbFive2Eight(i & 31u) / 255;
		float g = (float)rgbFive2Eight((i>>5) & 31u) / 255;
		float r = (float)rgbFive2Eight(i>>10) / 255;

		// Convert to linear gamma.
		const float targetGamma = p->targetGamma;
		b = powf(b, targetGamma);
		g = powf(g, targetGamma);
		r = powf(r, targetGamma);

		// Apply luminance.
		const float lum = p->lum;
		b = clamp_float(b * lum, 0.f, 1.f);
		g = clamp_float(g * lum, 0.f, 1.f);
		r = clamp_float(r * lum, 0.f, 1.f);

		/*
		 *               Input
		 *                [r]
		 *                [g]
		 *                [b]
		 *
		 * Correction    Output
		 * [ r][gr][br]   [r]
		 * [rg][ g][bg]   [g]
		 * [rb][gb][ b]   [b]
		*/
		// Assuming no alpha channel in original calculation.
		float newB = p->rb * r + p->gb * g + p->b * b;
		float newG = p->rg * r + p->g * g + p->bg * b;
		float newR = p->r * r + p->gr * g + p->br * b;

		newB = (newB < 0.f ? 0.f : newB);
		newG = (newG < 0.f ? 0.f : newG);
		newR = (newR < 0.f ? 0.f : newR);

		// Convert to display gamma.
		const float displayGamma = p->displayGamma;
		newB = powf(newB, displayGamma);
		newG = powf(newG, displayGamma);
		newR = powf(newR, displayGamma);

		// Denormalize, clamp, convert to ABGR8 and write lut.
		u32 tmp = 0xFF; // Alpha.
		tmp |= clamp_s32(lroundf(newB * 255), 0, 255)<<8;
		tmp |= clamp_s32(lroundf(newG * 255), 0, 255)<<16;
		tmp |= clamp_s32(lroundf(newR * 255), 0, 255)<<24;
		*colorLut++ = tmp;
	}

	flushDCacheRange((void*)COLOR_LUT_ADDR, 1024u * 128);
}

static Result dumpFrameTex(void)
{
	// Stop LgyCap before dumping the frame to prevent glitches.
	LGYCAP_stop(LGYCAP_DEV_TOP);

	// A1BGR5 format (alpha ignored).
	constexpr u32 alignment = 0x80; // Make PPF happy.
	alignas(4) static BmpV1WithMasks bmpHeaders =
	{
		{
			.magic       = 0x4D42,
			.fileSize    = alignment + 240 * 160 * 2,
			.reserved    = 0,
			.reserved2   = 0,
			.pixelOffset = alignment
		},
		{
			.headerSize      = sizeof(Bitmapinfoheader),
			.width           = 240,
			.height          = -160,
			.colorPlanes     = 1,
			.bitsPerPixel    = 16,
			.compression     = BI_BITFIELDS,
			.imageSize       = 240 * 160 * 2,
			.xPixelsPerMeter = 0,
			.yPixelsPerMeter = 0,
			.colorsUsed      = 0,
			.colorsImportant = 0
		},
		.rMask = 0xF800,
		.gMask = 0x07C0,
		.bMask = 0x003E
	};

	u32 outDim   = PPF_DIM(240, 160);
	u32 fileSize = alignment + 240 * 160 * 2;
	if(g_oafConfig.scaler > 1)
	{
		outDim   = PPF_DIM(360, 240);
		fileSize = alignment + 360 * 240 * 2;

		bmpHeaders.header.fileSize = fileSize;
		bmpHeaders.dib.width     = 360;
		bmpHeaders.dib.height    = -240;
		bmpHeaders.dib.imageSize = 360 * 240 * 2;
	}

	// Transfer frame data out of the 512x512 texture.
	// We will use the currently hidden frame buffer as temporary buffer.
	// Note: This is a race with the currently displaying frame buffer
	//       because we just swapped buffers in the gfx handler function.
	u32 *const tmpBuf = GFX_getBuffer(GFX_LCD_TOP, GFX_SIDE_LEFT);
	GX_displayTransfer((u32*)GPU_TEXTURE_ADDR, PPF_DIM(512, 240), tmpBuf + (alignment / 4), outDim,
	                   PPF_O_FMT(GX_A1BGR5) | PPF_I_FMT(GX_A1BGR5) | PPF_CROP_EN);
	memcpy(tmpBuf, &bmpHeaders, sizeof(bmpHeaders));
	GFX_waitForPPF();

	// Get current date & time.
	RtcTimeDate td;
	MCU_getRtcTimeDate(&td);

	// Construct file path from date & time. Then write the file.
	char fn[36];
	ee_sprintf(fn, OAF_SCREENSHOT_DIR "/%04X_%02X_%02X_%02X_%02X_%02X.bmp",
	           td.y + 0x2000, td.mon, td.d, td.h, td.min, td.s);
	const Result res = fsQuickWrite(fn, tmpBuf, fileSize);

	// Restart LgyCap.
	LGYCAP_start(LGYCAP_DEV_TOP);

	return res;
}

static void convFinishedHandler(UNUSED const u32 intSource)
{
	signalEvent(g_convFinishedEvent, false);
}

static void gbaGfxHandler(void *args)
{
	const KHandle event = (KHandle)args;

	while(1)
	{
		if(waitForEvent(event) != KRES_OK) break;
		clearEvent(event);

		// All measurements are the worst timings in ~30 seconds of runtime.
		// Measured with timer prescaler 1.
		// BGR8:
		// 240x160 no scaling:    ~184 µs
		// 240x160 bilinear x1.5: ~408 µs
		// 360x240 no scaling:    ~437 µs
		//
		// A1BGR5:
		// 240x160 no scaling:    ~188 µs (25300 ticks)
		// 240x160 bilinear x1.5: ~407 µs (54619 ticks)
		// 360x240 no scaling:    ~400 µs (53725 ticks)
		static bool inited = false;
		u32 listSize;
		const u32 *list;
		if(inited == false)
		{
			inited = true;

			listSize = sizeof(gbaGpuInitList);
			list = (u32*)gbaGpuInitList;
		}
		else
		{
			listSize = sizeof(gbaGpuList2);
			list = (u32*)gbaGpuList2;
		}
		GX_processCommandList(listSize, list);
		GFX_waitForP3D();
		GX_displayTransfer((u32*)GPU_RENDER_BUF_ADDR, PPF_DIM(240, 400), GFX_getBuffer(GFX_LCD_TOP, GFX_SIDE_LEFT),
		                   PPF_DIM(240, 400), PPF_O_FMT(GX_BGR8) | PPF_I_FMT(GX_BGR8));
		GFX_waitForPPF();
		GFX_swapBuffers();

		// Trigger only if both are held and at least one is detected as newly pressed down.
		if(hidKeysHeld() == (KEY_Y | KEY_SELECT) && hidKeysDown() != 0)
			dumpFrameTex();
	}

	taskExit();
}

static KHandle setupFrameCapture(const u8 scaler, const bool colorCorrectionEnabled)
{
	const bool is240x160 = scaler < 2;
	static s16 matrix[12 * 8] =
	{
		// Vertical.
		      0,       0,       0,       0,       0,       0,       0,       0,
		      0,       0,       0,       0,       0,       0,       0,       0,
		      0,  0x24B0,  0x4000,       0,  0x24B0,  0x4000,       0,       0,
		 0x4000,  0x2000,       0,  0x4000,  0x2000,       0,       0,       0,
		      0,  -0x4B0,       0,       0,  -0x4B0,       0,       0,       0,
		      0,       0,       0,       0,       0,       0,       0,       0,

		// Horizontal.
		      0,       0,       0,       0,       0,       0,       0,       0,
		      0,       0,       0,       0,       0,       0,       0,       0,
		      0,       0,  0x24B0,       0,       0,  0x24B0,       0,       0,
		 0x4000,  0x4000,  0x2000,  0x4000,  0x4000,  0x2000,       0,       0,
		      0,       0,  -0x4B0,       0,       0,  -0x4B0,       0,       0,
		      0,       0,       0,       0,       0,       0,       0,       0
	};

	const Result res = fsQuickRead("gba_scaler_matrix.bin", matrix, sizeof(matrix));
	if(res != RES_OK && res != RES_FR_NO_FILE)
	{
		ee_printf("Failed to load hardware scaling matrix: %s\n", result2String(res));
	}

	LgyCapCfg gbaCfg;
	gbaCfg.cnt   = LGYCAP_SWIZZLE | LGYCAP_ROT_NONE | LGYCAP_FMT_A1BGR5 | (is240x160 ? 0 : LGYCAP_HSCALE_EN | LGYCAP_VSCALE_EN);
	gbaCfg.w     = (is240x160 ? 240 : 360);
	gbaCfg.h     = (is240x160 ? 160 : 240);
	gbaCfg.irq   = (colorCorrectionEnabled ? LGYCAP_IRQ_DMA_REQ : 0); // We need the DMA request IRQ for core 1.
	gbaCfg.vLen  = 6;
	gbaCfg.vPatt = 0b00011011;
	memcpy(gbaCfg.vMatrix, matrix, 6 * 8 * 2);
	gbaCfg.hLen  = 6;
	gbaCfg.hPatt = 0b00011011;
	memcpy(gbaCfg.hMatrix, &matrix[6 * 8], 6 * 8 * 2);

	return LGYCAP_init(LGYCAP_DEV_TOP, &gbaCfg);
}

KHandle OAF_videoInit(void)
{
#ifdef NDEBUG
	// Force black and turn the backlight off on the bottom screen.
	// Don't turn the backlight off on 2DS (1 panel).
	GFX_setForceBlack(false, true);
	if(MCU_getSystemModel() != SYS_MODEL_2DS)
		GFX_powerOffBacklight(GFX_BL_BOT);
#endif

	// Initialize frame capture.
	const u8 scaler = g_oafConfig.scaler;
	const u8 colorProfile = g_oafConfig.colorProfile;
	KHandle frameReadyEvent;
	KHandle convFinishedEvent;
	if(colorProfile > 0)
	{
		// Start capture hardware and create event handles.
		frameReadyEvent = setupFrameCapture(scaler, true);
		convFinishedEvent = createEvent(false);
		g_convFinishedEvent = convFinishedEvent;

		// Patch GPU cmd list with texture location 2.
		patchGbaGpuCmdList(scaler, true);

		// Compute the (linear) 3D lookup table.
		makeColorLut(&g_colorProfiles[colorProfile - 1]);

		// Register IPI handler and start core 1 for color conversion.
		IRQ_registerIsr(IRQ_IPI15, 13, 0, convFinishedHandler);
		__systemBootCore1((scaler < 2 ? convert160pFrameFast : convert240pFrameFast));
	}
	else
	{
		// Start capture hardware.
		frameReadyEvent = setupFrameCapture(scaler, false);

		// Patch GPU cmd list with texture location 1.
		patchGbaGpuCmdList(scaler, false);
	}

	// Start frame handler.
	createTask(0x800, 3, gbaGfxHandler, (void*)(colorProfile > 0 ? convFinishedEvent : frameReadyEvent));

	// Adjust hardware gamma table.
	adjustGammaTableForGba();

	// Load border if any exists.
	if(scaler == 0) // No borders for scaled modes.
	{
		// Abuse currently invisible frame buffer as temporary buffer.
		void *const borderBuf = GFX_getBuffer(GFX_LCD_TOP, GFX_SIDE_LEFT);
		if(fsQuickRead("border.bgr", borderBuf, 400 * 240 * 3) == RES_OK)
		{
			// Copy border in swizzled form to GPU render buffer.
			GX_displayTransfer(borderBuf, PPF_DIM(240, 400), (u32*)GPU_RENDER_BUF_ADDR,
			                   PPF_DIM(240, 400), PPF_O_FMT(GX_BGR8) | PPF_I_FMT(GX_BGR8) | PPF_OUT_TILED);
			GFX_waitForPPF();
		}
	}

	return frameReadyEvent;
}

void OAF_videoExit(void)
{
	// frameReadyEvent deleted by this function.
	// gbaGfxHandler() will automatically terminate.
	LGYCAP_deinit(LGYCAP_DEV_TOP);
	if(g_convFinishedEvent != 0)
	{
		deleteEvent(g_convFinishedEvent);
		g_convFinishedEvent = 0;
	}
}