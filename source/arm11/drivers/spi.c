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
#include "arm11/drivers/spi.h"
#include "arm11/drivers/cfg11.h"
#include "arm11/drivers/interrupt.h"


static const struct
{
	u8 busId;
	u8 csClk;
} g_spiDevTable[4] =
{
	{SPI_BUS1, NSPI_CS_0 | NSPI_CLK_2MHZ},
	{SPI_BUS1, NSPI_CS_1 | NSPI_CLK_4MHZ},   // Used with the old interface in Horizon OS?
	{SPI_BUS1, NSPI_CS_2 | NSPI_CLK_4MHZ},   // Used with the old interface in Horizon OS.
	{SPI_BUS2, NSPI_CS_0 | NSPI_CLK_16MHZ},
	//{SPI_BUS2, NSPI_CS_1 | NSPI_CLK_512KHZ}, // Unused.
	//{SPI_BUS2, NSPI_CS_2 | NSPI_CLK_512KHZ}, // Unused.
	//{SPI_BUS3, NSPI_CS_0 | NSPI_CLK_512KHZ}  // Debugger?
};



static NspiBus* getBusRegs(u8 busId)
{
	// Don't force inline here.
	return getNspiBusRegs(busId);
}

static inline void nspiWaitBusy(const NspiBus *const nspiBus)
{
	while(nspiBus->cnt & NSPI_EN);
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

	NspiBus *nspiBus = getBusRegs(SPI_BUS1);
	nspiBus->int_mask = NSPI_INT_TRAN_END; // Disable interrupt 1.
	nspiBus->int_stat = NSPI_INT_AP_TMOUT | NSPI_INT_AP_MATCH | NSPI_INT_TRAN_END; // Acknowledge.

	nspiBus = getBusRegs(SPI_BUS2);
	nspiBus->int_mask = NSPI_INT_TRAN_END;
	nspiBus->int_stat = NSPI_INT_AP_TMOUT | NSPI_INT_AP_MATCH | NSPI_INT_TRAN_END;

	nspiBus = getBusRegs(SPI_BUS3);
	nspiBus->int_mask = NSPI_INT_TRAN_END;
	nspiBus->int_stat = NSPI_INT_AP_TMOUT | NSPI_INT_AP_MATCH | NSPI_INT_TRAN_END;

	IRQ_registerIsr(IRQ_SPI2, 14, 0, NULL);
	IRQ_registerIsr(IRQ_SPI3, 14, 0, NULL);
	IRQ_registerIsr(IRQ_SPI1, 14, 0, NULL);
}

bool NSPI_autoPollBit(SpiDevice dev, u32 apParams)
{
	dev &= 0x7Fu;
	NspiBus *const nspiBus = getBusRegs(g_spiDevTable[dev].busId);

	nspiBus->cnt      = g_spiDevTable[dev].csClk;
	nspiBus->autopoll = NSPI_AP_START | apParams;

	u32 res;
	do
	{
		__wfi();
		res = nspiBus->int_stat;
	} while(!(res & (NSPI_INT_AP_TMOUT | NSPI_INT_AP_MATCH)));
	nspiBus->int_stat = res; // Aknowledge.

	return (res & NSPI_INT_AP_TMOUT) == 0; // Timeout error.
}

void NSPI_writeRead(SpiDevice dev, const u32 *in, u32 *out, u32 inSize, u32 outSize)
{
	NspiBus *const nspiBus = getBusRegs(g_spiDevTable[dev & 0x7Fu].busId);
	const u32 cntParams = NSPI_EN | g_spiDevTable[dev & 0x7Fu].csClk;

	if(in)
	{
		nspiBus->blklen = inSize;
		nspiBus->cnt = cntParams | NSPI_DIR_W;

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
		nspiBus->cnt = cntParams | NSPI_DIR_R;

		u32 counter = 0;
		do
		{
			if((counter & 31) == 0) nspiWaitFifoBusy(nspiBus);
			*out++ = nspiBus->fifo;
			counter += 4;
		} while(counter < outSize);

		nspiWaitBusy(nspiBus);
	}

	if(dev & NSPI_DEV_CS_HIGH) nspiBus->cs = NSPI_CS_HIGH;
}
