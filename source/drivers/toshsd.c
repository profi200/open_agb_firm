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
#include "drivers/toshsd.h"
#include "drivers/toshsd_config.h"
#ifdef _3DS
#ifdef ARM9
#include "arm9/drivers/interrupt.h"
#include "arm9/drivers/cfg9.h"
#elif ARM11
#include "arm11/drivers/interrupt.h"
#endif // #ifdef ARM9
#elif TWL
// TODO: DSi IRQ stuff.
#endif // #ifdef _3DS



static void toshsdIsr(UNUSED u32 id)
{
	Toshsd *const regs = getToshsdRegs(TOSHSD_SLOT_PORT / 2u);
	regs->sd_status = ~(STATUS_INSERT | STATUS_REMOVE);
	// TODO: Some kind of event to notify the main loop.
}

void TOSHSD_init(void)
{
#if (_3DS && ARM9)
	// Note: The power bits don't affect regular card detect. Port remapping does.
	// TODO: Can we switch controllers/ports glitch-free?
	const u32 slotPort = (TOSHSD_SLOT_PORT == 2u ? SDMMCCTL_SLOT_TOSHSD3_SEL : SDMMCCTL_SLOT_TOSHSD1_SEL);
	const u32 c3Map = (TOSHSD_C3_MAP == 1u ? SDMMCCTL_TOSHSD3_MAP11 : SDMMCCTL_TOSHSD3_MAP9);
	getCfg9Regs()->sdmmcctl = slotPort | c3Map | SDMMCCTL_UNK_BIT6;
#endif // #if (_3DS && ARM9)

	// TODO: 3DS: Do we get controller 3 IRQs on the side the controller is NOT mapped to?
#ifdef _3DS
#ifdef ARM9
	IRQ_registerIsr(IRQ_SDIO_1, NULL);
	IRQ_registerIsr(IRQ_SDIO_3, toshsdIsr);
	// IRQ_SDIO_1_ASYNC not needed.
	// IRQ_SDIO_3_ASYNC not needed.
#elif ARM11
	IRQ_registerIsr(IRQ_TOSHSD2, 14, 0, NULL);
	IRQ_registerIsr(IRQ_TOSHSD3, 14, 0, toshsdIsr);
	//IRQ_registerIsr(IRQ_TOSHSD2_IRQ, 14, 0, toshsdIsr); // TODO: Should we register this externally?
	// IRQ_SDIO3_IRQ not needed.
#endif // #ifdef ARM9
#elif TWL
	// TODO: DSi IRQ stuff.
#endif // #ifdef _3DS

	// Reset all controllers.
	for(u8 i = 0; i < 2; i++) // TODO: 3DS: Don't touch controller 3 if not mapped.
	{
		Toshsd *const regs = getToshsdRegs(i);
		// Setup 32 bit FIFO.
		regs->sd_fifo32_cnt   = FIFO32_CLEAR | FIFO32_EN;
		regs->sd_blocklen32   = 512;
		regs->sd_blockcount32 = 1;
		regs->dma_ext_mode    = DMA_EXT_DMA_MODE;

		// Reset. Unlike similar controllers no delay is needed.
		regs->soft_rst = SOFT_RST_RST;
		regs->soft_rst = SOFT_RST_NORST;

		regs->sd_portsel         = PORTSEL_P0;
		regs->sd_blockcount      = 1;
		regs->sd_status_mask     = STATUS_MASK_DEFAULT;
		regs->sd_clk_ctrl        = SD_CLK_DIV_128;
		regs->sd_blocklen        = 512;
		regs->sd_option          = OPTION_BUS_WIDTH1 | OPTION_UNK14 | 0xE9; // ~7 ms card detection time.
		regs->ext_cdet_mask      = EXT_CDET_MASK_ALL;
		regs->ext_cdet_dat3_mask = EXT_CDET_DAT3_MASK_ALL;

		// SDIO init here?
	}
}

void TOSHSD_deinit(void)
{
	for(u8 i = 0; i < 2; i++) // TODO: 3DS: Don't touch controller 3 if not mapped.
	{
		Toshsd *const regs = getToshsdRegs(i);

		// TODO: Handle this differently. The last block of the previous
		//       write transfer may still be in progress.
		// TODO: Why is waiting for CMD_BUSY getting cleared not enough?
		//       Hangs on 2 single block writes in a row otherwise.
		while((regs->sd_status & (1u<<29 | STATUS_CMD_BUSY)) != 1u<<29);
	}

#if (_3DS && ARM9)
	getCfg9Regs()->sdmmcctl = SDMMCCTL_UNK_BIT6 | SDMMCCTL_SLOT_PWR_OFF;
#endif // #if (_3DS && ARM9)
}

void TOSHSD_initPort(ToshsdPort *const port, u8 portNum)
{
	// Reset port state.
	port->portNum     = portNum;
	port->sd_clk_ctrl = SD_CLK_DIV_128;
	port->sd_blocklen = 512;
	port->sd_option   = OPTION_BUS_WIDTH1 | OPTION_UNK14 | 0xE9;
}

static void setPort(Toshsd *const regs, const ToshsdPort *const port)
{
	// TODO: Can we somehow prevent all these reg writes each time?
	//       Maybe some kind of dirty flag + active port check?
	regs->sd_portsel    = port->portNum % 2u;
	regs->sd_clk_ctrl   = port->sd_clk_ctrl;
	const u16 blocklen = port->sd_blocklen;
	regs->sd_blocklen   = blocklen;
	regs->sd_option     = port->sd_option;
	regs->sd_blocklen32 = blocklen;
}

bool TOSHSD_cardDetected(void)
{
	return getToshsdRegs(TOSHSD_SLOT_PORT / 2u)->sd_status & STATUS_DETECT;
}

bool TOSHSD_cardSliderUnlocked(void)
{
	return getToshsdRegs(TOSHSD_SLOT_PORT / 2u)->sd_status & STATUS_NO_WRPROT;
}

// TODO: Clock in Hz?
void TOSHSD_setClock(ToshsdPort *const port, u16 clk)
{
	// On SD/MMC init we have to permanently turn on clock
	// for a while so this needs to immediately take effect.
	port->sd_clk_ctrl = clk;
	getToshsdRegs(port->portNum / 2u)->sd_clk_ctrl = clk;
}

static void getResponse(Toshsd *const regs, ToshsdPort *const port, u16 cmd)
{
	if((cmd & CMD_RESP_MASK) != CMD_RESP_R2)
	{
		port->resp[0] = regs->sd_resp[0];
	}
	else // 136 bit responses need special treatment...
	{
		u32 resp[4];
		for(u32 i = 0; i < 4; i++) resp[i] = regs->sd_resp[i];

		port->resp[0] = resp[3]<<8 | resp[2]>>24;
		port->resp[1] = resp[2]<<8 | resp[1]>>24;
		port->resp[2] = resp[1]<<8 | resp[0]>>24;
		port->resp[3] = resp[0]<<8; // TODO: Add the missing CRC7 and the always 1 bit?
	}
}

static void doCpuTransfer(Toshsd *const regs, u16 cmd, u32 *buf)
{
	const u32 blockLen = regs->sd_blocklen; // | TODO: GCC adds a weird & to one of these 2.
	u32 blockCount = regs->sd_blockcount;   // |
	vu32 *const fifo = getToshsdFifo(regs);
	if(cmd & CMD_DIR_R)
	{
		do
		{
			__wfi();
			if(regs->sd_fifo32_cnt & FIFO32_FULL) // RX ready.
			{
				const u32 *const blockEnd = buf + (blockLen / 4);
				do
				{
					buf[0] = *fifo;
					buf[1] = *fifo;
					buf[2] = *fifo;
					buf[3] = *fifo;

					buf += 4;
				} while(buf < blockEnd);

				blockCount--;
			}
		// TODO: Check detect bit (needs insert/remove IRQ handling).
		// TODO: Use DATA_END IRQ instead of counter?
		} while((regs->sd_status & STATUS_MASK_ERR) == 0 && blockCount);
	}
	else
	{
		// TODO: Write first block ahead of time.
		// gbatek Command/Param/Response/Data at bottom of page.
		do
		{
			__wfi();
			if(!(regs->sd_fifo32_cnt & FIFO32_NOT_EMPTY)) // TX request.
			{
				const u32 *const blockEnd = buf + (blockLen / 4);
				do
				{
					*fifo = buf[0];
					*fifo = buf[1];
					*fifo = buf[2];
					*fifo = buf[3];

					buf += 4;
				} while(buf < blockEnd);

				blockCount--;
			}
		// TODO: Check detect bit (needs insert/remove IRQ handling).
		// TODO: Use DATA_END IRQ instead of counter? This may be difficult for write
		//       if we want the last block to be processed in background.
		} while((regs->sd_status & STATUS_MASK_ERR) == 0 && blockCount);
	}
}

u32 TOSHSD_sendCommand(ToshsdPort *const port, u16 cmd, u32 arg)
{
	Toshsd *const regs = getToshsdRegs(port->portNum / 2u); // TODO: gcc generates a udf instruction for this line.

	// TODO: Handle this differently. The last block of the previous
	//       write transfer may still be in progress.
	// TODO: Why is waiting for CMD_BUSY getting cleared not enough?
	//       Hangs on 2 single block writes in a row otherwise.
	while((regs->sd_status & (1u<<29 | STATUS_CMD_BUSY)) != 1u<<29);

	setPort(regs, port);
	regs->sd_blockcount   = port->blocks;
	// sd_blockcount32 doesn't need to be set for the 32 bit FIFO to work.
	regs->sd_stop         = ((cmd & CMD_MBT) ? STOP_AUTO_STOP : 0); // TODO: Works with SDIO?
	regs->sd_arg          = arg;
	regs->sd_fifo32_cnt   = ((cmd & CMD_DIR_R) ? FIFO32_FULL_IE : FIFO32_NOT_EMPTY_IE) |
	                        FIFO32_CLEAR | FIFO32_EN;
	regs->sd_cmd          = cmd; // Start

	u32 *buf = port->buf;
	// Check for data transfer and if buf is NULL (NULL means DMA).
	if((cmd & CMD_DT) && (buf != NULL))
		doCpuTransfer(regs, cmd, buf);

	// On multi-block read transfer response end fires
	// while reading the last block from FIFO
	// so we need to check before __wfi().
	while(!(regs->sd_status & STATUS_RESP_END)) __wfi();
	getResponse(regs, port, cmd);

	const u32 status = regs->sd_status & STATUS_MASK_ERR;
	regs->sd_status = STATUS_CMD_BUSY; // Acknowledge all but CMD busy.
	return status;
}

#if (_3DS && ARM11)
#include "arm11/fmt.h"
void TOSHSD_dbgPrint(ToshsdPort *const port)
{
	Toshsd *const regs = getToshsdRegs(port->portNum / 2u); // TODO: gcc generates a udf instruction for this line.

	ee_printf("Toshsd last cmd: %u\n"
	          " sd_status: 0x%lX\n"
	          " sd_err_status: 0x%lX\n",
	          regs->sd_cmd & 0xFFu,
	          regs->sd_status,
	          regs->sd_err_status);
	ee_printf(" sd_portsel: 0x%X\n"
	          " sd_clk_ctrl: 0x%X\n"
	          " sd_option: 0x%X\n"
	          " resp: ",
	          regs->sd_portsel,
	          regs->sd_clk_ctrl,
	          regs->sd_option);
	for(u32 i = 0; i < 4; i++) ee_printf("%08lX ", port->resp[i]);
	ee_puts("");
}
#endif // #if (_3DS && ARM11)
