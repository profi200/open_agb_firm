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
#include "arm11/config.h"
#include "drivers/gfx.h"
#include "arm11/drivers/codec.h"
#include "arm11/drivers/gpio.h"
#include "arm11/drivers/hid.h"
#include "arm11/drivers/interrupt.h"
#include "arm11/drivers/lgycap.h"
#include "arm11/drivers/lgy11.h"
#include "arm11/drivers/mcu.h"
#include "arm11/console.h"
#include "arm11/power.h"



static void handleLidSleep(void)
{
	CODEC_setVolumeOverride(-128);
	LGYCAP_stop(LGYCAP_DEV_TOP);
	GFX_powerOffBacklight(GFX_BL_BOTH);
	MCU_setPowerLedPattern(MCU_PWR_LED_SLEEP);

	while(GPIO_read(GPIO_1_SHELL))
		__wfi();

	MCU_setPowerLedPattern(MCU_PWR_LED_AUTO);
	GFX_powerOnBacklight(GFX_BL_BOTH);
	LGYCAP_start(LGYCAP_DEV_TOP);
	CODEC_setVolumeOverride(g_oafConfig.volume);

	REG_HID_PADCNT = 0;
	getLgy11Regs()->sleep |= BIT(0);
}

int main(void)
{
	Result res = oafParseConfigEarly();
	GFX_init(GFX_BGR8, GFX_BGR565, GFX_TOP_2D);
	changeBacklight(0); // Apply backlight config.
	consoleInit(GFX_LCD_BOT, NULL);
	//CODEC_init();

	if(res == RES_OK && (res = oafInitAndRun()) == RES_OK)
	{
		while(1)
		{
			hidScanInput();
			const u32 extraKeys = hidGetExtraKeys(KEY_SHELL);
			if(extraKeys & (KEY_POWER_HELD | KEY_POWER)) break;
			if(extraKeys & KEY_SHELL) handleLidSleep();

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