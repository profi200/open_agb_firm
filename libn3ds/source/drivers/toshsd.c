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
#ifdef _3DS
#ifdef ARM9
#include "arm9/drivers/interrupt.h"
#include "arm9/drivers/cfg9.h"
#elif ARM11
#include "arm11/drivers/interrupt.h"
#endif // #ifdef ARM9
#elif TWL
#error "Missing TWL includes."
#endif // #ifdef _3DS


static u32 g_status[2] = {0};



ALWAYS_INLINE u8 portNum2Controller(const u8 portNum)
{
	return portNum / 2u;
}

ALWAYS_INLINE u8 irq2controller(const u32 id)
{
#ifdef _3DS
#ifdef ARM9
	return (id == IRQ_TOSHSD1 ? 0u : 1u);
#elif ARM11
	return (id == IRQ_TOSHSD2 ? 0u : 1u);
#endif // #ifdef ARM9
#elif TWL
	#error "Unimplemented TWL IRQ to controller ID function."
#endif // #ifdef _3DS
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
// TODO: Move most of these #ifs to toshsd_config.h.
#if (_3DS && ARM9)
	// Note: The power bits don't affect regular card detect. Port remapping does.
	// TODO: Can we switch controllers/ports glitch-free?
	const u32 cardPort = (TOSHSD_CARD_PORT == 2u ? SDMMCCTL_SLOT_TOSHSD3_SEL : SDMMCCTL_SLOT_TOSHSD1_SEL);
	const u32 c2Map = (TOSHSD_C2_MAP == 1u ? SDMMCCTL_TOSHSD3_MAP11 : SDMMCCTL_TOSHSD3_MAP9);
	getCfg9Regs()->sdmmcctl = cardPort | c2Map | SDMMCCTL_UNK_BIT6 | SDMMCCTL_UNK_PWR_OFF;
#endif // #if (_3DS && ARM9)

	atomic_store_explicit(&g_status[0], 0, memory_order_relaxed);
	atomic_store_explicit(&g_status[1], 0, memory_order_relaxed);

	// TODO: 3DS: Do we get controller 3 IRQs on the side the controller is NOT mapped to?
#ifdef _3DS
#ifdef ARM9
	IRQ_registerIsr(IRQ_TOSHSD1, toshsdIsr);
	IRQ_registerIsr(IRQ_TOSHSD3, toshsdIsr);
#elif ARM11
	IRQ_registerIsr(IRQ_TOSHSD2, 14, 0, toshsdIsr);
	IRQ_registerIsr(IRQ_TOSHSD3, 14, 0, toshsdIsr);
#endif // #ifdef ARM9
#elif TWL
	#error "Unimplemented TWL register ISR."
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
		regs->sd_clk_ctrl        = SD_CLK_DIV_128; // TODO: Different default on 3DS.
		regs->sd_blocklen        = 512;
		regs->sd_option          = OPTION_BUS_WIDTH1 | OPTION_UNK14 | 0xE9; // ~7 ms card detection time.
		regs->ext_cdet_mask      = EXT_CDET_MASK_ALL;
		regs->ext_cdet_dat3_mask = EXT_CDET_DAT3_MASK_ALL;

		// SDIO init here?
	}
}

// TODO: Powerdown argument?
void TOSHSD_deinit(void)
{
	// TODO: Should we unregister the SDIO IRQs too or leave it external like registering?
#ifdef _3DS
#ifdef ARM9
	IRQ_unregisterIsr(IRQ_TOSHSD1);
	IRQ_unregisterIsr(IRQ_TOSHSD3);

	getCfg9Regs()->sdmmcctl = SDMMCCTL_UNK_BIT6 | SDMMCCTL_UNK_PWR_OFF | SDMMCCTL_SLOT_PWR_OFF;
#elif ARM11
	IRQ_unregisterIsr(IRQ_TOSHSD2);
	IRQ_unregisterIsr(IRQ_TOSHSD3);
#endif // #ifdef ARM9
#elif TWL
	#error "Unimplemented TWL register ISR."
#endif // #ifdef _3DS
}

void TOSHSD_initPort(ToshsdPort *const port, const u8 portNum)
{
	// Reset port state.
	port->portNum     = portNum;
	port->sd_clk_ctrl = SD_CLK_DIV_128; // TODO: Different default on 3DS.
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
	return getToshsdRegs(portNum2Controller(TOSHSD_CARD_PORT))->sd_status & STATUS_DETECT;
}

bool TOSHSD_cardSliderUnlocked(void)
{
	return getToshsdRegs(portNum2Controller(TOSHSD_CARD_PORT))->sd_status & STATUS_NO_WRPROT;
}

// TODO: Clock in Hz?
// TODO: This might be a little dodgy not using setPort() before changing clock.
//       It's fine as long as only one port is used per controller
//       and there is no concurrent access to it.
void TOSHSD_setClockImmediately(ToshsdPort *const port, u16 clk)
{
	clk |= SD_CLK_EN;
	port->sd_clk_ctrl = clk;
	getToshsdRegs(portNum2Controller(port->portNum))->sd_clk_ctrl = clk;
}

static void getResponse(const Toshsd *const regs, ToshsdPort *const port, const u16 cmd)
{
	// We could check for response type none as well but it's not worth it.
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
		port->resp[3] = resp[0]<<8; // TODO: Add the missing CRC7 and bit 0?
	}
}

// Note: Using STATUS_DATA_END to detect transfer end doesn't work reliably
//       because STATUS_DATA_END fires before we even read anything on
//       single block read transfer.
// TODO: Can the controller send data end early and should we treat it as an error?
static void doCpuTransfer(Toshsd *const regs, const u16 cmd, u32 *buf, const u32 *const statusPtr)
{
	const u32 blockLen = regs->sd_blocklen; // gcc generates an invisible "& ~3u" here. Why?
	u32 blockCount = regs->sd_blockcount;
	vu32 *const fifo = getToshsdFifo(regs);
	if(cmd & CMD_DIR_R)
	{
		while((atomic_load_explicit(statusPtr, memory_order_relaxed) & STATUS_MASK_ERR) == 0 && blockCount)
		{
			__wfi();
			if(regs->sd_fifo32_cnt & FIFO32_FULL) // RX ready.
			{
				const u32 *const blockEnd = buf + (blockLen / 4u);
				do
				{
					buf[0] = *fifo;
					buf[1] = *fifo;
					buf[2] = *fifo;
					buf[3] = *fifo;

					buf += 4u;
				} while(buf < blockEnd);

				blockCount--;
			}
		}
	}
	else
	{
		// TODO: Write first block ahead of time?
		// gbatek Command/Param/Response/Data at bottom of page.
		while((atomic_load_explicit(statusPtr, memory_order_relaxed) & STATUS_MASK_ERR) == 0 && blockCount)
		{
			__wfi();
			if(!(regs->sd_fifo32_cnt & FIFO32_NOT_EMPTY)) // TX request.
			{
				const u32 *const blockEnd = buf + (blockLen / 4u);
				do
				{
					*fifo = buf[0];
					*fifo = buf[1];
					*fifo = buf[2];
					*fifo = buf[3];

					buf += 4u;
				} while(buf < blockEnd);

				blockCount--;
			}
		}
	}
}

// TODO: Test if driver hangs removing the SD on read and write transfers.
//       Read: Tested, no hang but may need a few seconds for removal detection.
//       Write: Untested. Destructive!
// TODO: What if we get rid of setPort() and only use one port per controller?
u32 TOSHSD_sendCommand(ToshsdPort *const port, const u16 cmd, const u32 arg)
{
	const u8 controller = portNum2Controller(port->portNum);
	Toshsd *const regs = getToshsdRegs(controller);

	setPort(regs, port);
	regs->sd_blockcount   = port->blocks; // sd_blockcount32 doesn't need to be set for the 32 bit FIFO to work.
	regs->sd_stop         = ((cmd & CMD_MBT) ? STOP_AUTO_STOP : 0u); // Auto CMD12 on multi-block transfer.
	regs->sd_arg          = arg;

	u32 *buf = port->buf;
	const u16 fifoIrqs = (buf != NULL ? (cmd & CMD_DIR_R ? FIFO32_FULL_IE : FIFO32_NOT_EMPTY_IE) : 0u);
	regs->sd_fifo32_cnt   = fifoIrqs | FIFO32_CLEAR | FIFO32_EN;
	regs->sd_cmd          = cmd; // Start.

	// If we have to transfer data do so now. buf = NULL means DMA.
	u32 *const statusPtr = &g_status[controller];
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

	const u32 res = atomic_load_explicit(statusPtr, memory_order_relaxed) & STATUS_MASK_ERR;
	atomic_store_explicit(statusPtr, 0, memory_order_relaxed);
	return res;
}
