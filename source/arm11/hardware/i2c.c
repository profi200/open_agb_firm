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
#include "arm11/hardware/i2c.h"
#include "kevent.h"
#include "kmutex.h"
#include "arm11/hardware/interrupt.h"


typedef struct
{
	vu8  I2C_DATA;
	vu8  I2C_CNT;
	vu16 I2C_CNTEX;
	vu16 I2C_SCL;
} I2cRegs;

enum
{
	I2C_BUS1 = 0u,
	I2C_BUS2 = 1u,
	I2C_BUS3 = 2u
};

static const struct
{
	u8 busId;
	u8 devAddr;
} i2cDevTable[] =
{
	{I2C_BUS1, 0x4A},
	{I2C_BUS1, 0x7A},
	{I2C_BUS1, 0x78},
	{I2C_BUS2, 0x4A},
	{I2C_BUS2, 0x78},
	{I2C_BUS2, 0x2C},
	{I2C_BUS2, 0x2E},
	{I2C_BUS2, 0x40},
	{I2C_BUS2, 0x44},
	{I2C_BUS3, 0xD6},
	{I2C_BUS3, 0xD0},
	{I2C_BUS3, 0xD2},
	{I2C_BUS3, 0xA4},
	{I2C_BUS3, 0x9A},
	{I2C_BUS3, 0xA0},
	{I2C_BUS2, 0xEE},
	{I2C_BUS1, 0x40},
	{I2C_BUS3, 0x54}
};

typedef struct
{
	I2cRegs *const regs;
	KEvent event;
	KMutex mutex;
} I2cState;
static I2cState g_i2cState[3] = {{(I2cRegs*)I2C1_REGS_BASE, NULL, NULL},
                                 {(I2cRegs*)I2C2_REGS_BASE, NULL, NULL},
                                 {(I2cRegs*)I2C3_REGS_BASE, NULL, NULL}};



static bool checkAck(I2cRegs *const regs)
{
	// If we received a NACK stop the transfer.
	if((regs->I2C_CNT & I2C_ACK) == 0u)
	{
		regs->I2C_CNT = I2C_ENABLE | I2C_IRQ_ENABLE | I2C_ERROR | I2C_STOP;
		return false;
	}

	return true;
}

static void sendByte(I2cRegs *const regs, u8 data, u8 params, const KEvent event)
{
	regs->I2C_DATA = data;
	regs->I2C_CNT = I2C_ENABLE | I2C_IRQ_ENABLE | I2C_DIR_WRITE | params;
	waitForEvent(event);
}

static u8 recvByte(I2cRegs *const regs, u8 params, const KEvent event)
{
	regs->I2C_CNT = I2C_ENABLE | I2C_IRQ_ENABLE | I2C_DIR_READ | params;
	waitForEvent(event);
	return regs->I2C_DATA;
}

void I2C_init(void)
{
	static bool inited = false;
	if(inited) return;
	inited = true;


	KEvent tmp = createEvent(true);
	bindInterruptToEvent(tmp, IRQ_I2C1, 14);
	g_i2cState[0].event = tmp;
	tmp = createEvent(true);
	bindInterruptToEvent(tmp, IRQ_I2C2, 14);
	g_i2cState[1].event = tmp;
	tmp = createEvent(true);
	bindInterruptToEvent(tmp, IRQ_I2C3, 14);
	g_i2cState[2].event = tmp;

	for(u32 i = 0; i < 3; i++)
	{
		g_i2cState[i].mutex = createMutex();
	}

	while(REG_I2C1_CNT & I2C_ENABLE);
	REG_I2C1_CNTEX = I2C_CLK_STRETCH;
	REG_I2C1_SCL = I2C_DELAYS(5u, 0u);

	while(REG_I2C2_CNT & I2C_ENABLE);
	REG_I2C2_CNTEX = I2C_CLK_STRETCH;
	REG_I2C2_SCL = I2C_DELAYS(5u, 0u);

	while(REG_I2C3_CNT & I2C_ENABLE);
	REG_I2C3_CNTEX = I2C_CLK_STRETCH;
	REG_I2C3_SCL = I2C_DELAYS(5u, 0u);
}

static bool startTransfer(u8 devAddr, u8 regAddr, bool read, const I2cState *const state)
{
	u32 tries = 8;
	do
	{
		I2cRegs *const regs = state->regs;
		const KEvent event = state->event;

		// Edge case on previous transfer error (NACK).
		// This is a special case where we can't predict when or if
		// the IRQ has fired. If it fires after checking but
		// before a wfi this would hang.
		if(regs->I2C_CNT & I2C_ENABLE) waitForEvent(event);
		clearEvent(event);

		// Select device and start.
		sendByte(regs, devAddr, I2C_START, event);
		if(!checkAck(regs)) continue;

		// Select register.
		sendByte(regs, regAddr, 0, event);
		if(!checkAck(regs)) continue;

		// Select device in read mode for read transfer.
		if(read)
		{
			sendByte(regs, devAddr | 1u, I2C_START, event);
			if(!checkAck(regs)) continue;
		}

		break;
	} while(--tries > 0);

	return tries > 0;
}

bool I2C_readRegBuf(I2cDevice devId, u8 regAddr, u8 *out, u32 size)
{
	const u8 devAddr = i2cDevTable[devId].devAddr;
	const I2cState *const state = &g_i2cState[i2cDevTable[devId].busId];
	I2cRegs *const regs = state->regs;
	const KEvent event = state->event;
	const KMutex mutex = state->mutex;


	bool res = true;
	lockMutex(mutex);
	if(startTransfer(devAddr, regAddr, true, state))
	{
		while(--size) *out++ = recvByte(regs, I2C_ACK, event);

		// Last byte transfer.
		*out = recvByte(regs, I2C_STOP, event);
	}
	else res = false;
	unlockMutex(mutex);

	return res;
}

bool I2C_writeRegBuf(I2cDevice devId, u8 regAddr, const u8 *in, u32 size)
{
	const u8 devAddr = i2cDevTable[devId].devAddr;
	const I2cState *const state = &g_i2cState[i2cDevTable[devId].busId];
	I2cRegs *const regs = state->regs;
	const KEvent event = state->event;
	const KMutex mutex = state->mutex;


	lockMutex(mutex);
	if(!startTransfer(devAddr, regAddr, false, state))
	{
		unlockMutex(mutex);
		return false;
	}

	while(--size)
	{
		sendByte(regs, *in++, 0, event);
		if(!checkAck(regs))
		{
			unlockMutex(mutex);
			return false;
		}
	}

	// Last byte transfer.
	sendByte(regs, *in, I2C_STOP, event);
	if(!checkAck(regs))
	{
		unlockMutex(mutex);
		return false;
	}
	unlockMutex(mutex);

	return true;
}

u8 I2C_readReg(I2cDevice devId, u8 regAddr)
{
	u8 data;
	if(!I2C_readRegBuf(devId, regAddr, &data, 1)) return 0xFF;
	return data;
}

bool I2C_writeReg(I2cDevice devId, u8 regAddr, u8 data)
{
	return I2C_writeRegBuf(devId, regAddr, &data, 1);
}

static I2cRegs* i2cGetBusRegsBase(u8 busId)
{
	I2cRegs *base;
	switch(busId)
	{
		case I2C_BUS1:
			base = (I2cRegs*)I2C1_REGS_BASE;
			break;
		case I2C_BUS2:
			base = (I2cRegs*)I2C2_REGS_BASE;
			break;
		case I2C_BUS3:
			base = (I2cRegs*)I2C3_REGS_BASE;
			break;
		default:
			base = NULL;
	}

	return base;
}

bool I2C_writeRegIntSafe(I2cDevice devId, u8 regAddr, u8 data)
{
	const u8 devAddr = i2cDevTable[devId].devAddr;
	I2cRegs *const regs = i2cGetBusRegsBase(i2cDevTable[devId].busId);


	u32 tries = 8;
	do
	{
		while(regs->I2C_CNT & I2C_ENABLE);

		// Select device and start.
		regs->I2C_DATA = devAddr;
		regs->I2C_CNT = I2C_ENABLE | I2C_DIR_WRITE | I2C_START;
		while(regs->I2C_CNT & I2C_ENABLE);
		if(!checkAck(regs)) continue;

		// Select register.
		regs->I2C_DATA = regAddr;
		regs->I2C_CNT = I2C_ENABLE | I2C_DIR_WRITE;
		while(regs->I2C_CNT & I2C_ENABLE);
		if(!checkAck(regs)) continue;

		break;
	} while(--tries > 0);

	if(tries == 0) return false;

	regs->I2C_DATA = data;
	regs->I2C_CNT = I2C_ENABLE | I2C_DIR_WRITE | I2C_STOP;
	while(regs->I2C_CNT & I2C_ENABLE);
	if(!checkAck(regs)) return false;

	return true;
}
