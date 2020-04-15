/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2019 Sergi Granell (xerpi), Paul LaMendola (paulguy), derrek, profi200
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
#include "arm11/hardware/csnd.h"
#include "arm11/hardware/codec.h"



void CSND_init(void)
{
	static bool inited = false;
	if(inited) return;
	inited = true;


	CODEC_init();

	//static const u8 sliderBounds[2] = {0xE, 0xF6}; // Volume slider 0% and 100% offset
	//I2C_writeRegBuf(I2C_DEV_CTR_MCU, 0x58, sliderBounds, 2);
	REG_CSND_MASTER_VOL = 0x8000;
	REG_CSND_UNK_CNT = 1u<<15 | 1u<<14;

	for(u32 i = 0; i < 32; i++) REG_CSND_CH_CNT(i) = 0;
	for(u32 i = 0; i < 2; i++) REG_CSND_CAP_CNT(i) = 0;
}

void CSND_setupCh(u8 ch, s16 sampleRate, u32 vol, const u32 *const data, const u32 *const data2, u32 size, u16 flags)
{
	REG_CSND_CH_SR(ch) = sampleRate;
	REG_CSND_CH_VOL(ch) = vol;
	REG_CSND_CH_CAPVOL(ch) = vol;
	REG_CSND_CH_ST_ADDR(ch) = (u32)data;
	REG_CSND_CH_SIZE(ch) = size;
	REG_CSND_CH_LP_ADDR(ch) = (u32)data2;
	REG_CSND_CH_ST_ADPCM(ch) = 0; // Hardcoded for now. TODO
	REG_CSND_CH_LP_ADPCM(ch) = 0; // Hardcoded for now. TODO
	REG_CSND_CH_CNT(ch) = CSND_CH_START | flags; // Start in paused state.
}


void CSND_startCap(u8 ch, s16 sampleRate, u32 *const data, u32 size, u16 flags)
{
	REG_CSND_CAP_SR(ch) = sampleRate;
	REG_CSND_CAP_SIZE(ch) = size;
	REG_CSND_CAP_ADDR(ch) = (u32)data;
	REG_CSND_CAP_CNT(ch) = CSND_CAP_START | flags;
}
