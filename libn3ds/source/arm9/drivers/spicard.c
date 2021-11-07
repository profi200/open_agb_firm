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
#include "arm9/drivers/spicard.h"
#include "arm9/drivers/cfg9.h"
#include "arm9/drivers/interrupt.h"
#include "arm9/drivers/timer.h"
#include "arm9/drivers/ndma.h"



static inline void spicWaitBusy(const Spic *const spic)
{
	while(spic->cnt & SPIC_EN);
}

static inline void spicWaitFifoBusy(const Spic *const spic)
{
	while(spic->fifo_stat & SPIC_FIFO_BUSY);
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

	cfg9->cardctl = CARDCTL_SPIC_SEL | CARDCTL_NTRCARD;

	IRQ_registerIsr(IRQ_CTR_CARD_1, NULL);
	Spic *const spic = getSpicRegs();
	spic->int_mask = SPIC_INT_TRAN_END; // Disable interrupt 1.
	spic->int_stat = SPIC_INT_AP_TMOUT | SPIC_INT_AP_MATCH | SPIC_INT_TRAN_END; // Acknowledge.
}

bool SPICARD_autoPollBit(SpicClk clk, u32 apParams)
{
	Spic *const spic = getSpicRegs();
	spic->cnt      = clk & 7u;
	spic->autopoll = SPIC_AP_START | apParams;

	u32 res;
	do
	{
		__wfi();
		res = spic->int_stat;
	} while(!(res & (SPIC_INT_AP_TMOUT | SPIC_INT_AP_MATCH)));
	spic->int_stat = res; // Acknowledge.

	return (res & SPIC_INT_AP_TMOUT) == 0; // Timeout error.
}

void SPICARD_writeRead(SpicClk clk, const u32 *in, u32 *out, u32 inSize, u32 outSize)
{
	const u32 cntParams = SPIC_EN | SPIC_BUS_1BIT | (clk & 7u);

	Spic *const spic = getSpicRegs();
	if(in)
	{
		spic->blklen = inSize;
		spic->cnt = cntParams | SPIC_DIR_W;

		u32 counter = 0;
		do
		{
			if((counter & 31) == 0) spicWaitFifoBusy(spic);
			spic->fifo = *in++;
			counter += 4;
		} while(counter < inSize);

		spicWaitBusy(spic);
	}
	if(out)
	{
		spic->blklen = outSize;
		spic->cnt = cntParams | SPIC_DIR_R;

		u32 counter = 0;
		do
		{
			if((counter & 31) == 0) spicWaitFifoBusy(spic);
			*out++ = spic->fifo;
			counter += 4;
		} while(counter < outSize);

		spicWaitBusy(spic);
	}

	if(clk & SPIC_CLK_CS_HIGH) spic->cs = SPIC_CS_HIGH;
}
