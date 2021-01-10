/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2021 Sergi Granell (xerpi), Paul LaMendola (paulguy), derrek, profi200
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

// Based on code from https://github.com/xerpi/linux_3ds/blob/master/drivers/input/misc/nintendo3ds_codec_hid.c

#include "types.h"
#include "arm11/hardware/codec.h"
#include "arm11/hardware/spi.h"
#include "arm11/hardware/pdn.h"
#include "arm11/hardware/timer.h"
#include "arm11/hardware/gpio.h"


typedef struct
{
	u8 driverGainHP;
	u8 driverGainSP;
	u8 analogVolumeHP;
	u8 analogVolumeSP;
	s8 shutterVolume[2];
	u8 microphoneBias;
	u8 quickCharge;
	u8 PGA_GAIN; // microphone gain
	u8 reserved[3];
	s16 filterHP32[15]; // 3 * 5
	s16 filterHP47[15];
	s16 filterSP32[15];
	s16 filterSP47[15];
	s16 filterMic32[28]; // (1+2)+((1+4)*5)
	s16 filterMic47[28];
	s16 filterFree[28];
	u8 analogInterval;
	u8 analogStabilize;
	u8 analogPrecharge;
	u8 analogSense;
	u8 analogDebounce;
	u8 analog_XP_Pullup;
	u8 YM_Driver;
	u8 reserved2;
} CodecCal;


alignas(4) static CodecCal fallbackCal =
{
	0u,
	1u,
	0u,
	7u,
	{0xFD, 0xEC},
	3u,
	2u,
	0u,
	{0, 0, 0},
	{32767, 0, 0, 0, 0, 32767, 0, 0, 0, 0, 32736, 49168, 0, 16352, 0},
	{32767, 0, 0, 0, 0, 32767, 0, 0, 0, 0, 32745, 49164, 0, 16361, 0},
	{32767, 38001, 22413, 30870, 36440, 51536, 30000, 51536, 0, 0, 32736, 49168, 0, 16352, 0},
	{32767, 36541, 25277, 31456, 35336, 51134, 30000, 51134, 0, 0, 32745, 49164, 0, 16361, 0},
	{32767, 0, 0, 32767, 0, 0, 0, 0, 32767, 0, 0, 0, 0, 32767, 0, 0, 0, 0, 32767, 0, 0, 0, 0, 32767, 0, 0, 0, 0},
	{32767, 0, 0, 32767, 0, 0, 0, 0, 32767, 0, 0, 0, 0, 32767, 0, 0, 0, 0, 32767, 0, 0, 0, 0, 32767, 0, 0, 0, 0},
	{32767, 0, 0, 52577, 56751, 32767, 8785, 12959, 52577, 56751, 32767, 8785, 12959, 52577, 56751, 32767, 8785, 12959, 32767, 0, 0, 0, 0, 32767, 0, 0, 0, 0},
	1u,
	9u,
	4u,
	3u,
	0u,
	6u,
	1u,
	0u
};



static void codecSwitchBank(u8 bank)
{
	static u8 curBank = 0x63;
	if(curBank != bank)
	{
		curBank = bank;

		alignas(4) u8 inBuf[4];
		inBuf[0] = 0; // Write
		inBuf[1] = bank;
		NSPI_writeRead(NSPI_DEV_CTR_CODEC, (u32*)inBuf, NULL, 2, 0, true);
	}
}

static void codecReadRegBuf(u8 bank, u8 reg, u32 *buf, u32 size)
{
	codecSwitchBank(bank);

	alignas(4) u8 inBuf[4];
	inBuf[0] = reg<<1 | 1u;
	NSPI_writeRead(NSPI_DEV_CTR_CODEC, (u32*)inBuf, buf, 1, size, true);
}

static u8 codecReadReg(u8 bank, u8 reg)
{
	alignas(4) u8 outBuf[4];
	codecReadRegBuf(bank, reg, (u32*)outBuf, 1);

	return outBuf[0];
}

static void codecWriteRegBuf(u8 bank, u8 reg, u32 *buf, u32 size)
{
	codecSwitchBank(bank);

	alignas(4) u8 inBuf[4];
	inBuf[0] = reg<<1; // Write
	NSPI_writeRead(NSPI_DEV_CTR_CODEC, (u32*)inBuf, NULL, 1, 0, false);
	NSPI_writeRead(NSPI_DEV_CTR_CODEC, buf, NULL, size, 0, true);
}

static void codecWriteReg(u8 bank, u8 reg, u8 val)
{
	codecSwitchBank(bank);

	alignas(4) u8 inBuf[4];
	inBuf[0] = reg<<1; // Write
	inBuf[1] = val;
	NSPI_writeRead(NSPI_DEV_CTR_CODEC, (u32*)inBuf, NULL, 2, 0, true);
}

static void codecMaskReg(u8 bank, u8 reg, u8 val, u8 mask)
{
	u8 data = codecReadReg(bank, reg);
	data = (data & ~mask) | (val & mask);
	codecWriteReg(bank, reg, data);
}

// Helpers
static void codecSwapCalibrationData(CodecCal *cal)
{
	u16 *tmp = (u16*)cal->filterHP32;
	for(int i = 0; i < 15; i++)
	{
		tmp[i] = __builtin_bswap16(tmp[i]);
	}

	tmp = (u16*)cal->filterHP47;
	for(int i = 0; i < 15; i++)
	{
		tmp[i] = __builtin_bswap16(tmp[i]);
	}

	tmp = (u16*)cal->filterSP32;
	for(int i = 0; i < 15; i++)
	{
		tmp[i] = __builtin_bswap16(tmp[i]);
	}

	tmp = (u16*)cal->filterSP47;
	for(int i = 0; i < 15; i++)
	{
		tmp[i] = __builtin_bswap16(tmp[i]);
	}

	tmp = (u16*)cal->filterMic32;
	for(int i = 0; i < 28; i++)
	{
		tmp[i] = __builtin_bswap16(tmp[i]);
	}

	tmp = (u16*)cal->filterMic47;
	for(int i = 0; i < 28; i++)
	{
		tmp[i] = __builtin_bswap16(tmp[i]);
	}

	tmp = (u16*)cal->filterFree;
	for(int i = 0; i < 28; i++)
	{
		tmp[i] = __builtin_bswap16(tmp[i]);
	}
}

static void codecMaskWaitReg(u8 bank, u8 reg, u8 val, u8 mask)
{
	for(u32 i = 0; i < 64; i++) // Some kind of timeout? No error checking.
	{
		codecMaskReg(bank, reg, val, mask);
		if((codecReadReg(bank, reg) & mask) == val) break;
	}
}

static void codecEnableTouchscreen(void)
{
	codecMaskReg(0x67, 0x26, 0x80, 0x80);
	codecMaskReg(0x67, 0x24, 0, 0x80);
	codecMaskReg(0x67, 0x25, 0x10, 0x3C);
}

static void codecDisableTouchscreen(void)
{
	codecMaskReg(0x67, 0x26, 0, 0x80);
	codecMaskReg(0x67, 0x24, 0x80, 0x80);
}

static void codecLegacyStuff(bool enabled)
{
	if(enabled)
	{
		*((vu16*)0x10141114) |= 2u;
		*((vu16*)0x10141116) |= 2u;
		codecMaskReg(0x67, 0x25, 0x40, 0x40);
	}
	else
	{
		codecMaskReg(0x67, 0x25, 0, 0x40);
		*((vu16*)0x10141114) &= ~2u;
	}
}


void CODEC_init(void)
{
	static bool inited = false;
	if(inited) return;
	inited = true;


	NSPI_init();

	// TODO: Load calibration from HWCAL files on eMMC.
	CodecCal *const cal = &fallbackCal;
	codecSwapCalibrationData(cal); // Come the fuck on. Why is this not stored in the correct endianness?

	// General codec reset + init?
	REG_PDN_I2S_CNT = PDN_I2S_CNT_I2S_CLK2_E;
	codecWriteReg(0x64, 1, 1);
	TIMER_sleepMs(40);
	codecSwitchBank(0); // What? Dummy switch after reset?
	codecWriteReg(0x64, 0x43, 0x11);
	codecMaskReg(0x65, 0x77, 1, 1);
	codecMaskReg(0, 0x39, 0x66, 0x66);
	codecWriteReg(0x65, 0x7A, 1);
	codecMaskReg(0x64, 0x22, 0x18, 0x18);
	GPIO_config(GPIO_2_HEADPH_JACK, GPIO_IRQ_ENABLE | GPIO_EDGE_RISING | GPIO_INPUT); // Headphone jack IRQ.
	//codecMaskReg(0x64, 0x45, (*((vu8*)0x10147010) & 1u)<<4 | 1u<<5, 0x30); // GPIO bitmask 8.
	codecMaskReg(0x64, 0x45, 0, 0x30); // With automatic output switching
	codecMaskReg(0x64, 0x43, 0, 0x80);
	codecMaskReg(0x64, 0x43, 0x80, 0x80);
	codecWriteReg(0, 0xB, 0x87);
	codecMaskReg(0x64, 0x7C, 0, 1);

	// sub_3257FC()
	codecMaskReg(0x64, 0x22, 0, 4);
	// In AgbBg this is swapped at runtime.
	alignas(4) static const u16 unkData1[3] = {0xE17F, 0x1F80, 0xC17F};
	codecWriteRegBuf(4, 8, (u32*)unkData1, 6);
	codecWriteRegBuf(5, 8, (u32*)cal->filterMic32, 56);
	codecWriteRegBuf(5, 0x48, (u32*)cal->filterMic47, 56);
	codecMaskReg(1, 0x30, 0x40, 0xC0);
	codecMaskReg(1, 0x31, 0x40, 0xC0);
	codecWriteReg(0x65, 0x33, cal->microphoneBias);
	codecMaskWaitReg(0x65, 0x41, cal->PGA_GAIN, 0x3F);
	codecMaskWaitReg(0x65, 0x42, cal->quickCharge, 3);
	codecWriteReg(1, 0x2F, 0x2Bu & 0x7Fu);
	codecMaskReg(0x64, 0x31, 0x44, 0x44); // AgbBg uses val = 0 here
	codecWriteReg(0, 0x41, cal->shutterVolume[0]);
	codecWriteReg(0, 0x42, cal->shutterVolume[0]);
	codecWriteReg(0x64, 0x7B, cal->shutterVolume[1]);

	// Sound stuff starts here
	// Speaker "depop circuit"? Whatever that is. Not existent on retail?
	GPIO_config(GPIO_3_0, GPIO_OUTPUT);
	GPIO_write(GPIO_3_0, 1); // GPIO bitmask 0x40
	TIMER_sleepMs(10); // Fixed 10 ms delay when setting this GPIO.
	*((vu16*)0x10145000) = 0xC800u | 0x20u<<6;
	*((vu16*)0x10145002) = 0xE000u;
	codecMaskReg(0x65, 0x11, 0x10, 0x1C);
	codecWriteReg(0x64, 0x7A, 0);
	codecWriteReg(0x64, 0x78, 0);
	{ // This code block is missing in AgbBg but present in codec module.
		const bool flag = (~codecReadReg(0, 0x40) & 0xCu) == 0;
		codecMaskReg(0, 0x3F, 0, 0xC0);
		codecWriteReg(0, 0x40, 0xC);
		for(u32 i = 0; i < 100; i++) // Some kind of timeout? No error checking.
		{
			if(!(~codecReadReg(0x64, 0x26) & 0x44u)) break;
			TIMER_sleepMs(1);
		}
		codecWriteRegBuf(9, 2, (u32*)cal->filterFree, 6);
		codecWriteRegBuf(8, 0xC, (u32*)&cal->filterFree[3], 50);
		codecWriteRegBuf(9, 8, (u32*)cal->filterFree, 6);
		codecWriteRegBuf(8, 0x4C, (u32*)&cal->filterFree[3], 50);
		if(!flag)
		{
			codecMaskReg(0, 0x3F, 0xC0, 0xC0);
			codecWriteReg(0, 0x40, 0);
		}
	}
	{
		const bool flag = (~codecReadReg(0x64, 0x77) & 0xCu) == 0;
		codecMaskReg(0x64, 0x77, 0xC, 0xC);
		for(u32 i = 0; i < 100; i++) // Some kind of timeout? No error checking.
		{
			if(!(~codecReadReg(0x64, 0x26) & 0x88u)) break;
			TIMER_sleepMs(1);
		}
		codecWriteRegBuf(0xA, 2, (u32*)cal->filterFree, 6);
		codecWriteRegBuf(0xA, 0xC, (u32*)&cal->filterFree[3], 50);
		if(!flag) codecMaskReg(0x64, 0x77, 0, 0xC);
	}

	codecWriteRegBuf(0xC, 2, (u32*)cal->filterSP32, 30);
	codecWriteRegBuf(0xC, 0x42, (u32*)cal->filterSP32, 30);
	codecWriteRegBuf(0xC, 0x20, (u32*)cal->filterSP47, 30);
	codecWriteRegBuf(0xC, 0x60, (u32*)cal->filterSP47, 30);
	codecWriteRegBuf(0xB, 2, (u32*)cal->filterHP32, 30);
	codecWriteRegBuf(0xB, 0x42, (u32*)cal->filterHP32, 30);
	codecWriteRegBuf(0xB, 0x20, (u32*)cal->filterHP47, 30);
	codecWriteRegBuf(0xB, 0x60, (u32*)cal->filterHP47, 30);
	codecMaskReg(0x64, 0x76, 0xC0, 0xC0);
	TIMER_sleepMs(10);
	for(u32 i = 0; i < 100; i++) // Some kind of timeout? No error checking.
	{
		if(!(~codecReadReg(0x64, 0x25) & 0x88u)) break;
		TIMER_sleepMs(1);
	}
	codecWriteReg(0x65, 0xA, 0xA);

	codecMaskReg(0, 0x3F, 0xC0, 0xC0);
	codecWriteReg(0, 0x40, 0);
	codecMaskReg(0x64, 0x77, 0, 0xC);

	u8 val;
	if((codecReadReg(0, 2) & 0xFu) <= 1u && ((codecReadReg(0, 3) & 0x70u)>>4 <= 2u))
	{
		val = 0x3C;
	}
	else val = 0x1C;
	codecWriteReg(0x65, 0xB, val);

	codecWriteReg(0x65, 0xC, (cal->driverGainHP<<3) | 4);
	codecWriteReg(0x65, 0x16, cal->analogVolumeHP);
	codecWriteReg(0x65, 0x17, cal->analogVolumeHP);
	codecMaskReg(0x65, 0x11, 0xC0, 0xC0);
	codecWriteReg(0x65, 0x12, (cal->driverGainSP<<2) | 2);
	codecWriteReg(0x65, 0x13, (cal->driverGainSP<<2) | 2);
	codecWriteReg(0x65, 0x1B, cal->analogVolumeSP);
	codecWriteReg(0x65, 0x1C, cal->analogVolumeSP);
	TIMER_sleepMs(38);
	GPIO_write(GPIO_3_0, 0); // GPIO bitmask 0x40
	TIMER_sleepMs(18); // Fixed 18 ms delay when unsetting this GPIO.


	// Circle pad
	codecWriteReg(0x67, 0x24, 0x98);
	codecWriteReg(0x67, 0x26, 0x00);
	codecWriteReg(0x67, 0x25, 0x43);
	codecWriteReg(0x67, 0x24, 0x18);
	codecWriteReg(0x67, 0x17, cal->analogPrecharge<<4 | cal->analogSense);
	codecWriteReg(0x67, 0x19, cal->analog_XP_Pullup<<4 | cal->analogStabilize);
	codecWriteReg(0x67, 0x1B, cal->YM_Driver<<7 | cal->analogDebounce);
	codecWriteReg(0x67, 0x27, 0x10u | cal->analogInterval);
	codecWriteReg(0x67, 0x26, 0xEC);
	codecWriteReg(0x67, 0x24, 0x18);
	codecWriteReg(0x67, 0x25, 0x53);

	// Not needed?
	//I2C_writeReg(I2C_DEV_CTR_MCU, 0x26, I2C_readReg(I2C_DEV_CTR_MCU, 0x26) | 0x10);

	codecEnableTouchscreen();
}

bool touchscreenState = false;
bool legacySwitchState = false;

void CODEC_deinit(void)
{
	GPIO_write(GPIO_3_0, 1); // GPIO bitmask 0x40
	TIMER_sleepMs(10); // Fixed 10 ms delay when setting this GPIO.
	legacySwitchState = (codecReadReg(0x67, 0x25) & 0x40u) != 0;
	if(!legacySwitchState) codecLegacyStuff(true);
	codecMaskReg(0x67, 0x25, 0, 3);
	touchscreenState = (codecReadReg(0x67, 0x24)>>7) == 0;
	codecDisableTouchscreen();
	codecMaskReg(0x64, 0x76, 0, 0xC0);
	TIMER_sleepMs(30);
	for(u32 i = 0; i < 100; i++)
	{
		if(!(codecReadReg(0x64, 0x25) & 0x88u)) break;
		TIMER_sleepMs(1);
	}
	codecMaskReg(0x64, 0x22, 2, 2);
	TIMER_sleepMs(30);
	for(u32 i = 0; i < 64; i++)
	{
		if(codecReadReg(0x64, 0x22) & 1u) break;
		TIMER_sleepMs(1);
	}
	*((vu16*)0x10145000) &= ~0x8000u;
	*((vu16*)0x10145002) &= ~0x8000u;
	REG_PDN_I2S_CNT = 0;
	GPIO_write(GPIO_3_0, 0); // GPIO bitmask 0x40
	TIMER_sleepMs(18); // Fixed 18 ms delay when unsetting this GPIO.
}

void CODEC_wakeup(void)
{
	GPIO_write(GPIO_3_0, 1); // GPIO bitmask 0x40
	TIMER_sleepMs(10); // Fixed 10 ms delay when setting this GPIO.
	REG_PDN_I2S_CNT = PDN_I2S_CNT_I2S_CLK2_E;
	*((vu16*)0x10145000) |= 0x8000u;
	*((vu16*)0x10145002) |= 0x8000u;
	//codecMaskReg(0x64, 0x45, 0, 0x30); // Output select automatic
	codecMaskReg(0x64, 0x43, 0, 0x80);
	codecMaskReg(0x64, 0x43, 0x80, 0x80);
	codecMaskReg(0x64, 0x22, 0, 2);
	TIMER_sleepMs(40);
	for(u32 i = 0; i < 40; i++)
	{
		if(!(codecReadReg(0x64, 0x22) & 1u)) break;
		TIMER_sleepMs(1);
	}
	codecMaskReg(0x64, 0x76, 0xC0, 0xC0);
	TIMER_sleepMs(10);
	for(u32 i = 0; i < 100; i++)
	{
		if(!(~codecReadReg(0x64, 0x25) & 0x88u)) break;
		TIMER_sleepMs(1);
	}
	codecMaskReg(0x67, 0x25, 3, 3);
	codecLegacyStuff(legacySwitchState);
	if(touchscreenState) codecEnableTouchscreen();
	GPIO_write(GPIO_3_0, 0); // GPIO bitmask 0x40
	TIMER_sleepMs(18); // Fixed 18 ms delay when unsetting this GPIO.
}

bool CODEC_getRawAdcData(CdcAdcData *data)
{
	if((codecReadReg(0x67, 0x26) & 2u) == 0)
	{
		codecReadRegBuf(0xFB, 1, (u32*)data, sizeof(CdcAdcData));

		return true;
	}

	// Codec module does this when data is unavailable. Why?
	//codecSwitchBank(0);

	return false;
}
