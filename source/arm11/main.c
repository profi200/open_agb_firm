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

#include "types.h"
#include "arm11/hardware/hid.h"
#include "arm11/hardware/codec.h"
#include "arm11/hardware/lgy.h"
#include "arm11/console.h"
#include "arm11/fmt.h"
#include "arm11/power.h"
#include "hardware/gfx.h"
#include "arm.h"
 #include "arm11/hardware/i2c.h"



void clearScreens(void)
{
	GX_memoryFill((u64*)RENDERBUF_TOP, 1u<<9, SCREEN_SIZE_TOP, 0, (u64*)RENDERBUF_BOT, 1u<<9, SCREEN_SIZE_BOT, 0);
	GFX_waitForEvent(GFX_EVENT_PSC0, true);
}

void updateScreens(void)
{
	GX_textureCopy((u64*)RENDERBUF_TOP, 0, (u64*)GFX_getFramebuffer(SCREEN_TOP),
				   0, SCREEN_SIZE_TOP + SCREEN_SIZE_BOT);
	GFX_waitForEvent(GFX_EVENT_PPF, true); // Texture copy
	GFX_swapFramebufs();
	GFX_waitForEvent(GFX_EVENT_PDC0, true); // VBlank
}

int main(void)
{
	GFX_init();
	consoleInit(SCREEN_TOP, NULL, false);
	CODEC_init();

	ee_puts("Prepare legacy mode...");
	updateScreens();
	LGY_prepareLegacyMode();

	clearScreens();
	updateScreens();
	updateScreens(); // Clear both framebuffers.
	LGY_switchMode();

	do
	{
		__wfi();

		LGY_handleEvents();

		hidScanInput();
		if(hidGetExtraKeys(KEY_POWER) & KEY_POWER) break;
	} while(1);

	LGY_deinit();
	CODEC_deinit();
	GFX_deinit(false);
	power_off();

	return 0;
}
