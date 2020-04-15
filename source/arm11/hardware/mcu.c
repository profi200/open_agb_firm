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

#include "mem_map.h"
#include "arm11/hardware/i2c.h"
#include "arm11/hardware/mcu.h"


enum McuRegisters {
	RegBattery  = 0x0Bu,
	RegExHW     = 0x0Fu,
	RegPower    = 0x20u,
	RegLCDs     = 0x22u,
	RegWifiLED  = 0x2Au,
	RegCamLED   = 0x2Bu,
	Reg3DLED    = 0x2Cu,
	RegRTC      = 0x30u,
	RegSysModel = 0x7Fu
};



void MCU_init(void)
{
	I2C_init();
}

void MCU_disableLEDs(void)
{
	// disable wifi LED
	I2C_writeReg(I2C_DEV_CTR_MCU, RegWifiLED, 0);
	
	// disable 3D LED
	I2C_writeReg(I2C_DEV_CTR_MCU, Reg3DLED, 0);
	
	// disable camera LED
	I2C_writeReg(I2C_DEV_CTR_MCU, RegCamLED, 0);
}

void MCU_powerOnLCDs(void)
{
	// bit1 = lcd power enable for both screens
	I2C_writeReg(I2C_DEV_CTR_MCU, RegLCDs, 1<<1);
}

void MCU_powerOffLCDs(void)
{
	// bit0 = lcd power disable for both screens (also disables backlight)
	I2C_writeReg(I2C_DEV_CTR_MCU, RegLCDs, 1);
}

void MCU_triggerPowerOff(void)
{
	I2C_writeRegIntSafe(I2C_DEV_CTR_MCU, RegPower, 1);
}

void MCU_triggerReboot(void)
{
	I2C_writeRegIntSafe(I2C_DEV_CTR_MCU, RegPower, 1u << 2);
}

u8 MCU_readBatteryLevel(void)
{
	u8 state;

	if(!I2C_readRegBuf(I2C_DEV_CTR_MCU, RegBattery, &state, 1))
		return 0;
	
	return state;
}

u8 MCU_readExternalHwState(void)
{
	return I2C_readReg(I2C_DEV_CTR_MCU, RegExHW);
}

u8 MCU_readSystemModel(void)
{
	u8 sysinfo[0x13];

	if(!I2C_readRegBuf(I2C_DEV_CTR_MCU, RegSysModel, sysinfo, sizeof sysinfo))
		return 0xFF;
	
	return sysinfo[9];
}

void MCU_readRTC(void *rtc)
{
	if(!rtc) return;
	
	I2C_readRegBuf(I2C_DEV_CTR_MCU, RegRTC, rtc, 8);
}

u32 MCU_readReceivedIrqs(void)
{
	u32 data;
	if(!I2C_readRegBuf(I2C_DEV_CTR_MCU, 0x10, (u8*)&data, 4)) return 0;
	return data;
}

bool MCU_setIrqBitmask(u32 mask)
{
	return I2C_writeRegBuf(I2C_DEV_CTR_MCU, 0x18, (const u8*)&mask, 4);
}

u8 MCU_readHidHeld(void)
{
	u8 data[19];
	if(!I2C_readRegBuf(I2C_DEV_CTR_MCU, 0x7F, data, sizeof(data))) return 0xFF;
	return data[18];
}

bool MCU_powerOnLcdBacklights(void)
{
	return I2C_writeReg(I2C_DEV_CTR_MCU, 0x22, 1<<5 | 1<<3); // bit3 = lower screen, bit5 = upper
}

bool MCU_setPowerLedState(PwLedState state)
{
	return I2C_writeReg(I2C_DEV_CTR_MCU, 0x29, state);
}
