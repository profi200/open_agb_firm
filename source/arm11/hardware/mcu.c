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

#include <stdatomic.h>
#include "arm11/hardware/mcu.h"
#include "arm11/hardware/i2c.h"
#include "arm11/hardware/interrupt.h"
#include "arm11/hardware/gpio.h"


static bool g_mcuIrq = false;
static u32 g_mcuEvents = 0;



static void mcuIrqHandler(UNUSED u32 intSource);

u8 MCU_readReg(McuReg reg)
{
	return I2C_readReg(I2C_DEV_CTR_MCU, reg);
}

bool MCU_writeReg(McuReg reg, u8 data)
{
	return I2C_writeReg(I2C_DEV_CTR_MCU, reg, data);
}

bool MCU_readRegBuf(McuReg reg, u8 *out, u32 size)
{
	return I2C_readRegBuf(I2C_DEV_CTR_MCU, reg, out, size);
}

bool MCU_writeRegBuf(McuReg reg, const u8 *const in, u32 size)
{
	return I2C_writeRegBuf(I2C_DEV_CTR_MCU, reg, in, size);
}

void MCU_init(void)
{
	static bool inited = false;
	if(inited) return;
	inited = true;

	I2C_init();

	// Configure GPIO for MCU event IRQs
	GPIO_config(GPIO_3_MCU, GPIO_INPUT | GPIO_EDGE_FALLING | GPIO_IRQ_ENABLE);
	IRQ_registerIsr(IRQ_CTR_MCU, 14, 0, mcuIrqHandler);

	atomic_store_explicit(&g_mcuIrq, true, memory_order_relaxed);
	(void)MCU_getEvents(0);

	MCU_setEventMask(0xC0BF3F80);
}

static void mcuIrqHandler(UNUSED u32 intSource)
{
	g_mcuIrq = true;
}

bool MCU_setEventMask(u32 mask)
{
	return MCU_writeRegBuf(MCU_REG_EVENT_MASK, (const u8*)&mask, 4);
}

u32 MCU_getEvents(u32 mask)
{
	u32 events = g_mcuEvents;

	if(atomic_load_explicit(&g_mcuIrq, memory_order_relaxed))
	{
		atomic_store_explicit(&g_mcuIrq, false, memory_order_relaxed);

		u32 data;
		if(!MCU_readRegBuf(MCU_REG_EVENTS, (u8*)&data, 4)) return 0;

		events |= data;
	}

	g_mcuEvents = events & ~mask;

	return events & mask;
}

u32 MCU_waitEvents(u32 mask)
{
	u32 events;

	while((events = MCU_getEvents(mask)) == 0u)
	{
		__wfi();
	}

	return events;
}
