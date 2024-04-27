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

#include "oaf_error_codes.h"
#include "fs.h"
#include "arm11/open_agb_firm.h"
#include "drivers/gfx.h"
#include "arm11/drivers/mcu.h"
#include "arm11/console.h"
#include "arm11/drivers/codec.h"
#include "arm11/drivers/hid.h"
#include "arm11/power.h"



int main(void)
{
	Result res = oafParseConfigEarly();
	GFX_init(GFX_BGR8, GFX_R5G6B5, GFX_TOP_2D);
	changeBacklight(0); // Apply backlight config.
	consoleInit(GFX_LCD_BOT, NULL);
	//CODEC_init();

	if(res == RES_OK && (res = oafInitAndRun()) == RES_OK)
	{
		while(1)
		{
			hidScanInput();
			if(hidGetExtraKeys(0) & (KEY_POWER_HELD | KEY_POWER)) break;
			if(hidGetExtraKeys(0) & KEY_SHELL) oafSleep();
			if(!(hidGetExtraKeys(0) & KEY_SHELL)) oafWakeup();

			oafUpdate();
		}

		oafFinish();
	}
	else printErrorWaitInput(res, 0);

	CODEC_deinit();
	GFX_deinit();
	fUnmount(FS_DRIVE_SDMC); // TODO: Move elsewhere. __systemDeinit() already calls it.

	power_off();

	return 0;
}