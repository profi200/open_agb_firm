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

#include <stdlib.h>
#include "types.h"
#include "arm11/hardware/hid.h"
#include "arm11/hardware/codec.h"
#include "hardware/lgy.h"
#include "arm11/console.h"
#include "arm11/fmt.h"
#include "arm11/power.h"
#include "hardware/gfx.h"
#include "fs.h"
#include "arm11/filebrowser.h"
#include "arm.h"



int main(void)
{
	GFX_init(GFX_BGR8, GFX_RGB565);
	GFX_setBrightness(DEFAULT_BRIGHTNESS, DEFAULT_BRIGHTNESS);
	consoleInit(SCREEN_BOT, NULL);
	//CODEC_init();

	Result res;
	char *romPath = (char*)malloc(512);
	*romPath = '\0';
	if((res = fMount(FS_DRIVE_SDMC)) != RES_OK || (res = browseFiles("sdmc:/", romPath)) != RES_OK || *romPath == '\0')
		goto end;

	ee_puts("Reading ROM and save...");
	if((res = LGY_prepareGbaMode(false, romPath)) == RES_OK)
	{
#ifdef NDEBUG
		GFX_setForceBlack(false, true);
		GFX_powerOffBacklights(GFX_BLIGHT_BOT);
#endif
		// Sync LgyFb start with LCD VBlank.
		GFX_waitForVBlank0();
		LGY_switchMode();

		do
		{
			hidScanInput();
			if(hidGetExtraKeys(KEY_POWER) & KEY_POWER) break;

			LGY_handleEvents();

			__wfi();
		} while(1);
	}

end:
	free(romPath);
	if(res != RES_OK) printErrorWaitInput(res, 0);

	LGY_deinit();
	fUnmount(FS_DRIVE_SDMC);
	CODEC_deinit();
	GFX_deinit();
	power_off();

	return 0;
}
