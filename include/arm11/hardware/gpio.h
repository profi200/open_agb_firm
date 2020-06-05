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


// bits 3-7 pin number, bits 0-3 reg index.
#define MAKE_GPIO(pin, reg) ((pin)<<3 | (reg))

typedef enum
{
	GPIO_1_0           =  MAKE_GPIO(0u, 0u),
	GPIO_1_1           =  MAKE_GPIO(1u, 0u),
	GPIO_1_2           =  MAKE_GPIO(2u, 0u),

	GPIO_2_0           =  MAKE_GPIO(0u, 1u),
	GPIO_2_1           =  MAKE_GPIO(1u, 1u),
	GPIO_2_2           =  MAKE_GPIO(0u, 2u), // REG_GPIO2_DAT2

	GPIO_3_0           =  MAKE_GPIO(0u, 3u),
	GPIO_3_1           =  MAKE_GPIO(1u, 3u),
	GPIO_3_2           =  MAKE_GPIO(2u, 3u),
	GPIO_3_3           =  MAKE_GPIO(3u, 3u),
	GPIO_3_4           =  MAKE_GPIO(4u, 3u),
	GPIO_3_5           =  MAKE_GPIO(5u, 3u),
	GPIO_3_6           =  MAKE_GPIO(6u, 3u),
	GPIO_3_7           =  MAKE_GPIO(7u, 3u),
	GPIO_3_8           =  MAKE_GPIO(8u, 3u),
	GPIO_3_9           =  MAKE_GPIO(9u, 3u),
	GPIO_3_10          = MAKE_GPIO(10u, 3u),
	GPIO_3_11          = MAKE_GPIO(11u, 3u),
	GPIO_3_12          =  MAKE_GPIO(0u, 4u), // REG_GPIO3_DAT2

	// Aliases
	GPIO_1_TOUCHSCREEN = GPIO_1_1, // Unset while touchscreen pen down. Unused after CODEC init.
	GPIO_1_SHELL       = GPIO_1_2, // 1 when closed.

	GPIO_2_HEADPH_JACK = GPIO_2_0, // Used after CODEC init.

	GPIO_3_HEADPH_JACK = GPIO_3_8, // Unused/other function after CODEC init.
	GPIO_3_MCU         = GPIO_3_9
} Gpio;

#undef MAKE_GPIO



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
bool GPIO_read(Gpio gpio);

/**
 * @brief      Writes a GPIO pin.
 *
 * @param[in]  gpio  The gpio.
 * @param[in]  val   The value. Must be 0 or 1.
 */
void GPIO_write(Gpio gpio, bool val);
