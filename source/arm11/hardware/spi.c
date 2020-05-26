/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2019 derrek, profi200
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
#include "arm11/hardware/spi.h"
#include "arm11/hardware/cfg11.h"
#include "arm11/hardware/interrupt.h"


#define NSPI1_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x42800)
#define NSPI2_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x43800)
#define NSPI3_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x60800)


typedef struct
{
	vu32 NSPI_CNT;
	vu8  NSPI_CS;        // 32 bit but can be accessed as u8
	u8 padd1[3];
	vu32 NSPI_BLKLEN;
	vu32 NSPI_FIFO;
	vu8  NSPI_FIFO_STAT; // 32 bit but can be accessed as u8
	u8 padd2[3];
	vu32 NSPI_AUTOPOLL;
	vu32 NSPI_INT_MASK;
	vu32 NSPI_INT_STAT;
} SpiRegs;

enum
{
	SPI_BUS1 = 0u,
	SPI_BUS2 = 1u,
	SPI_BUS3 = 2u
};

// Chip selects.
enum
{
	SPI_CS_0 = 0u<<6,
	SPI_CS_1 = 1u<<6,
	SPI_CS_2 = 2u<<6
};

static const struct
{
	u8 busId;
	u8 csClk;
} spiDevTable[] =
{
	{SPI_BUS3, SPI_CS_0 | NSPI_CLK_2MHz},
	{SPI_BUS3, SPI_CS_1 | NSPI_CLK_4MHz},
	{SPI_BUS3, SPI_CS_2 | NSPI_CLK_4MHz},   // Normally used with the old SPI interface.
	{SPI_BUS1, SPI_CS_0 | NSPI_CLK_16MHz},
	{SPI_BUS1, SPI_CS_1 | NSPI_CLK_512KHz}, // Unused "CS2".
	{SPI_BUS1, SPI_CS_2 | NSPI_CLK_512KHz}, // Unused "CS3".
	{SPI_BUS2, SPI_CS_0 | NSPI_CLK_512KHz}  // Debugger?
};



static SpiRegs* nspiGetBusRegsBase(u8 busId)
{
	SpiRegs *base;
	switch(busId)
	{
		case SPI_BUS1:
			base = (SpiRegs*)NSPI1_REGS_BASE;
			break;
		case SPI_BUS2:
			base = (SpiRegs*)NSPI2_REGS_BASE;
			break;
		case SPI_BUS3:
			base = (SpiRegs*)NSPI3_REGS_BASE;
			break;
		default:
			base = NULL;
	}

	return base;
}

static inline void nspiWaitBusy(const SpiRegs *const regs)
{
	while(regs->NSPI_CNT & NSPI_ENABLE);
}

static inline void nspiWaitFifoBusy(const SpiRegs *const regs)
{
	while(regs->NSPI_FIFO_STAT & NSPI_FIFO_BUSY);
}

void NSPI_init(void)
{
	static bool inited = false;
	if(inited) return;
	inited = true;

	// Switch all 3 buses to the new interface
	REG_CFG11_SPI_CNT = 1u<<2 | 1u<<1 | 1u;

	SpiRegs *regs = nspiGetBusRegsBase(SPI_BUS1);
	regs->NSPI_INT_MASK = NSPI_INT_TRANSF_END; // Disable interrupt 1
	regs->NSPI_INT_STAT = NSPI_INT_AP_TIMEOUT | NSPI_INT_AP_SUCCESS | NSPI_INT_TRANSF_END; // Aknowledge

	regs = nspiGetBusRegsBase(SPI_BUS2);
	regs->NSPI_INT_MASK = NSPI_INT_TRANSF_END;
	regs->NSPI_INT_STAT = NSPI_INT_AP_TIMEOUT | NSPI_INT_AP_SUCCESS | NSPI_INT_TRANSF_END;

	regs = nspiGetBusRegsBase(SPI_BUS3);
	regs->NSPI_INT_MASK = NSPI_INT_TRANSF_END;
	regs->NSPI_INT_STAT = NSPI_INT_AP_TIMEOUT | NSPI_INT_AP_SUCCESS | NSPI_INT_TRANSF_END;

	IRQ_registerIsr(IRQ_SPI2, 14, 0, true, NULL);
	IRQ_registerIsr(IRQ_SPI3, 14, 0, true, NULL);
	IRQ_registerIsr(IRQ_SPI1, 14, 0, true, NULL);
}

bool _NSPI_autoPollBit(SpiDevice dev, u32 params)
{
	SpiRegs *const regs = nspiGetBusRegsBase(spiDevTable[dev].busId);

	regs->NSPI_AUTOPOLL = NSPI_AUTOPOLL_START | params;

	u32 res;
	do
	{
		__wfi();
		res = regs->NSPI_INT_STAT;
	} while(!(res & (NSPI_INT_AP_TIMEOUT | NSPI_INT_AP_SUCCESS)));
	regs->NSPI_INT_STAT = res; // Aknowledge

	return (res & NSPI_INT_AP_TIMEOUT) == 0; // Timeout error
}

void NSPI_writeRead(SpiDevice dev, const u32 *in, u32 *out, u32 inSize, u32 outSize, bool done)
{
	SpiRegs *const regs = nspiGetBusRegsBase(spiDevTable[dev].busId);
	const u32 cntParams = NSPI_ENABLE | spiDevTable[dev].csClk;

	if(in)
	{
		regs->NSPI_BLKLEN = inSize;
		regs->NSPI_CNT = cntParams | NSPI_DIR_WRITE;

		u32 counter = 0;
		do
		{
			if((counter & 31) == 0) nspiWaitFifoBusy(regs);
			regs->NSPI_FIFO = *in++;
			counter += 4;
		} while(counter < inSize);

		nspiWaitBusy(regs);
	}
	if(out)
	{
		regs->NSPI_BLKLEN = outSize;
		regs->NSPI_CNT = cntParams | NSPI_DIR_READ;

		u32 counter = 0;
		do
		{
			if((counter & 31) == 0) nspiWaitFifoBusy(regs);
			*out++ = regs->NSPI_FIFO;
			counter += 4;
		} while(counter < outSize);

		nspiWaitBusy(regs);
	}

	if(done) regs->NSPI_CS = NSPI_DESELECT;
}
