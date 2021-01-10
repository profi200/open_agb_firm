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
#include "hardware/corelink_dma-330.h"
#include "hardware/cache.h"


//#define USE_NEW_CDMA 1

#ifdef ARM11
#ifdef USE_NEW_CDMA
#define DMA330_REGS_BASE          (IO_MEM_ARM11_ONLY + 0x6000)
#else
#define DMA330_REGS_BASE          (IO_MEM_ARM11_ONLY + 0x0000)
#endif // ifdef USE_NEW_CDMA
#elif ARM9
#define DMA330_REGS_BASE          (IO_MEM_ARM9_ONLY + 0xC000)
#endif // ifdef ARM11
#define REG_DMA330_DSR            *((const vu32*)(DMA330_REGS_BASE + 0x000)) // DMA Manager Status Register.
#define REG_DMA330_DPC            *((const vu32*)(DMA330_REGS_BASE + 0x004)) // DMA Program Counter Register (manager).
#define REG_DMA330_INTEN          *((      vu32*)(DMA330_REGS_BASE + 0x020)) // Interrupt Enable Register.
#define REG_DMA330_INT_EVENT_RIS  *((const vu32*)(DMA330_REGS_BASE + 0x024)) // Event-Interrupt Raw Status Register.
#define REG_DMA330_INTMIS         *((const vu32*)(DMA330_REGS_BASE + 0x028)) // Interrupt Status Register.
#define REG_DMA330_INTCLR         *((      vu32*)(DMA330_REGS_BASE + 0x02C)) // Interrupt Clear Register (write-only).
#define REG_DMA330_FSRD           *((const vu32*)(DMA330_REGS_BASE + 0x030)) // Fault Status DMA Manager Register.
#define REG_DMA330_FSRC           *((const vu32*)(DMA330_REGS_BASE + 0x034)) // Fault Status DMA Channel Register.
#define REG_DMA330_FTRD           *((const vu32*)(DMA330_REGS_BASE + 0x038)) // Fault Type DMA Manager Register.

// Fault Type DMA Channel 0-7 Registers.
#define REG_DMA330_FTR(n)         *((const vu32*)(DMA330_REGS_BASE + 0x040 + ((n) * 4)))

// Channel Status 0-7 Registers.
#define REG_DMA330_CSR(n)         *((const vu32*)(DMA330_REGS_BASE + 0x100 + ((n) * 8)))

// Channel Program Counter 0-7 Registers.
#define REG_DMA330_CPC(n)         *((const vu32*)(DMA330_REGS_BASE + 0x104 + ((n) * 8)))

// Source Address 0-7 Registers.
#define REG_DMA330_SAR(n)         *((const vu32*)(DMA330_REGS_BASE + 0x400 + ((n) * 0x20)))

// Destination Address 0-7 Registers.
#define REG_DMA330_DAR(n)         *((const vu32*)(DMA330_REGS_BASE + 0x404 + ((n) * 0x20)))

// Channel Control 0-7 Registers.
#define REG_DMA330_CCR(n)         *((const vu32*)(DMA330_REGS_BASE + 0x408 + ((n) * 0x20)))

// Loop Counter 0 0-7 Registers.
#define REG_DMA330_LC0_(n)        *((const vu32*)(DMA330_REGS_BASE + 0x40C + ((n) * 0x20)))

// Loop Counter 1 0-7 Registers.
#define REG_DMA330_LC1_(n)        *((const vu32*)(DMA330_REGS_BASE + 0x410 + ((n) * 0x20)))

#define REG_DMA330_DBGSTATUS      *((const vu32*)(DMA330_REGS_BASE + 0xD00)) // Debug Status Register.
#define REG_DMA330_DBGCMD         *((      vu32*)(DMA330_REGS_BASE + 0xD04)) // Debug Command Register (write-only).
#define REG_DMA330_DBGINST0       *((      vu32*)(DMA330_REGS_BASE + 0xD08)) // Debug Instruction-0 Register (write-only).
#define REG_DMA330_DBGINST1       *((      vu32*)(DMA330_REGS_BASE + 0xD0C)) // Debug Instruction-1 Register (write-only).

// Configuration Registers.
#define REG_DMA330_CR0            *((const vu32*)(DMA330_REGS_BASE + 0xE00))
#define REG_DMA330_CR1            *((const vu32*)(DMA330_REGS_BASE + 0xE04))
#define REG_DMA330_CR2            *((const vu32*)(DMA330_REGS_BASE + 0xE08))
#define REG_DMA330_CR3            *((const vu32*)(DMA330_REGS_BASE + 0xE0C))
#define REG_DMA330_CR4            *((const vu32*)(DMA330_REGS_BASE + 0xE10))
#define REG_DMA330_CRD            *((const vu32*)(DMA330_REGS_BASE + 0xE14)) // DMA Configuration Register.

// Watchdog Register (r1p0 only).
#define REG_DMA330_WD             *((      vu32*)(DMA330_REGS_BASE + 0xE80))

// Periphal and component identification registers.
#define REG_DMA330_PERIPH_ID_0    *((const vu32*)(DMA330_REGS_BASE + 0xFE0))
#define REG_DMA330_PERIPH_ID_1    *((const vu32*)(DMA330_REGS_BASE + 0xFE4))
#define REG_DMA330_PERIPH_ID_2    *((const vu32*)(DMA330_REGS_BASE + 0xFE8))
#define REG_DMA330_PERIPH_ID_3    *((const vu32*)(DMA330_REGS_BASE + 0xFEC))
#define REG_DMA330_PCELL_ID_0     *((const vu32*)(DMA330_REGS_BASE + 0xFF0))
#define REG_DMA330_PCELL_ID_1     *((const vu32*)(DMA330_REGS_BASE + 0xFF4))
#define REG_DMA330_PCELL_ID_2     *((const vu32*)(DMA330_REGS_BASE + 0xFF8))
#define REG_DMA330_PCELL_ID_3     *((const vu32*)(DMA330_REGS_BASE + 0xFFC))


#ifdef ARM11
#ifdef USE_NEW_CDMA
#error "TODO: New3DS CDMA"
#else
#define CHANNELS   (8u)
#define PERIPHALS  (18u)
#define IRQ_LINES  (9u) // The controller reports 16 but we only have 9 physical lines.
#endif // ifdef USE_NEW_CDMA
#elif ARM9
#define CHANNELS   (4u)
#define PERIPHALS  (8u)
#define IRQ_LINES  (12u)
#endif // ifdef ARM11

#define INTEN_VAL  ((1u<<IRQ_LINES) - 1) // Not 32 bit safe!



static inline void waitForChannelStatus(u8 ch, u8 status)
{
	while((REG_DMA330_CSR(ch) & CSR_STAT_MASK) != status);
}

static inline void waitDebugBusy(void)
{
	while(REG_DMA330_DBGSTATUS & DBGSTATUS_BUSY);
}

static void sendDebugCmd(u32 inst0, u32 inst1)
{
	waitDebugBusy();
	REG_DMA330_DBGINST0 = inst0;
	REG_DMA330_DBGINST1 = inst1;
	REG_DMA330_DBGCMD = DBGCMD_EXECUTE;
}

void DMA330_init(void)
{
	static bool inited = false;
	if(inited) return;
	inited = true;

	// Kill manager thread.
	sendDebugCmd(DBGINST0(0x01u, 0, DBGINST0_THR_MGR), 0);

	// Kill all channels.
	for(u32 i = 0; i < CHANNELS; i++)
	{
		// DMAKILL channel.
		sendDebugCmd(DBGINST0(0x01u, i, DBGINST0_THR_CH), 0);
	}
	waitForChannelStatus(CHANNELS - 1, CSR_STAT_STOPPED);

	REG_DMA330_INTEN = INTEN_VAL;
	REG_DMA330_INTCLR = 0xFFFFFFFF; // Clear all interrupts.
	REG_DMA330_WD = 0;              // Watchdog aborts hanging channels.

	if(PERIPHALS > 0)
	{
#ifdef ARM11
		u16 progBuf[33]; // Max 32 periphals + 1 for DMAEND.
#elif ARM9
		u16 *progBuf = (u16*)(A9_RAM_BASE + A9_RAM_SIZE - 33 * 2); // ARM9 DTCM stack workaround.
#endif // ifdef ARM11
		for(u32 i = 0; i < PERIPHALS; i++)
		{
			// DMAFLUSHP i.
			progBuf[i] = i<<11 | 0x35u;
		}
		progBuf[PERIPHALS] = 0; // DMAEND.
		cleanDCacheRange(progBuf, 33 * 2);
		// DMAGO channel 0 non-secure.
		sendDebugCmd(DBGINST0(0u<<8 | 0xA2u, 0, DBGINST0_THR_MGR), (u32)progBuf);
		waitForChannelStatus(0, CSR_STAT_STOPPED); // Wait for IRQ instead?
	}
}

u8 DMA330_run(u8 ch, const u8 *const prog)
{
	u8 status;
	if((status = (REG_DMA330_CSR(ch) & CSR_STAT_MASK)) != CSR_STAT_STOPPED)
		return status;

	// DMAGO non-secure.
	sendDebugCmd(DBGINST0(ch<<8 | 0xA2u, 0, DBGINST0_THR_MGR), (u32)prog);

	return status;
}

u8 DMA330_status(u8 ch)
{
	return REG_DMA330_CSR(ch) & CSR_STAT_MASK;
}

void DMA330_ackIrq(u8 eventIrq)
{
	REG_DMA330_INTCLR = INTCLR_IRQ_CLR(eventIrq);
}

void DMA330_sev(u8 event)
{
	// DMASEV.
	sendDebugCmd(DBGINST0(event<<11 | 0x34u, 0, DBGINST0_THR_MGR), 0);
}

void DMA330_kill(u8 ch)
{
	if((REG_DMA330_CSR(ch) & CSR_STAT_MASK) != CSR_STAT_STOPPED)
	{
		sendDebugCmd(DBGINST0(0x01u, ch, DBGINST0_THR_CH), 0);
		waitForChannelStatus(ch, CSR_STAT_STOPPED);
	}
}

/*#ifdef ARM11
#include "arm11/fmt.h"
void DMA330_dbgPrint(void)
{
	ee_printf("DSR: %08lX FTRD: %08lX\n", REG_DMA330_DSR, REG_DMA330_FTRD);
	for(u32 i = 0; i < CHANNELS; i++)
	{
		ee_printf(" CSR/FTR%lu: %08lX %08lX\n", i, REG_DMA330_CSR(i), REG_DMA330_FTR(i));
	}
}
#endif*/ // ifdef ARM11
