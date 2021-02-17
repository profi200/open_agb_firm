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
#include "arm11/hardware/spi.h"
#include "arm11/hardware/cfg11.h"
#include "arm11/hardware/interrupt.h"


#define NSPI1_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x42800)
#define NSPI2_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x43800)
#define NSPI3_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x60800)


typedef struct
{
	vu32 cnt;
	vu8  cs;        // 32 bit but can be accessed as u8.
	u8 _0x5[3];
	vu32 blklen;
	vu32 fifo;
	vu8  fifo_stat; // 32 bit but can be accessed as u8.
	u8 _0x11[3];
	vu32 autopoll;
	vu32 int_mask;
	vu32 int_stat;
} NspiBus;
static_assert(offsetof(NspiBus, int_stat) == 0x1C, "Error: Member int_stat of NspiBus is not at offset 0x1C!");

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
} g_spiDevTable[] =
{
	{SPI_BUS3, SPI_CS_0 | NSPI_CLK_2MHz},
	{SPI_BUS3, SPI_CS_1 | NSPI_CLK_4MHz},
	{SPI_BUS3, SPI_CS_2 | NSPI_CLK_4MHz},   // Normally used with the old SPI interface.
	{SPI_BUS1, SPI_CS_0 | NSPI_CLK_16MHz},
	{SPI_BUS1, SPI_CS_1 | NSPI_CLK_512KHz}, // Unused "CS2".
	{SPI_BUS1, SPI_CS_2 | NSPI_CLK_512KHz}, // Unused "CS3".
	{SPI_BUS2, SPI_CS_0 | NSPI_CLK_512KHz}  // Debugger?
};



static NspiBus* getNspiBusRegs(u8 busId)
{
	NspiBus *nspiBus;
	switch(busId)
	{
		case SPI_BUS1:
			nspiBus = (NspiBus*)NSPI1_REGS_BASE;
			break;
		case SPI_BUS2:
			nspiBus = (NspiBus*)NSPI2_REGS_BASE;
			break;
		case SPI_BUS3:
			nspiBus = (NspiBus*)NSPI3_REGS_BASE;
			break;
		default:
			nspiBus = NULL;
	}

	return nspiBus;
}

static inline void nspiWaitBusy(const NspiBus *const nspiBus)
{
	while(nspiBus->cnt & NSPI_ENABLE);
}

static inline void nspiWaitFifoBusy(const NspiBus *const nspiBus)
{
	while(nspiBus->fifo_stat & NSPI_FIFO_BUSY);
}

void NSPI_init(void)
{
	static bool inited = false;
	if(inited) return;
	inited = true;

	// Switch all 3 buses to the new interface.
	getCfg11Regs()->spi_cnt = SPI_CNT_SPI3_NEW_IF | SPI_CNT_SPI2_NEW_IF | SPI_CNT_SPI1_NEW_IF;

	NspiBus *nspiBus = getNspiBusRegs(SPI_BUS1);
	nspiBus->int_mask = NSPI_INT_TRANSF_END; // Disable interrupt 1.
	nspiBus->int_stat = NSPI_INT_AP_TIMEOUT | NSPI_INT_AP_SUCCESS | NSPI_INT_TRANSF_END; // Aknowledge.

	nspiBus = getNspiBusRegs(SPI_BUS2);
	nspiBus->int_mask = NSPI_INT_TRANSF_END;
	nspiBus->int_stat = NSPI_INT_AP_TIMEOUT | NSPI_INT_AP_SUCCESS | NSPI_INT_TRANSF_END;

	nspiBus = getNspiBusRegs(SPI_BUS3);
	nspiBus->int_mask = NSPI_INT_TRANSF_END;
	nspiBus->int_stat = NSPI_INT_AP_TIMEOUT | NSPI_INT_AP_SUCCESS | NSPI_INT_TRANSF_END;

	IRQ_registerIsr(IRQ_SPI2, 14, 0, NULL);
	IRQ_registerIsr(IRQ_SPI3, 14, 0, NULL);
	IRQ_registerIsr(IRQ_SPI1, 14, 0, NULL);
}

bool NSPI_autoPollBit(SpiDevice dev, u32 ap_params)
{
	NspiBus *const nspiBus = getNspiBusRegs(g_spiDevTable[dev].busId);

	nspiBus->cnt = g_spiDevTable[dev].csClk;
	nspiBus->autopoll = NSPI_AUTOPOLL_START | ap_params;

	u32 res;
	do
	{
		__wfi();
		res = nspiBus->int_stat;
	} while(!(res & (NSPI_INT_AP_TIMEOUT | NSPI_INT_AP_SUCCESS)));
	nspiBus->int_stat = res; // Aknowledge.

	return (res & NSPI_INT_AP_TIMEOUT) == 0; // Timeout error.
}

void NSPI_writeRead(SpiDevice dev, const u32 *in, u32 *out, u32 inSize, u32 outSize)
{
	NspiBus *const nspiBus = getNspiBusRegs(g_spiDevTable[dev].busId);
	const u32 cntParams = NSPI_ENABLE | g_spiDevTable[dev].csClk;

	if(in)
	{
		nspiBus->blklen = inSize;
		nspiBus->cnt = cntParams | NSPI_DIR_WRITE;

		u32 counter = 0;
		do
		{
			if((counter & 31) == 0) nspiWaitFifoBusy(nspiBus);
			nspiBus->fifo = *in++;
			counter += 4;
		} while(counter < inSize);

		nspiWaitBusy(nspiBus);
	}
	if(out)
	{
		nspiBus->blklen = outSize;
		nspiBus->cnt = cntParams | NSPI_DIR_READ;

		u32 counter = 0;
		do
		{
			if((counter & 31) == 0) nspiWaitFifoBusy(nspiBus);
			*out++ = nspiBus->fifo;
			counter += 4;
		} while(counter < outSize);

		nspiWaitBusy(nspiBus);
	}
}

void NSPI_deselect(SpiDevice dev)
{
	getNspiBusRegs(g_spiDevTable[dev].busId)->cs = NSPI_DESELECT;
}
