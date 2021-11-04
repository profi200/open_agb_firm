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

#include "fs.h"
#include "drivers/gfx.h"
#include "arm11/console.h"
#include "arm11/open_agb_firm.h"
#include "arm11/drivers/hid.h"
#include "arm11/drivers/codec.h"
#include "arm11/power.h"



int main(void)
{
	GFX_init(GFX_BGR8, GFX_RGB565);
	consoleInit(SCREEN_BOT, NULL);
	//CODEC_init();

	Result res = fMount(FS_DRIVE_SDMC);
	if(res == RES_OK && (res = oafInitAndRun()) == RES_OK)
	{
		do
		{
			hidScanInput();
			if(hidGetExtraKeys(0) & (KEY_POWER_HELD | KEY_POWER)) break;

			oafUpdate();
		} while(1);

		oafFinish();
	}
	else printErrorWaitInput(res, 0);

	CODEC_deinit();
	GFX_deinit();
	fUnmount(FS_DRIVE_SDMC);

	power_off();

	return 0;
}
