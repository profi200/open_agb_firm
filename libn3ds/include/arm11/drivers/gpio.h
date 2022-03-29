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
#include "mem_map.h"


#define GPIO_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x47000)
// 3 GPIOs (bits 0-2)
#define REG_GPIO1_DAT   *((const vu8*)(GPIO_REGS_BASE + 0x00)) // Read-only.

// 2 GPIOs (bits 0-1)
#define REG_GPIO2       *((     vu32*)(GPIO_REGS_BASE + 0x10))
#define REG_GPIO2_DAT   *((      vu8*)(GPIO_REGS_BASE + 0x10))
#define REG_GPIO2_DIR   *((      vu8*)(GPIO_REGS_BASE + 0x11)) // 0 = input, 1 = output.
#define REG_GPIO2_EDGE  *((      vu8*)(GPIO_REGS_BASE + 0x12)) // IRQ edge 0 = falling, 1 = rising.
#define REG_GPIO2_IRQ   *((      vu8*)(GPIO_REGS_BASE + 0x13)) // 1 = IRQ enable.
// 1 GPIO (bit 0)
#define REG_GPIO2_DAT2  *((     vu16*)(GPIO_REGS_BASE + 0x14)) // Only bit 0 writable.

// 12 GPIOs (bits 0-11)
#define REG_GPIO3_H1    *((     vu32*)(GPIO_REGS_BASE + 0x20)) // First half.
#define REG_GPIO3_DAT   *((     vu16*)(GPIO_REGS_BASE + 0x20))
#define REG_GPIO3_DIR   *((     vu16*)(GPIO_REGS_BASE + 0x22))
#define REG_GPIO3_H2    *((     vu32*)(GPIO_REGS_BASE + 0x24)) // Second half.
#define REG_GPIO3_EDGE  *((     vu16*)(GPIO_REGS_BASE + 0x24))
#define REG_GPIO3_IRQ   *((     vu16*)(GPIO_REGS_BASE + 0x26))
// 1 GPIO (bit 0)
#define REG_GPIO3_DAT2  *((     vu16*)(GPIO_REGS_BASE + 0x28)) // WiFi.


#define GPIO_INPUT           (0u)
#define GPIO_OUTPUT          (1u)
#define GPIO_NO_IRQ          (0u)
#define GPIO_IRQ_FALLING     (1u<<2 | 0u)
#define GPIO_IRQ_RISING      (1u<<2 | 1u<<1)


// bits 3-7 pin number, bits 0-3 reg index.
#define MAKE_GPIO(pin, reg)  ((pin)<<3 | (reg))

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
 * @param[in]  cfg   The configuration. See defines above.
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
 * @param[in]  val   The value. Must be 0 or 1.
 */
void GPIO_write(Gpio gpio, u8 val);
