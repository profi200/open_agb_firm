#pragma once

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


#define MCU_HID_POWER_BUTTON_PRESSED       (1u)
#define MCU_HID_POWER_BUTTON_LONG_PRESSED  (1u<<1)
#define MCU_HID_HOME_BUTTON_PRESSED        (1u<<2)
#define MCU_HID_HOME_BUTTON_RELEASED       (1u<<3)
#define MCU_HID_HOME_BUTTON_NOT_HELD       (1u<<1)
#define MCU_HID_SHELL_GOT_CLOSED           (1u<<5)
#define MCU_HID_SHELL_GOT_OPENED           (1u<<6)

typedef enum
{
	PWLED_NORMAL    = 0u,
	PWLED_SLEEP     = 2u,
	PWLED_OFF       = 3u,
	PWLED_RED       = 4u,
	PWLED_BLUE      = 5u,
	PWLED_BLINK_RED = 6u
} PwLedState;



void MCU_init(void);
void MCU_disableLEDs(void);
void MCU_powerOnLCDs(void);
void MCU_powerOffLCDs(void);
void MCU_triggerPowerOff(void);
void MCU_triggerReboot(void);
u8 MCU_readBatteryLevel(void);
u8 MCU_readExternalHwState(void);
u8 MCU_readSystemModel(void);
void MCU_readRTC(void *rtc);
u32 MCU_readReceivedIrqs(void);
bool MCU_setIrqBitmask(u32 mask);
u8 MCU_readHidHeld(void);
bool MCU_powerOnLcdBacklights(void);
bool MCU_setPowerLedState(PwLedState state);
