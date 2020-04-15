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


#define GPIO_INPUT         (0u)
#define GPIO_OUTPUT        (1u)
#define GPIO_EDGE_FALLING  (0u)
#define GPIO_EDGE_RISING   (1u<<1)
#define GPIO_IRQ_ENABLE    (1u<<2)


typedef enum
{
	GPIO_1_0           =  0u<<3 | 0u,
	GPIO_1_1           =  1u<<3 | 0u,
	GPIO_1_2           =  2u<<3 | 0u,

	GPIO_2_0           =  0u<<3 | 1u,
	GPIO_2_1           =  1u<<3 | 1u,

	GPIO_3_0           =  0u<<3 | 2u,

	GPIO_4_0           =  0u<<3 | 3u,
	GPIO_4_1           =  1u<<3 | 3u,
	GPIO_4_2           =  2u<<3 | 3u,
	GPIO_4_3           =  3u<<3 | 3u,
	GPIO_4_4           =  4u<<3 | 3u,
	GPIO_4_5           =  5u<<3 | 3u,
	GPIO_4_6           =  6u<<3 | 3u,
	GPIO_4_7           =  7u<<3 | 3u,
	GPIO_4_8           =  8u<<3 | 3u,
	GPIO_4_9           =  9u<<3 | 3u,
	GPIO_4_10          = 10u<<3 | 3u,
	GPIO_4_11          = 11u<<3 | 3u,

	GPIO_5_0           =  0u<<3 | 4u,

	// Aliases
	GPIO_1_TOUCHSCREEN = GPIO_1_1, // Unset while touchscreen pen down
	GPIO_1_SHELL       = GPIO_1_2, // 1 when closed

	GPIO_4_HEADPH_JACK = GPIO_4_8, // Unset while headphones are plugged in
	GPIO_4_MCU         = GPIO_4_9
} Gpio;



/**
 * @brief      Configures the specified GPIO.
 *
 * @param[in]  gpio  The gpio.
 * @param[in]  cfg   The configuration.
 */
void GPIO_config(Gpio gpio, u8 cfg);

/**
 * @brief      Reads a GPIO pin.
 *
 * @param[in]  gpio  The gpio.
 *
 * @return     The state. Either 0 or 1.
 */
u8 GPIO_read(Gpio gpio);

/**
 * @brief      Writes a GPIO pin.
 *
 * @param[in]  gpio  The gpio.
 * @param[in]  val   The value.
 */
void GPIO_write(Gpio gpio, u8 val);
