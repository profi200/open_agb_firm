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
#include "arm11/drivers/gpio.h"


#define GPIO_EDGE_FALLING  (0u)
#define GPIO_EDGE_RISING   (1u<<1)
#define GPIO_IRQ_ENABLE    (1u<<2)


static vu16 *const g_datRegs[5] = {(vu16*)&REG_GPIO1_DAT, (vu16*)&REG_GPIO2_DAT,
                                   &REG_GPIO2_DAT2, &REG_GPIO3_DAT, &REG_GPIO3_DAT2};



void GPIO_config(Gpio gpio, u8 cfg)
{
	const u8 regIdx = gpio & 7u;
	const u8 pinNum = gpio>>3;

	// GPIO1 and GPIO3_DAT2 are not configurable.
	if(regIdx == 1)
	{
		u32 reg = REG_GPIO2 & ~((1u<<24 | 1u<<16 | 1u<<8)<<pinNum);

		if(cfg & GPIO_OUTPUT)      reg |= (1u<<8)<<pinNum;  // Direction.
		if(cfg & GPIO_EDGE_RISING) reg |= (1u<<16)<<pinNum; // IRQ edge.
		if(cfg & GPIO_IRQ_ENABLE)  reg |= (1u<<24)<<pinNum; // IRQ enable.

		REG_GPIO2 = reg;
	}
	else if(regIdx == 3)
	{
		u32 reg  = REG_GPIO3_H1 & ~((1u<<16)<<pinNum);
		u32 reg2 = REG_GPIO3_H2 & ~((1u<<16 | 1u)<<pinNum);

		if(cfg & GPIO_OUTPUT)      reg  |= (1u<<16)<<pinNum; // Direction.
		if(cfg & GPIO_EDGE_RISING) reg2 |= 1u<<pinNum;       // IRQ edge.
		if(cfg & GPIO_IRQ_ENABLE)  reg2 |= (1u<<16)<<pinNum; // IRQ enable.

		REG_GPIO3_H1 = reg;
		REG_GPIO3_H2 = reg2;
	}
}

u8 GPIO_read(Gpio gpio)
{
	const u8 regIdx = gpio & 7u;
	const u8 pinNum = gpio>>3;

	if(regIdx > 4) return 0;

	return *g_datRegs[regIdx]>>pinNum & 1u;
}

void GPIO_write(Gpio gpio, u8 val)
{
	const u8 regIdx = gpio & 7u;
	const u8 pinNum = gpio>>3;

	if(regIdx == 0 || regIdx > 4) return;

	u16 tmp = *g_datRegs[regIdx];
	tmp = (tmp & ~(1u<<pinNum)) | (u16)val<<pinNum;
	*g_datRegs[regIdx] = tmp;
}

/*#include "arm11/fmt.h"
void GPIO_print(void)
{
	ee_printf("REG_GPIO1_DAT %04" PRIx8 "\n", REG_GPIO1_DAT);
	ee_printf("REG_GPIO2_DAT %02" PRIx8 "\nREG_GPIO2_DIR %02" PRIx8 "\nREG_GPIO2_EDGE %02" PRIx8 "\nREG_GPIO2_IRQ %02" PRIx8 "\n", REG_GPIO2_DAT, REG_GPIO2_DIR, REG_GPIO2_EDGE, REG_GPIO2_IRQ);
	ee_printf("REG_GPIO2_DAT2 %04" PRIx16 "\n", REG_GPIO2_DAT2);
	ee_printf("REG_GPIO3_DAT %04" PRIx16 "\nREG_GPIO3_DIR %04" PRIx16 "\nREG_GPIO3_EDGE %04" PRIx16 "\nREG_GPIO3_IRQ %04" PRIx16 "\n", REG_GPIO3_DAT, REG_GPIO3_DIR, REG_GPIO3_EDGE, REG_GPIO3_IRQ);
	ee_printf("REG_GPIO3_DAT2 %04" PRIx16 "\n", REG_GPIO3_DAT2);
}*/
