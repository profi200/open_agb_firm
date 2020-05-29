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
#include "arm11/hardware/i2c.h"


typedef enum
{
	MCU_REG_VERS_HIGH    = 0x00u,
	MCU_REG_VERS_LOW     = 0x01u,
	MCU_REG_3D_SLIDER    = 0x08u,
	MCU_REG_VOL_SLIDER   = 0x09u, // 0-0x3F
	MCU_REG_BATTERY      = 0x0Bu,
	MCU_REG_EX_HW_STATE  = 0x0Fu,
	MCU_REG_EVENTS       = 0x10u,
	MCU_REG_EVENT_MASK   = 0x18u,
	MCU_REG_POWER        = 0x20u,
	MCU_REG_LCDs         = 0x22u,
	MCU_REG_POWER_LED    = 0x29u,
	MCU_REG_WIFI_LED     = 0x2Au,
	MCU_REG_CAM_LED      = 0x2Bu,
	MCU_REG_3D_LED       = 0x2Cu,
	MCU_REG_RTC_TIME     = 0x30u,
	MCU_REG_RAW_STATE    = 0x7Fu
} McuReg;

typedef enum
{
	PWLED_AUTO      = 0u,
	//PWLED_BLUE      = 1u, // wtf is "forced default blue"?
	PWLED_SLEEP     = 2u,
	PWLED_OFF       = 3u,
	PWLED_RED       = 4u,
	PWLED_BLUE      = 5u,
	PWLED_BLINK_RED = 6u
} PwLedState;



void MCU_init(void);

bool MCU_setEventMask(u32 mask);

u32 MCU_getEvents(u32 mask);

u32 MCU_waitEvents(u32 mask);

u8 MCU_readReg(McuReg reg);

bool MCU_writeReg(McuReg reg, u8 data);

bool MCU_readRegBuf(McuReg reg, u8 *out, u32 size);

bool MCU_writeRegBuf(McuReg reg, const u8 *const in, u32 size);


static inline u8 MCU_getBatteryLevel(void)
{
	u8 state;

	if(!MCU_readRegBuf(MCU_REG_BATTERY, &state, 1)) return 0;
	
	return state;
}

static inline u8 MCU_getExternalHwState(void)
{
	return MCU_readReg(MCU_REG_EX_HW_STATE);
}

static inline void MCU_powerOffSys(void)
{
	I2C_writeRegIntSafe(I2C_DEV_CTR_MCU, MCU_REG_POWER, 1u);
}

static inline void MCU_rebootSys(void)
{
	I2C_writeRegIntSafe(I2C_DEV_CTR_MCU, MCU_REG_POWER, 1u<<2);
}

static inline void MCU_controlLCDPower(u8 bits)
{
	MCU_writeReg(MCU_REG_LCDs, bits);
}

static inline bool MCU_setPowerLedState(PwLedState state)
{
	return MCU_writeReg(MCU_REG_POWER_LED, state);
}

static inline bool MCU_getRTCTime(u8 rtc[7])
{
	if(!rtc) return true;

	return MCU_readRegBuf(MCU_REG_RTC_TIME, rtc, 7);
}

static inline u8 MCU_getSystemModel(void)
{
	u8 buf[10];

	if(!MCU_readRegBuf(MCU_REG_RAW_STATE, buf, sizeof(buf))) return 0xFF;
	
	return buf[9];
}

static inline u8 MCU_getHidHeld(void)
{
	u8 data[19];

	if(!MCU_readRegBuf(MCU_REG_RAW_STATE, data, sizeof(data))) return 0xFF;

	return data[18];
}
