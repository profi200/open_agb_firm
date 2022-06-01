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

#include <stdatomic.h>
#include "types.h"
#include "drivers/toshsd.h"
#include "drivers/toshsd_config.h"


static u32 g_status[2] = {0};



ALWAYS_INLINE u8 port2Controller(const u8 portNum)
{
	return portNum / 2;
}

ALWAYS_INLINE u8 irq2controller(const u32 id)
{
	return (id == TOSHSD_IRQ_ID_CONTROLLER1 ? 0 : 1);
}

static void toshsdIsr(const u32 id)
{
	const u8 controller = irq2controller(id);
	Toshsd *const regs = getToshsdRegs(controller);

	g_status[controller] |= regs->sd_status;
	regs->sd_status = STATUS_CMD_BUSY; // Never acknowledge STATUS_CMD_BUSY.

	// TODO: Some kind of event to notify the main loop for remove/insert.
}

void TOSHSD_init(void)
{
	// Do controller and port mapping (see toshsd_config.h).
	TOSHSD_MAP_CONTROLLERS();

	// Register ISR and enable IRQs.
	// IRQs are only fired on the side a controller is mapped to.
	TOSHSD_REGISTER_ISR(toshsdIsr);

	// Reset all controllers.
	for(u32 i = 0; i < TOSHSD_NUM_CONTROLLERS; i++)
	{
		// Setup 32 bit FIFO.
		Toshsd *const regs = getToshsdRegs(i);
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
		regs->sd_clk_ctrl        = SD_CLK_DEFAULT;
		regs->sd_blocklen        = 512;
		regs->sd_option          = OPTION_BUS_WIDTH1 | OPTION_UNK14 | OPTION_DEFAULT_TIMINGS;
		regs->ext_cdet_mask      = EXT_CDET_MASK_ALL;
		regs->ext_cdet_dat3_mask = EXT_CDET_DAT3_MASK_ALL;

		// Disable SDIO.
		regs->sdio_mode        = 0;
		regs->sdio_status_mask = SDIO_STATUS_MASK_ALL;
		regs->ext_sdio_irq     = EXT_SDIO_IRQ_MASK_ALL;
	}
}

void TOSHSD_deinit(void)
{
	// Unregister ISR and disable IRQs.
	TOSHSD_UNREGISTER_ISR();

	// Mask all IRQs.
	for(u32 i = 0; i < TOSHSD_NUM_CONTROLLERS; i++)
	{
		// 32 bit FIFO IRQs.
		Toshsd *const regs = getToshsdRegs(i);
		regs->sd_fifo32_cnt = 0; // FIFO and all IRQs disabled/masked.

		// Regular IRQs.
		regs->sd_status_mask = STATUS_MASK_ALL;

		// SDIO IRQs.
		regs->sdio_status_mask = SDIO_STATUS_MASK_ALL;
	}

	// Reset controller and port mapping (see toshsd_config.h).
	TOSHSD_UNMAP_CONTROLLERS();
}

void TOSHSD_initPort(ToshsdPort *const port, const u8 portNum)
{
	// Reset port state.
	port->portNum     = portNum;
	port->sd_clk_ctrl = SD_CLK_DEFAULT;
	port->sd_blocklen = 512;
	port->sd_option   = OPTION_BUS_WIDTH1 | OPTION_UNK14 | OPTION_DEFAULT_TIMINGS;
}

// TODO: What if we get rid of setPort() and only use one port per controller?
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
	return getToshsdRegs(port2Controller(TOSHSD_CARD_PORT))->sd_status & STATUS_DETECT;
}

bool TOSHSD_cardWritable(void)
{
	return getToshsdRegs(port2Controller(TOSHSD_CARD_PORT))->sd_status & STATUS_NO_WRPROT;
}

// TODO: Clock in Hz?
// TODO: This might be a little dodgy not using setPort() before changing clock.
//       It's fine as long as only one port is used per controller
//       and there is no concurrent access to it.
void TOSHSD_setClockImmediately(ToshsdPort *const port, u16 clk)
{
	clk |= SD_CLK_EN;
	port->sd_clk_ctrl = clk;
	getToshsdRegs(port2Controller(port->portNum))->sd_clk_ctrl = clk;
}

static void getResponse(const Toshsd *const regs, ToshsdPort *const port, const u16 cmd)
{
	// We could check for response type none as well but it's not worth it.
	if((cmd & CMD_RESP_MASK) != CMD_RESP_R2)
	{
		port->resp[0] = regs->sd_resp[0];
	}
	else // 136 bit R2 responses need special treatment...
	{
		u32 resp[4];
		for(u32 i = 0; i < 4; i++) resp[i] = regs->sd_resp[i];

		port->resp[0] = resp[3]<<8 | resp[2]>>24;
		port->resp[1] = resp[2]<<8 | resp[1]>>24;
		port->resp[2] = resp[1]<<8 | resp[0]>>24;
		port->resp[3] = resp[0]<<8; // TODO: Add the missing CRC7 and bit 0?
	}
}

// Note: Using STATUS_DATA_END to detect transfer end doesn't work reliably
//       because STATUS_DATA_END fires before we even read anything on
//       single block read transfer.
static void doCpuTransfer(Toshsd *const regs, const u16 cmd, u32 *buf, const u32 *const statusPtr)
{
	const u32 blockLen = regs->sd_blocklen;
	u32 blockCount = regs->sd_blockcount;
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
		} while((atomic_load_explicit(statusPtr, memory_order_relaxed) & STATUS_MASK_ERR) == 0 && blockCount);
	}
	else
	{
		// TODO: Write first block ahead of time?
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
		} while((atomic_load_explicit(statusPtr, memory_order_relaxed) & STATUS_MASK_ERR) == 0 && blockCount);
	}
}

u32 TOSHSD_sendCommand(ToshsdPort *const port, const u16 cmd, const u32 arg)
{
	const u8 controller = port2Controller(port->portNum);
	Toshsd *const regs = getToshsdRegs(controller);

	// Clear status before sending another command.
	u32 *const statusPtr = &g_status[controller];
	atomic_store_explicit(statusPtr, 0, memory_order_relaxed);

	setPort(regs, port);
	regs->sd_blockcount = port->blocks;                           // sd_blockcount32 doesn't need to be set.
	regs->sd_stop       = ((cmd & CMD_MBT) ? STOP_AUTO_STOP : 0); // Auto CMD12 on multi-block transfer.
	regs->sd_arg        = arg;

	// We don't need FIFO IRQs when using DMA. buf = NULL means DMA.
	u32 *buf = port->buf;
	const u16 fifoIrqs = (buf != NULL ? (cmd & CMD_DIR_R ? FIFO32_FULL_IE : FIFO32_NOT_EMPTY_IE) : 0u);
	regs->sd_fifo32_cnt = fifoIrqs | FIFO32_CLEAR | FIFO32_EN;
	regs->sd_cmd        = cmd; // Start.

	// If we have to transfer data do so now.
	if((cmd & CMD_DT_EN) && (buf != NULL))
		doCpuTransfer(regs, cmd, buf, statusPtr);

	// Response end usually comes immediately after the command
	// has been sent so we need to check before __wfi().
	// On error response end still fires.
	while(!(atomic_load_explicit(statusPtr, memory_order_relaxed) & STATUS_RESP_END)) __wfi();
	getResponse(regs, port, cmd);

	// Wait for data end if needed.
	// On error data end still fires.
	if(cmd & CMD_DT_EN)
		while(!(atomic_load_explicit(statusPtr, memory_order_relaxed) & STATUS_DATA_END)) __wfi();

	// STATUS_CMD_BUSY is no longer set at this point.

	return atomic_load_explicit(statusPtr, memory_order_relaxed) & STATUS_MASK_ERR;
}
