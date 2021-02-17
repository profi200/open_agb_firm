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

#include <assert.h>
#include "types.h"
#include "mem_map.h"
#include "arm9/hardware/spicard.h"
#include "arm9/hardware/cfg9.h"
#include "arm9/hardware/interrupt.h"
#include "arm9/hardware/timer.h"
#include "arm9/hardware/ndma.h"


#define SPICARD_REGS_BASE  (IO_MEM_ARM9_ONLY + 0xD800)

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
} Nspi;
static_assert(offsetof(Nspi, int_stat) == 0x1C, "Error: Member int_stat of Nspi is not at offset 0x1C!");

ALWAYS_INLINE Nspi* getNspiRegs(void)
{
	return (Nspi*)SPICARD_REGS_BASE;
}



static inline void nspiWaitBusy(void)
{
	Nspi *const nspi = getNspiRegs();
	while(nspi->cnt & NSPI_ENABLE);
}

static inline void nspiWaitFifoBusy(void)
{
	Nspi *const nspi = getNspiRegs();
	while(nspi->fifo_stat & NSPI_FIFO_BUSY);
}

void SPICARD_init(void)
{
	static bool inited = false;
	if(inited) return;
	inited = true;

	// TODO: The whole gamecard init doesn't belong here.
#define REG_NTRCARDMCNT       *((vu16*)0x10164000)
#define REG_NTRCARDROMCNT     *((vu32*)0x10164004)

	Cfg9 *const cfg9 = getCfg9Regs();
	cfg9->card_insert_delay = 0x1988; // 100 ms.
	cfg9->card_pwroff_delay = 0x264C; // 150 ms.
	// boot9 waits here. Unnecessary?

	cfg9->card_power = CARD_POWER_OFF_REQ;     // Request power off.
	while(cfg9->card_power != CARD_POWER_OFF); // Aotomatically changes to off.
	TIMER_sleepMs(1);

	cfg9->card_power = CARD_POWER_ON_RESET; // Power on and reset.
	TIMER_sleepMs(10);

	cfg9->card_power = CARD_POWER_ON; // Power on.
	TIMER_sleepMs(27);

	// Switch to NTRCARD controller.
	cfg9->cardctl = CARDCTL_NTRCARD;
	REG_NTRCARDMCNT = 0xC000u;
	REG_NTRCARDROMCNT = 0x20000000;
	TIMER_sleepMs(120);

	cfg9->cardctl = CARDCTL_NSPI_SEL | CARDCTL_NTRCARD;

	IRQ_registerIsr(IRQ_CTR_CARD_1, NULL);
	Nspi *const nspi = getNspiRegs();
	nspi->int_mask = NSPI_INT_TRANSF_END; // Disable interrupt 1.
	nspi->int_stat = NSPI_INT_AP_TIMEOUT | NSPI_INT_AP_SUCCESS | NSPI_INT_TRANSF_END; // Aknowledge.
}

bool SPICARD_autoPollBit(NspiClk clk, u32 ap_params)
{
	Nspi *const nspi = getNspiRegs();
	nspi->cnt = clk;
	nspi->autopoll = NSPI_AUTOPOLL_START | ap_params;

	u32 res;
	do
	{
		__wfi();
		res = nspi->int_stat;
	} while(!(res & (NSPI_INT_AP_TIMEOUT | NSPI_INT_AP_SUCCESS)));
	nspi->int_stat = res; // Aknowledge.

	return (res & NSPI_INT_AP_TIMEOUT) == 0; // Timeout error.
}

void SPICARD_writeRead(NspiClk clk, const u32 *in, u32 *out, u32 inSize, u32 outSize)
{
	const u32 cntParams = NSPI_ENABLE | NSPI_BUS_1BIT | clk;

	Nspi *const nspi = getNspiRegs();
	if(in)
	{
		nspi->blklen = inSize;
		nspi->cnt = cntParams | NSPI_DIR_WRITE;

		u32 counter = 0;
		do
		{
			if((counter & 31) == 0) nspiWaitFifoBusy();
			nspi->fifo = *in++;
			counter += 4;
		} while(counter < inSize);

		nspiWaitBusy();
	}
	if(out)
	{
		nspi->blklen = outSize;
		nspi->cnt = cntParams | NSPI_DIR_READ;

		u32 counter = 0;
		do
		{
			if((counter & 31) == 0) nspiWaitFifoBusy();
			*out++ = nspi->fifo;
			counter += 4;
		} while(counter < outSize);

		nspiWaitBusy();
	}
}

void SPICARD_deselect(void)
{
	getNspiRegs()->cs = NSPI_DESELECT;
}
