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

#include "types.h"
#include "arm11/drivers/lcd.h"
#include "arm11/drivers/i2c.h"
#include "arm11/drivers/timer.h"


#define LCD_BACKLIGHT_TIMEOUT (10u)



u8 LCDI2C_readReg(u8 lcd, LcdI2cReg reg)
{
	u8 buf[2];
	const u8 dev = (lcd == 0 ? I2C_DEV_LCD0 : I2C_DEV_LCD1);

	I2C_writeReg(dev, LCD_I2C_REG_READ_ADDR, reg);
	I2C_readRegBuf(dev, LCD_I2C_REG_READ_ADDR, buf, 2);

	return buf[1];
}

void LCDI2C_writeReg(u8 lcd, LcdI2cReg reg, u8 data)
{
	const u8 dev = (lcd == 0 ? I2C_DEV_LCD0 : I2C_DEV_LCD1);

	I2C_writeReg(dev, reg, data);
}

void LCDI2C_init(void)
{
	const u16 revs = LCDI2C_getRevisions();

	// Top screen
	if(revs & 0xFFu) LCDI2C_writeReg(0, LCD_I2C_REG_RST_STATUS, LCD_REG_RST_STATUS_NONE);
	else
	{
		LCDI2C_writeReg(0, LCD_I2C_REG_UNK11, LCD_REG_UNK11_UNK10);
		LCDI2C_writeReg(0, LCD_I2C_REG_HS_SERIAL, LCD_REG_HS_SERIAL_ON);
	}

	// Bottom screen
	if(revs>>8) LCDI2C_writeReg(1, LCD_I2C_REG_RST_STATUS, LCD_REG_RST_STATUS_NONE);
	else        LCDI2C_writeReg(1, LCD_I2C_REG_UNK11, LCD_REG_UNK11_UNK10);

	LCDI2C_writeReg(0, LCD_I2C_REG_STATUS, LCD_REG_STATUS_OK); // Initialize status flag.
	LCDI2C_writeReg(1, LCD_I2C_REG_STATUS, LCD_REG_STATUS_OK); // Initialize status flag.
	LCDI2C_writeReg(0, LCD_I2C_REG_POWER, LCD_REG_POWER_ON);   // Power on LCD.
	LCDI2C_writeReg(1, LCD_I2C_REG_POWER, LCD_REG_POWER_ON);   // Power on LCD.
}

void LCDI2C_waitBacklightsOn(void)
{
	const u16 revs = LCDI2C_getRevisions();

	if((revs & 0xFFu) == 0 || (revs>>8) == 0)
	{
		// Bug workaround for early LCD driver revisions?
		TIMER_sleepMs(150);
	}
	else
	{
		u32 i = 0;
		do
		{
			const u8 top = LCDI2C_readReg(0, LCD_I2C_REG_BL_STATUS);
			const u8 bot = LCDI2C_readReg(1, LCD_I2C_REG_BL_STATUS);

			if(top == LCD_REG_BL_STATUS_ON && bot == LCD_REG_BL_STATUS_ON) break;

			TIMER_sleepTicks(TIMER_FREQ(1, 1000) * 33.333f);
		} while(++i < LCD_BACKLIGHT_TIMEOUT);
	}
}

u16 LCDI2C_getRevisions(void)
{
	static bool lcdRevsRead = false;
	static u16 lcdRevs;

	if(!lcdRevsRead)
	{
		lcdRevsRead = true;

		lcdRevs = LCDI2C_readReg(0, LCD_I2C_REG_REVISION) |
		          LCDI2C_readReg(1, LCD_I2C_REG_REVISION)<<8;
	}

	return lcdRevs;
}
