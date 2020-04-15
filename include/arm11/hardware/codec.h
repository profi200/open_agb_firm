#pragma once

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

/**
 * @brief      Get raw ADC data for Circle-Pad/Touchscreen.
 *
 * @param[in]  buf   The buffer to write the data to.
 */
void CODEC_getRawAdcData(u32 buf[13]);
