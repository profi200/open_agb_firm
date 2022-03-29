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

/*
 *  Based on code from https://github.com/smealum/ctrulib
 */

#include "types.h"
#include "mem_map.h"
#include "arm11/drivers/hid.h"
#include "arm11/drivers/mcu.h"
#include "arm11/drivers/interrupt.h"
#include "arm11/drivers/gpio.h"
#include "arm11/drivers/codec.h"


#define MCU_HID_IRQ_MASK  (MCU_IRQ_VOL_SLIDER_CHANGE | MCU_IRQ_BATT_CHARGE_START | \
                           MCU_IRQ_BATT_CHARGE_STOP | MCU_IRQ_SHELL_OPEN | \
                           MCU_IRQ_SHELL_CLOSE | MCU_IRQ_WIFI_PRESS | \
                           MCU_IRQ_HOME_RELEASE | MCU_IRQ_HOME_PRESS | \
                           MCU_IRQ_POWER_HELD | MCU_IRQ_POWER_PRESS)

#define CPAD_THRESHOLD  (400)


static u32 g_kHeld = 0, g_kDown = 0, g_kUp = 0;
static u32 g_extraKeys = 0;
TouchPos g_tPos = {0};
CpadPos g_cPos = {0};



void hidInit(void)
{
	static bool inited = false;
	if(inited) return;
	inited = true;

	MCU_init();
	u16 state = MCU_getExternalHardwareStatus();
	u32 tmp = ~state<<3 & KEY_SHELL;      // Current shell state. Bit is inverted.
	tmp |= state<<1 & KEY_BAT_CHARGING;   // Current battery charging state
	state = MCU_getEarlyButtonsHeld();
	tmp |= ~state<<1 & KEY_HOME;          // Current HOME button state
	g_extraKeys = tmp;

	CODEC_init();
}

static void updateMcuHidState(void)
{
	const u32 state = MCU_getIrqs(MCU_HID_IRQ_MASK);
	if(state == 0) return;

	u32 tmp = g_extraKeys;
	tmp |= state & (KEY_POWER | KEY_POWER_HELD | KEY_HOME); // Power button pressed/held, HOME button pressed
	if(state & 1u<<3) tmp &= ~KEY_HOME;                     // HOME released
	tmp |= state>>1 & (KEY_WIFI | KEY_SHELL);               // WiFi switch, shell closed
	if(state & 1u<<6) tmp &= ~KEY_SHELL;                    // Shell opened
	tmp |= state>>10 & KEY_BAT_CHARGING;                    // Battery started charging
	if(state & 1u<<14) tmp &= ~KEY_BAT_CHARGING;            // Battery stopped charging
	tmp |= state>>16 & KEY_VOL_SLIDER;                      // Volume slider update
	g_extraKeys = tmp;
}

static u32 rawCodec2Hid(void)
{
	static u32 fakeKeysCache = 0;
	alignas(4) CdcAdcData adc;
	if(!CODEC_getRawAdcData(&adc)) return fakeKeysCache;

	// Touchscreen
	// TODO: Calibration
	const u16 tx = __builtin_bswap16(adc.touchX[0]);
	u32 fakeKeys = (~tx & 1u<<12)<<8; // KEY_TOUCH
	g_tPos.x = tx * 320u / 4096u;
	g_tPos.y = __builtin_bswap16(adc.touchY[0]) * 240u / 4096u;

	// Circle-Pad
	// TODO: Calibration
	g_cPos.y = (__builtin_bswap16(adc.cpadY[0]) & 0xFFFu) - 2048u;
	g_cPos.x = -((__builtin_bswap16(adc.cpadX[0]) & 0xFFFu) - 2048u); // X axis is inverted.

	if((g_cPos.x >= 0 ? g_cPos.x : -g_cPos.x) > CPAD_THRESHOLD)
	{
		if(g_cPos.x >= 0) fakeKeys |= KEY_CPAD_RIGHT;
		else              fakeKeys |= KEY_CPAD_LEFT;
	}
	if((g_cPos.y >= 0 ? g_cPos.y : -g_cPos.y) > CPAD_THRESHOLD)
	{
		if(g_cPos.y >= 0) fakeKeys |= KEY_CPAD_UP;
		else              fakeKeys |= KEY_CPAD_DOWN;
	}

	fakeKeysCache = fakeKeys;
	return fakeKeys;
}

void hidScanInput(void)
{
	updateMcuHidState();

	const u32 kOld = g_kHeld;
	g_kHeld = rawCodec2Hid() | REG_HID_PAD;
	g_kDown = (~kOld) & g_kHeld;
	g_kUp = kOld & (~g_kHeld);
}

u32 hidKeysHeld(void)
{
	return g_kHeld;
}

u32 hidKeysDown(void)
{
	return g_kDown;
}

u32 hidKeysUp(void)
{
	return g_kUp;
}

const TouchPos* hidGetTouchPosPtr(void)
{
	return &g_tPos;
}

const CpadPos* hidGetCpadPosPtr(void)
{
	return &g_cPos;
}

u32 hidGetExtraKeys(u32 clearMask)
{
	const u32 tmp = g_extraKeys;
	g_extraKeys &= ~clearMask;

	return tmp;
}
