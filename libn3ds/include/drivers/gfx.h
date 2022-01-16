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
#include "types.h"


#define SCREEN_TOP          (0u)
#define SCREEN_BOT          (1u)

#define SCREEN_WIDTH_TOP    (400u)
#define SCREEN_HEIGHT_TOP   (240u)
#define SCREEN_SIZE_TOP     (SCREEN_WIDTH_TOP * SCREEN_HEIGHT_TOP * 2)
#define SCREEN_WIDTH_BOT    (320u)
#define SCREEN_HEIGHT_BOT   (240u)
#define SCREEN_SIZE_BOT     (SCREEN_WIDTH_BOT * SCREEN_HEIGHT_BOT * 2)

// TODO:
// Because we are using a VRAM allocator this may break any time.
#define FRAMEBUF_TOP_A_1    ((void*)VRAM_BASE)
#define FRAMEBUF_BOT_A_1    (FRAMEBUF_TOP_A_1 + SCREEN_SIZE_TOP)
#define FRAMEBUF_TOP_A_2    (FRAMEBUF_BOT_A_1 + SCREEN_SIZE_BOT + SCREEN_SIZE_TOP) // Skip B1
#define FRAMEBUF_BOT_A_2    (FRAMEBUF_TOP_A_2 + SCREEN_SIZE_TOP)

#define DEFAULT_BRIGHTNESS  (0x30)

/// Converts packed RGB8 to packed RGB565.
#define RGB8_to_565(r,g,b)  (((((r)>>3) & 0x1f)<<11) | ((((g)>>2) & 0x3f)<<5) | (((b)>>3) & 0x1f))


/// Framebuffer format.
typedef enum
{
	GFX_RGBA8  = 0,  ///< RGBA8. (4 bytes)
	GFX_BGR8   = 1,  ///< BGR8. (3 bytes)
	GFX_RGB565 = 2,  ///< RGB565. (2 bytes)
	GFX_RGB5A1 = 3,  ///< RGB5A1. (2 bytes)
	GFX_RGBA4  = 4   ///< RGBA4. (2 bytes)
} GfxFbFmt;


#ifdef ARM11
typedef enum
{
	GFX_EVENT_PSC0   = 0u,
	GFX_EVENT_PSC1   = 1u,
	GFX_EVENT_PDC0   = 2u,
	GFX_EVENT_PDC1   = 3u,
	GFX_EVENT_PPF    = 4u,
	GFX_EVENT_P3D    = 5u
} GfxEvent;

typedef enum
{
	GFX_BLIGHT_BOT   = 1u<<2,
	GFX_BLIGHT_TOP   = 1u<<4,
	GFX_BLIGHT_BOTH  = GFX_BLIGHT_TOP | GFX_BLIGHT_BOT
} GfxBlight;



void GFX_init(GfxFbFmt fmtTop, GfxFbFmt fmtBot);

static inline void GFX_initDefault(void)
{
	GFX_init(GFX_BGR8, GFX_BGR8);
}

void GFX_deinit(void);

void GFX_setFramebufFmt(GfxFbFmt fmtTop, GfxFbFmt fmtBot);

void GFX_powerOnBacklights(GfxBlight mask);

void GFX_powerOffBacklights(GfxBlight mask);

void GFX_setBrightness(u8 top, u8 bot);

void GFX_setForceBlack(bool top, bool bot);

void GFX_setDoubleBuffering(u8 screen, bool dBuf);

void* GFX_getFramebuffer(u8 screen);

void GFX_swapFramebufs(void);

void GFX_waitForEvent(GfxEvent event, bool discard);

// Helpers
#define GFX_waitForPSC0()     GFX_waitForEvent(GFX_EVENT_PSC0, false)
#define GFX_waitForPSC1()     GFX_waitForEvent(GFX_EVENT_PSC1, false)
#define GFX_waitForVBlank0()  GFX_waitForEvent(GFX_EVENT_PDC0, true)
//#define GFX_waitForVBlank1()  GFX_waitForEvent(GFX_EVENT_PDC1, true) // Disabled
#define GFX_waitForPPF()      GFX_waitForEvent(GFX_EVENT_PPF, false)
#define GFX_waitForP3D()      GFX_waitForEvent(GFX_EVENT_P3D, false)

void GX_memoryFill(u32 *buf0a, u32 buf0v, u32 buf0Sz, u32 val0, u32 *buf1a, u32 buf1v, u32 buf1Sz, u32 val1);

void GX_displayTransfer(const u32 *const in, u32 indim, u32 *out, u32 outdim, u32 flags);

void GX_textureCopy(const u32 *const in, u32 indim, u32 *out, u32 outdim, u32 size);

void GX_processCommandList(u32 size, const u32 *const cmdList);

//void GFX_enterLowPowerState(void);

//void GFX_returnFromLowPowerState(void);

#endif
