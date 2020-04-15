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
 *  Based on code from https://github.com/smealum/ctrulib
 */

#include "types.h"
#include "mem_map.h"
#include "arm11/hardware/hid.h"
#include "arm11/hardware/mcu.h"
#include "arm11/hardware/interrupt.h"
#include "arm11/hardware/gpio.h"
#include "arm11/hardware/codec.h"


#define CPAD_THRESHOLD  (400)


static u32 kHeld, kDown, kUp;
static u32 extraKeys;
//TouchPos tPos;
//CpadPos cPos;
static volatile bool mcuIrq;



static void hidIrqHandler(UNUSED u32 intSource);

void hidInit(void)
{
	kUp = kDown = kHeld = 0;
	mcuIrq = false;

	(void)MCU_readReceivedIrqs();
	u8 state = MCU_readExternalHwState();
	u32 tmp = ~state<<3 & KEY_SHELL;      // Current shell state. Bit is inverted.
	tmp |= state<<1 & KEY_BAT_CHARGING;   // Current battery charging state
	state = MCU_readHidHeld();
	tmp |= ~state<<1 & KEY_HOME;          // Current HOME button state
	extraKeys = tmp;

	IRQ_registerHandler(IRQ_CTR_MCU, 14, 0, true, hidIrqHandler);
	MCU_setIrqBitmask(0xFFBF3F80u);
	// Configure GPIO for MCU event IRQs
	GPIO_config(GPIO_4_MCU, GPIO_INPUT | GPIO_EDGE_FALLING | GPIO_IRQ_ENABLE);

	//CODEC_init();
}

static void hidIrqHandler(UNUSED u32 intSource)
{
	mcuIrq = true;
}

static void updateMcuIrqState(void)
{
	if(!mcuIrq) return;
	mcuIrq = false;

	const u32 state = MCU_readReceivedIrqs();

	u32 tmp = extraKeys;
	tmp |= state & (KEY_POWER | KEY_POWER_HELD | KEY_HOME); // Power button pressed/held, HOME button pressed
	if(state & 1u<<3) tmp &= ~KEY_HOME;                     // HOME released
	tmp |= state>>1 & (KEY_WIFI | KEY_SHELL);               // WiFi switch, shell closed
	if(state & 1u<<6) tmp &= ~KEY_SHELL;                    // Shell opened
	tmp |= state>>10 & KEY_BAT_CHARGING;                    // Battery started charging
	if(state & 1u<<14) tmp &= ~KEY_BAT_CHARGING;            // Battery stopped charging
	tmp |= state>>16 & KEY_VOL_SLIDER;                      // Volume slider update
	extraKeys = tmp;
}

/*static u32 rawCodec2Hid(void)
{
	alignas(4) u8 buf[13 * 4];

	CODEC_getRawAdcData((u32*)buf);

	// Touchscreen
	u32 emuButtons = !(buf[0] & 1u<<4)<<20; // KEY_TOUCH
	tPos.x = (buf[0]<<8 | buf[1]) * 320u / 4096u; // TODO: Calibration
	tPos.y = (buf[10]<<8 | buf[11]) * 240u / 4096u;

	// Circle-Pad
	cPos.x = -(((buf[36]<<8 | buf[37]) & 0xFFFu) - 2048u); // X axis is inverted
	cPos.y = ((buf[20]<<8 | buf[21]) & 0xFFFu) - 2048u;

	if((cPos.x >= 0 ? cPos.x : -cPos.x) > CPAD_THRESHOLD)
	{
		if(cPos.x >= 0) emuButtons |= KEY_CPAD_RIGHT;
		else            emuButtons |= KEY_CPAD_LEFT;
	}
	if((cPos.y >= 0 ? cPos.y : -cPos.y) > CPAD_THRESHOLD)
	{
		if(cPos.y >= 0) emuButtons |= KEY_CPAD_UP;
		else            emuButtons |= KEY_CPAD_DOWN;
	}

	return emuButtons;
}*/

void hidScanInput(void)
{
	updateMcuIrqState();

	const u32 kOld = kHeld;
	kHeld = /*rawCodec2Hid() |*/ REG_HID_PAD;
	kDown = (~kOld) & kHeld;
	kUp = kOld & (~kHeld);
}

u32 hidKeysHeld(void)
{
	return kHeld;
}

u32 hidKeysDown(void)
{
	return kDown;
}

u32 hidKeysUp(void)
{
	return kUp;
}

/*const TouchPos* hidGetTouchPosPtr(void)
{
	return &tPos;
}

const CpadPos* hidGetCpadPosPtr(void)
{
	return &cPos;
}*/

u32 hidGetExtraKeys(u32 clearMask)
{
	const u32 tmp = extraKeys;
	extraKeys &= ~clearMask;

	return tmp;
}
