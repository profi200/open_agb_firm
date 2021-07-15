#pragma once

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


typedef struct
{
	u16 touchX[5];
	u16 touchY[5];
	u16 cpadY[8];
	u16 cpadX[8];
} CdcAdcData;



/**
 * @brief      Initialize CODEC for Circle-Pad/Touchscreen/Sound.
 */
void CODEC_init(void);

/**
 * @brief      Deinitializes the CODEC chip for sleep or poweroff.
 */
void CODEC_deinit(void);

/**
 * @brief      The opposite of CODEC_deinit(). Does a partial init.
 */
void CODEC_wakeup(void);

void CODEC_muteI2S(void);

void CODEC_unmuteI2S(void);

/**
 * @brief      Get raw ADC data for Circle-Pad/Touchscreen.
 *
 * @param      data  The output data pointer. Must be 4 bytes aligned.
 *
 * @return     Returns true if data was available and false otherwise.
 */
bool CODEC_getRawAdcData(CdcAdcData *data);
