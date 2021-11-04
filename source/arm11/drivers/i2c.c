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
#include "arm11/drivers/i2c.h"
#include "kevent.h"
#include "kmutex.h"
#include "arm11/drivers/interrupt.h"


static const struct
{
	u8 busId;
	u8 devAddr;
} g_i2cDevTable[18] =
{
	{I2C_BUS1, 0x25u<<1}, // 0x4A
	{I2C_BUS1, 0x3Du<<1}, // 0x7A
	{I2C_BUS1, 0x3Cu<<1}, // 0x78
	{I2C_BUS2, 0x25u<<1}, // 0x4A
	{I2C_BUS2, 0x3Cu<<1}, // 0x78
	{I2C_BUS2, 0x16u<<1}, // 0x2C
	{I2C_BUS2, 0x17u<<1}, // 0x2E
	{I2C_BUS2, 0x20u<<1}, // 0x40
	{I2C_BUS2, 0x22u<<1}, // 0x44
	{I2C_BUS3, 0x6Bu<<1}, // 0xD6
	{I2C_BUS3, 0x68u<<1}, // 0xD0
	{I2C_BUS3, 0x69u<<1}, // 0xD2
	{I2C_BUS3, 0x52u<<1}, // 0xA4
	{I2C_BUS3, 0x4Du<<1}, // 0x9A
	{I2C_BUS3, 0x50u<<1}, // 0xA0
	{I2C_BUS2, 0x77u<<1}, // 0xEE
	{I2C_BUS1, 0x20u<<1}, // 0x40
	{I2C_BUS3, 0x2Au<<1}  // 0x54
};

typedef struct
{
	KHandle mutex;
	I2cBus *const i2cBus;
	KHandle event;
} I2cState;
static I2cState g_i2cState[3] = {{0, (I2cBus*)I2C1_REGS_BASE, 0},
                                 {0, (I2cBus*)I2C2_REGS_BASE, 0},
                                 {0, (I2cBus*)I2C3_REGS_BASE, 0}};



static bool checkAck(I2cBus *const i2cBus)
{
	// If we received a NACK stop the transfer.
	if((i2cBus->cnt & I2C_ACK) == 0u)
	{
		i2cBus->cnt = I2C_EN | I2C_IRQ_EN | I2C_ERROR | I2C_STOP;
		return false;
	}

	return true;
}

static void sendByte(I2cBus *const i2cBus, u8 data, u8 params, KHandle const event)
{
	i2cBus->data = data;
	i2cBus->cnt = I2C_EN | I2C_IRQ_EN | I2C_DIR_W | params;
	waitForEvent(event);
}

static u8 recvByte(I2cBus *const i2cBus, u8 params, KHandle const event)
{
	i2cBus->cnt = I2C_EN | I2C_IRQ_EN | I2C_DIR_R | params;
	waitForEvent(event);
	return i2cBus->data;
}

void I2C_init(void)
{
	static bool inited = false;
	if(inited) return;
	inited = true;


	for(u32 i = 0; i < 3; i++)
	{
		static const Interrupt i2cIrqs[3] = {IRQ_I2C1, IRQ_I2C2, IRQ_I2C3};
		const KHandle event = createEvent(true);
		bindInterruptToEvent(event, i2cIrqs[i], 14);
		g_i2cState[i].event = event;

		g_i2cState[i].mutex = createMutex();

		I2cBus *const i2cBus = g_i2cState[i].i2cBus;
		while(i2cBus->cnt & I2C_EN);
		i2cBus->cntex = I2C_CLK_STRETCH;
		i2cBus->scl = I2C_DELAYS(5u, 0u);
	}
}

static bool startTransfer(u8 devAddr, u8 regAddr, bool read, const I2cState *const state)
{
	u32 tries = 8;
	I2cBus *const i2cBus = state->i2cBus;
	const KHandle event = state->event;
	do
	{
		// Edge case on previous transfer error (NACK).
		// This is a special case where we can't predict when or if
		// the IRQ has fired. If it fires after checking but
		// before a wfi this would hang.
		if(i2cBus->cnt & I2C_EN) waitForEvent(event);
		clearEvent(event);

		// Select device and start.
		sendByte(i2cBus, devAddr, I2C_START, event);
		if(!checkAck(i2cBus)) continue;

		// Select register.
		sendByte(i2cBus, regAddr, 0, event);
		if(!checkAck(i2cBus)) continue;

		// Select device in read mode for read transfer.
		if(read)
		{
			sendByte(i2cBus, devAddr | 1u, I2C_START, event);
			if(!checkAck(i2cBus)) continue;
		}

		break;
	} while(--tries > 0);

	return tries > 0;
}

bool I2C_readRegBuf(I2cDevice devId, u8 regAddr, u8 *out, u32 size)
{
	const u8 devAddr = g_i2cDevTable[devId].devAddr;
	const I2cState *const state = &g_i2cState[g_i2cDevTable[devId].busId];
	I2cBus *const i2cBus = state->i2cBus;
	const KHandle event = state->event;
	const KHandle mutex = state->mutex;


	bool res = true;
	lockMutex(mutex);
	if(startTransfer(devAddr, regAddr, true, state))
	{
		while(--size) *out++ = recvByte(i2cBus, I2C_ACK, event);

		// Last byte transfer.
		*out = recvByte(i2cBus, I2C_STOP, event);
	}
	else res = false;
	unlockMutex(mutex);

	return res;
}

bool I2C_writeRegBuf(I2cDevice devId, u8 regAddr, const u8 *in, u32 size)
{
	const u8 devAddr = g_i2cDevTable[devId].devAddr;
	const I2cState *const state = &g_i2cState[g_i2cDevTable[devId].busId];
	I2cBus *const i2cBus = state->i2cBus;
	const KHandle event = state->event;
	const KHandle mutex = state->mutex;


	lockMutex(mutex);
	if(!startTransfer(devAddr, regAddr, false, state))
	{
		unlockMutex(mutex);
		return false;
	}

	while(--size)
	{
		sendByte(i2cBus, *in++, 0, event);
		if(!checkAck(i2cBus))
		{
			unlockMutex(mutex);
			return false;
		}
	}

	// Last byte transfer.
	sendByte(i2cBus, *in, I2C_STOP, event);
	if(!checkAck(i2cBus))
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

bool I2C_writeRegIntSafe(I2cDevice devId, u8 regAddr, u8 data)
{
	const u8 devAddr = g_i2cDevTable[devId].devAddr;
	I2cBus *const i2cBus = getI2cBusRegs(g_i2cDevTable[devId].busId);


	u32 tries = 8;
	do
	{
		while(i2cBus->cnt & I2C_EN);

		// Select device and start.
		i2cBus->data = devAddr;
		i2cBus->cnt = I2C_EN | I2C_DIR_W | I2C_START;
		while(i2cBus->cnt & I2C_EN);
		if(!checkAck(i2cBus)) continue;

		// Select register.
		i2cBus->data = regAddr;
		i2cBus->cnt = I2C_EN | I2C_DIR_W;
		while(i2cBus->cnt & I2C_EN);
		if(!checkAck(i2cBus)) continue;

		break;
	} while(--tries > 0);

	if(tries == 0) return false;

	i2cBus->data = data;
	i2cBus->cnt = I2C_EN | I2C_DIR_W | I2C_STOP;
	while(i2cBus->cnt & I2C_EN);
	if(!checkAck(i2cBus)) return false;

	return true;
}
