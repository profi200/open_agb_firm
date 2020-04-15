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
#include "mem_map.h"
#include "arm11/hardware/gpio.h"


#define GPIO_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x47000)
// 3 GPIOs (bits 0-2)
#define REG_GPIO1_DAT   *((vu16*)(GPIO_REGS_BASE + 0x00)) // Read-only?
#define REG_GPIO1_UNK2  *((vu16*)(GPIO_REGS_BASE + 0x02)) // ?
#define REG_GPIO1_UNK4  *((vu32*)(GPIO_REGS_BASE + 0x04)) // ?

// 2 GPIOs (bits 0-1)
#define REG_GPIO2       *((vu32*)(GPIO_REGS_BASE + 0x10))
#define REG_GPIO2_DAT   *((vu8* )(GPIO_REGS_BASE + 0x10))
#define REG_GPIO2_DIR   *((vu8* )(GPIO_REGS_BASE + 0x11)) // 0 = input, 1 = output
#define REG_GPIO2_EDGE  *((vu8* )(GPIO_REGS_BASE + 0x12)) // IRQ edge 0 = falling, 1 = rising
#define REG_GPIO2_IRQ   *((vu8* )(GPIO_REGS_BASE + 0x13)) // 1 = IRQ enable

// 1 GPIO (bit 0)
#define REG_GPIO3       *((vu8* )(GPIO_REGS_BASE + 0x14))
#define REG_GPIO3_DAT   *((vu8* )(GPIO_REGS_BASE + 0x14)) // Only bit 0 writable
#define REG_GPIO3_UNK5  *((vu8* )(GPIO_REGS_BASE + 0x15)) // Only 1 bit?

// 12 GPIOs (bits 0-11)
#define REG_GPIO4_H1    *((vu32*)(GPIO_REGS_BASE + 0x20)) // First half
#define REG_GPIO4_DAT   *((vu16*)(GPIO_REGS_BASE + 0x20))
#define REG_GPIO4_DIR   *((vu16*)(GPIO_REGS_BASE + 0x22))
#define REG_GPIO4_H2    *((vu32*)(GPIO_REGS_BASE + 0x24)) // Second half
#define REG_GPIO4_EDGE  *((vu16*)(GPIO_REGS_BASE + 0x24))
#define REG_GPIO4_IRQ   *((vu16*)(GPIO_REGS_BASE + 0x26))

// 1 GPIO (bit 0)
#define REG_GPIO5_DAT   *((vu16*)(GPIO_REGS_BASE + 0x28)) // WiFi


static vu16 *const datRegs[5] = {&REG_GPIO1_DAT, (vu16*)&REG_GPIO2_DAT, (vu16*)&REG_GPIO3_DAT, &REG_GPIO4_DAT, &REG_GPIO5_DAT};



void GPIO_config(Gpio gpio, u8 cfg)
{
	const u8 bank = gpio & 7u;
	const u8 pin = gpio>>3;

	// GPIO1 and GPIO5 are not configurable.
	if(bank == 1)
	{
		u32 reg = REG_GPIO2 & ~((1u<<24 | 1u<<16 | 1u<<8)<<pin);

		if(cfg & GPIO_OUTPUT) reg |= (1u<<8)<<pin;       // Direction
		if(cfg & GPIO_EDGE_RISING) reg |= (1u<<16)<<pin; // IRQ edge
		if(cfg & GPIO_IRQ_ENABLE) reg |= (1u<<24)<<pin;  // IRQ enable

		REG_GPIO2 = reg;
	}
	else if(bank == 3)
	{
		u32 reg = REG_GPIO4_H1 & ~((1u<<16)<<pin);
		u32 reg2 = REG_GPIO4_H2 & ~((1u<<16 | 1u)<<pin);

		if(cfg & GPIO_OUTPUT) reg |= (1u<<16)<<pin;      // Direction
		if(cfg & GPIO_EDGE_RISING) reg2 |= 1u<<pin;      // IRQ edge
		if(cfg & GPIO_IRQ_ENABLE) reg2 |= (1u<<16)<<pin; // IRQ enable

		REG_GPIO4_H1 = reg;
		REG_GPIO4_H2 = reg2;
	}
}

u8 GPIO_read(Gpio gpio)
{
	const u8 bank = gpio & 7u;
	const u8 pin = gpio>>3;

	if(bank > 4) return 0;

	return *datRegs[bank]>>pin & 1u;
}

void GPIO_write(Gpio gpio, u8 val)
{
	const u8 bank = gpio & 7u;
	const u8 pin = gpio>>3;

	if(bank == 0 || bank > 4) return;

	u16 tmp = *datRegs[bank];
	tmp &= ~(1u<<pin) | val<<pin;
	*datRegs[bank] = tmp;
}

/*void GPIO_dbgPrint(void)
{
#include "arm11/fmt.h"

	ee_printf("REG_GPIO1_DAT %04" PRIx16 "\nREG_GPIO1_UNK2 %04" PRIx16 "\nREG_GPIO1_UNK4 %08" PRIx32 "\n", REG_GPIO1_DAT, REG_GPIO1_UNK2, REG_GPIO1_UNK4);
	ee_printf("REG_GPIO2_DAT %02" PRIx8 "\nREG_GPIO2_DIR %02" PRIx8 "\nREG_GPIO2_EDGE %02" PRIx8 "\nREG_GPIO2_IRQ %02" PRIx8 "\n", REG_GPIO2_DAT, REG_GPIO2_DIR, REG_GPIO2_EDGE, REG_GPIO2_IRQ);
	ee_printf("REG_GPIO3_DAT %02" PRIx8 "\nREG_GPIO3_UNK5 %02" PRIx8 "\n", REG_GPIO3_DAT, REG_GPIO3_UNK5);
	ee_printf("REG_GPIO4_DAT %04" PRIx16 "\nREG_GPIO4_DIR %04" PRIx16 "\nREG_GPIO4_EDGE %04" PRIx16 "\nREG_GPIO4_IRQ %04" PRIx16 "\n", REG_GPIO4_DAT, REG_GPIO4_DIR, REG_GPIO4_EDGE, REG_GPIO4_IRQ);
	ee_printf("REG_GPIO5_DAT %04" PRIx16 "\n", REG_GPIO5_DAT);
}*/
