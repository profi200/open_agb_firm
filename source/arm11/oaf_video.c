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

// Battery indicator state. Initialized from g_oafConfig.showBattery in OAF_videoInit().
static bool g_showBattery = false;

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

// libretro shader values. Credits: hunterk and Pokefan531.
// Last updated 2024-12-03.
static const ColorProfile g_colorProfiles[8] =
{
	{ // libretro GBA color (sRGB).
		2.2f + (0.3f * 1.6f), // Darken screen. Default 0. Modified to 0.3.
		0.91f,
		0.905f,  0.195f,  -0.1f,
		0.1f,    0.65f,    0.25f,
		0.1575f, 0.1425f,  0.7f,
		1.f / 2.2f
	},
	{ // libretro GB micro color (sRGB).
		2.2f,
		0.9f,
		0.8025f, 0.31f,   -0.1125f,
		0.1f,    0.6875f,  0.2125f,
		0.1225f, 0.1125f,  0.765f,
		1.f / 2.2f
	},
	{ // libretro GBA SP (AGS-101) color (sRGB).
		2.2f,
		0.935f,
		0.96f,    0.11f, -0.07f,
		0.0325f,  0.89f,  0.0775f,
		0.001f,  -0.03f,  1.029f,
		1.f / 2.2f
	},
	{ // libretro NDS color (sRGB).
		2.2f,
		0.905f,
		0.835f, 0.27f,   -0.105f,
		0.1f,   0.6375f,  0.2625f,
		0.105f, 0.175f,   0.72f,
		1.f / 2.2f
	},
	{ // libretro NDS lite color (sRGB).
		2.2f,
		0.935f,
		0.93f,   0.14f, -0.07f,
		0.025f,  0.9f,   0.075f,
		0.008f, -0.03f,  1.022f,
		1.f / 2.2f
	},
	{ // libretro Nintendo Switch Online color (sRGB).
		2.2f + 0.8f, // Darken screen. Default 0.8.
		1.f,
		0.865f,  0.1225f, 0.0125f,
		0.0575f, 0.925f,  0.0125f,
		0.0575f, 0.1225f, 0.82f,
		1.f / 2.2f
	},
	{ // libretro Visual Boy Advance/No$GBA full color.
		1.45f + 1.f, // Darken screen. Default 1.
		1.f,
		0.73f,   0.27f,   0.f,
		0.0825f, 0.6775f, 0.24f,
		0.0825f, 0.24f,   0.6775f,
		1.f / 1.45f
	},
	{ // Identity.
		2.2f,
		1.f,
		1.f, 0.f, 0.f,
		0.f, 1.f, 0.f,
		0.f, 0.f, 1.f,
		1.f / 2.2f
	}
};

ALWAYS_INLINE float clamp_float(const float x, const float min, const float max)
{
	return (x < min ? min : (x > max ? max : x));
}

void makeColorLut(const ColorProfile *const p)
{
	const float targetGamma    = p->targetGamma;
	const float contrast       = g_oafConfig.contrast;
	const float brightness     = g_oafConfig.brightness / contrast;
	const float targetContrast = powf(contrast, targetGamma);

	// Calculate saturation weights.
	// Note: We are using the Rec. 709 luminance vector here.
	const float sat   = g_oafConfig.saturation;
	const float rwgt  = (1.f - sat) * 0.2126f;
	const float gwgt  = (1.f - sat) * 0.7152f;
	const float bwgt  = (1.f - sat) * 0.0722f;

	u32 *const colorLut = (u32*)COLOR_LUT_ADDR;
	for(u32 i = 0; i < 32768; i++)
	{
		// Convert to 8-bit and normalize.
		float b = (float)rgbFive2Eight(i & 31u) / 255;
		float g = (float)rgbFive2Eight((i>>5) & 31u) / 255;
		float r = (float)rgbFive2Eight(i>>10) / 255;

		// Convert to linear gamma.
		b = powf(b + brightness, targetGamma);
		g = powf(g + brightness, targetGamma);
		r = powf(r + brightness, targetGamma);

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
		float tmpB = p->rb * r + p->gb * g + p->b  * b;
		float tmpG = p->rg * r + p->g  * g + p->bg * b;
		float tmpR = p->r  * r + p->gr * g + p->br * b;

		// Apply saturation.
		// Note: Some duplicated muls here. gcc optimizes them out.
		b = rwgt         * tmpR + gwgt         * tmpG + (bwgt + sat) * tmpB;
		g = rwgt         * tmpR + (gwgt + sat) * tmpG + bwgt         * tmpB;
		r = (rwgt + sat) * tmpR + gwgt         * tmpG + bwgt         * tmpB;

		b = (b < 0.f ? 0.f : b);
		g = (g < 0.f ? 0.f : g);
		r = (r < 0.f ? 0.f : r);

		// Convert to display gamma.
		const float displayGamma = p->displayGamma;
		b = powf(targetContrast * b, displayGamma);
		g = powf(targetContrast * g, displayGamma);
		r = powf(targetContrast * r, displayGamma);

		// Denormalize, clamp, convert to ABGR8 and write lut.
		u32 entry = 255; // Alpha.
		entry |= clamp_s32(lroundf(b * 255), 0, 255)<<8;
		entry |= clamp_s32(lroundf(g * 255), 0, 255)<<16;
		entry |= clamp_s32(lroundf(r * 255), 0, 255)<<24;
		colorLut[i] = entry;
	}

	flushDCacheRange(colorLut, 4 * 32768);
}

static Result dumpFrameTex(void)
{
	// Capture a single frame in native resolution.
	// Note: This adds 1 frame of delay after pressing the screenshot buttons.
	if(LGYCAP_captureFrameUnscaled(LGYCAP_DEV_TOP) != KRES_OK)
		return RES_INVALID_ARG;

	// A1BGR5 format (alpha ignored).
	constexpr u32 alignment = 0x80; // Make PPF happy.
	alignas(4) static const BmpV1WithMasks bmpHeaders =
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

	// Transfer frame data out of the 512x512 texture.
	// We will use the currently hidden frame buffer as temporary buffer.
	// Note: This is a race with the currently displaying frame buffer
	//       because we just swapped buffers in the gfx handler function.
	u32 *const tmpBuf = GFX_getBuffer(GFX_LCD_TOP, GFX_SIDE_LEFT);
	GX_displayTransfer((u32*)GPU_TEXTURE_ADDR, PPF_DIM(512, 160), tmpBuf + (alignment / 4), PPF_DIM(240, 160),
	                   PPF_O_FMT(GX_A1BGR5) | PPF_I_FMT(GX_A1BGR5) | PPF_CROP_EN);
	memcpy(tmpBuf, &bmpHeaders, sizeof(bmpHeaders));
	GFX_waitForPPF();

	// Get current date & time.
	RtcTimeDate td;
	MCU_getRtcTimeDate(&td);

	// Construct file path from date & time. Then write the file.
	char fn[36];
	ee_sprintf(fn, OAF_SCREENSHOT_DIR "/%04X_%02X_%02X_%02X_%02X_%02X.bmp",
	           td.year + 0x2000, td.mon, td.day, td.hour, td.min, td.sec);
	const Result res = fsQuickWrite(fn, tmpBuf, bmpHeaders.header.fileSize);

	// Clear overwritten texture area in case we overwrote padding (different resolution).
	// This is important because padding pixels must be fully transparent to get sharp edges when the GPU renders.
	GX_memoryFill((u32*)GPU_TEXTURE_ADDR, PSC_FILL_32_BITS, 512 * 160 * 2, 0, NULL, 0, 0, 0);
	GFX_waitForPSC0();

	// Restart LgyCap.
	LGYCAP_start(LGYCAP_DEV_TOP);

	return res;
}

// 3x5 digit bitmaps, each row is a byte with 3 LSBs used.
static const u8 g_digitFont[10][5] =
{
	{0x7, 0x5, 0x5, 0x5, 0x7}, // 0
	{0x2, 0x6, 0x2, 0x2, 0x7}, // 1
	{0x7, 0x1, 0x7, 0x4, 0x7}, // 2
	{0x7, 0x1, 0x7, 0x1, 0x7}, // 3
	{0x5, 0x5, 0x7, 0x1, 0x1}, // 4
	{0x7, 0x4, 0x7, 0x1, 0x7}, // 5
	{0x7, 0x4, 0x7, 0x5, 0x7}, // 6
	{0x7, 0x1, 0x2, 0x4, 0x4}, // 7
	{0x7, 0x5, 0x7, 0x5, 0x7}, // 8
	{0x7, 0x5, 0x7, 0x1, 0x7}, // 9
};

// Set a pixel in the tiled BGR8 GPU render buffer.
// The render buffer matches the linear framebuffer layout but in 8x8 Morton-order tiles.
// Linear framebuffer: pixel at screen (x, y) is at byte ((x * 240) + (239 - y)) * 3.
// So the tiled buffer "width" is 240 and "height" is 400 (LCD is rotated 90 CCW).
// Tiles are arranged in row-major order: 30 tiles across (240/8), 50 tiles down (400/8).
static inline void setPixelTiled(u8 *const buf, const u32 screenX, const u32 screenY, const u8 b, const u8 g, const u8 r)
{
	// Convert screen coords to tiled buffer coords.
	const u32 bufX = 239 - screenY; // 0..239, along buffer width
	const u32 bufY = screenX;       // 0..399, along buffer height

	// Tile position and pixel within tile.
	const u32 tileX = bufX >> 3;
	const u32 tileY = bufY >> 3;
	const u32 inX = bufX & 7;
	const u32 inY = bufY & 7;

	// PICA200 Morton (Z-order) index within 8x8 tile.
	// x bits in even positions, y bits in odd positions.
	const u32 morton = (inX & 1) | ((inY & 1) << 1)
	                 | ((inX & 2) << 1) | ((inY & 2) << 2)
	                 | ((inX & 4) << 2) | ((inY & 4) << 3);

	const u32 tileIdx = tileY * (240 / 8) + tileX;
	const u32 offset = (tileIdx * 64 + morton) * 3;

	buf[offset]     = b;
	buf[offset + 1] = g;
	buf[offset + 2] = r;
}

// Vertical battery indicator in the top-right corner.
// Fits within the right-side border strip (x>=360) that is not overwritten
// by the GPU in any scaler mode.
//
// Layout (screen coords):
//   y=2:       terminal nub (2px wide, centered on body)
//   y=3..10:   battery body outline 6px wide x 8px tall
//   y=12..16:  percentage text (up to 15px wide x 5px, centered)
//
// Body at x=385..390, nub at x=387..388, text centered on x=387
// Bounding box for clear/flush: x=380..399, y=2..16

#define BATT_REGION_X  (380u)
#define BATT_REGION_Y  (2u)
#define BATT_REGION_W  (20u)  // x 380..399
#define BATT_REGION_H  (15u)  // y 2..16

#define BATT_BODY_X    (385u)
#define BATT_BODY_Y    (3u)
#define BATT_BODY_W    (6u)
#define BATT_BODY_H    (8u)
#define BATT_INNER_W   (BATT_BODY_W - 2)  // 4
#define BATT_INNER_H   (BATT_BODY_H - 2)  // 24

static void drawBatteryRegion(u8 *const buf, const bool draw)
{
	// Clear the entire indicator region to black.
	for(u32 sy = BATT_REGION_Y; sy < BATT_REGION_Y + BATT_REGION_H; sy++)
		for(u32 sx = BATT_REGION_X; sx < BATT_REGION_X + BATT_REGION_W; sx++)
			setPixelTiled(buf, sx, sy, 0, 0, 0);

	if(!draw) goto flush;

	const u8 level = MCU_getBatteryLevel();

	// Color based on level.
	u8 cr, cg, cb;
	if(level >= 30)      { cr = 60;  cg = 200; cb = 60;  }
	else if(level >= 15) { cr = 220; cg = 200; cb = 20;  }
	else                 { cr = 220; cg = 40;  cb = 40;  }

	// Terminal nub: 2px wide, 1px tall, centered above body.
	const u32 nubX = BATT_BODY_X + (BATT_BODY_W / 2) - 1; // 395
	setPixelTiled(buf, nubX,     BATT_BODY_Y - 1, 255, 255, 255);
	setPixelTiled(buf, nubX + 1, BATT_BODY_Y - 1, 255, 255, 255);

	// Body outline.
	for(u32 dx = 0; dx < BATT_BODY_W; dx++)
	{
		setPixelTiled(buf, BATT_BODY_X + dx, BATT_BODY_Y,                    255, 255, 255);
		setPixelTiled(buf, BATT_BODY_X + dx, BATT_BODY_Y + BATT_BODY_H - 1, 255, 255, 255);
	}
	for(u32 dy = 0; dy < BATT_BODY_H; dy++)
	{
		setPixelTiled(buf, BATT_BODY_X,                    BATT_BODY_Y + dy, 255, 255, 255);
		setPixelTiled(buf, BATT_BODY_X + BATT_BODY_W - 1, BATT_BODY_Y + dy, 255, 255, 255);
	}

	// Fill interior from bottom up. Full = top, empty = bottom.
	const u32 fillH = (BATT_INNER_H * level + 50) / 100;
	const u32 innerX = BATT_BODY_X + 1;
	const u32 innerY = BATT_BODY_Y + 1;
	for(u32 dy = 0; dy < fillH; dy++)
		for(u32 dx = 0; dx < BATT_INNER_W; dx++)
			setPixelTiled(buf, innerX + dx, innerY + BATT_INNER_H - 1 - dy, cb, cg, cr);

	// Percentage text centered below the body.
	char numStr[4];
	u32 numLen;
	if(level >= 100)     { numStr[0] = '1'; numStr[1] = '0'; numStr[2] = '0'; numLen = 3; }
	else if(level >= 10) { numStr[0] = '0' + level / 10; numStr[1] = '0' + level % 10; numLen = 2; }
	else                 { numStr[0] = '0' + level; numLen = 1; }

	static const u8 percentGlyph[5] = {0x5, 0x1, 0x2, 0x4, 0x5};
	const u32 textW = numLen * 4 + 3; // digits + percent sign
	const u32 textX = BATT_BODY_X + (BATT_BODY_W / 2) - (textW / 2);
	const u32 textY = BATT_BODY_Y + BATT_BODY_H + 1;

	u32 cx = textX;
	for(u32 i = 0; i < numLen; i++)
	{
		const u8 *const glyph = g_digitFont[numStr[i] - '0'];
		for(u32 dy = 0; dy < 5; dy++)
			for(u32 dx = 0; dx < 3; dx++)
				if(glyph[dy] & (4 >> dx))
					setPixelTiled(buf, cx + dx, textY + dy, 255, 255, 255);
		cx += 4;
	}
	for(u32 dy = 0; dy < 5; dy++)
		for(u32 dx = 0; dx < 3; dx++)
			if(percentGlyph[dy] & (4 >> dx))
				setPixelTiled(buf, cx + dx, textY + dy, 255, 255, 255);

flush:
	// Flush affected render buffer tiles from cache.
	// The indicator spans tile rows starting at BATT_REGION_Y/8 = 0 through to the end
	// of the region. Conservative: flush from the tile column containing BATT_REGION_X.
	{
		const u32 startTileRow = BATT_REGION_X / 8; // tile "row" in buffer height = screen X
		const u32 tilesPerRow = 240 / 8;
		const u32 startOffset = startTileRow * tilesPerRow * 64 * 3;
		flushDCacheRange((u8*)GPU_RENDER_BUF_ADDR + startOffset,
		                 240 * 400 * 3 - startOffset);
	}
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
		// Update battery indicator in the render buffer before display transfer.
		// Only redraw on toggle or when the cached level changes (~every 2 seconds).
		{
			const u32 kHeld = hidKeysHeld();
			const u32 kDown = hidKeysDown();
			bool toggled = false;
			if(kHeld == (KEY_X | KEY_SELECT) && kDown != 0)
			{
				g_showBattery = !g_showBattery;
				toggled = true;
			}

			static u8 prevLevel = 0;
			static u32 frameCount = 0;
			if(g_showBattery)
			{
				// Refresh battery level roughly every 2 seconds.
				bool needsRedraw = toggled;
				if(++frameCount >= 120)
				{
					frameCount = 0;
					const u8 newLevel = MCU_getBatteryLevel();
					if(newLevel != prevLevel)
					{
						prevLevel = newLevel;
						needsRedraw = true;
					}
				}

				if(needsRedraw)
					drawBatteryRegion((u8*)GPU_RENDER_BUF_ADDR, true);
			}
			else if(toggled)
			{
				// Just toggled off — clear the region.
				drawBatteryRegion((u8*)GPU_RENDER_BUF_ADDR, false);
				frameCount = 0;
			}

			GX_displayTransfer((u32*)GPU_RENDER_BUF_ADDR, PPF_DIM(240, 400), GFX_getBuffer(GFX_LCD_TOP, GFX_SIDE_LEFT),
			                   PPF_DIM(240, 400), PPF_O_FMT(GX_BGR8) | PPF_I_FMT(GX_BGR8));
			GFX_waitForPPF();
			GFX_swapBuffers();

			// Trigger only if both are held and at least one is detected as newly pressed down.
			if(kHeld == (KEY_Y | KEY_SELECT) && kDown != 0)
				dumpFrameTex();
		}
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

	// Initialize battery indicator from config.
	g_showBattery = g_oafConfig.showBattery;
	if(g_showBattery)
		drawBatteryRegion((u8*)GPU_RENDER_BUF_ADDR, true);

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