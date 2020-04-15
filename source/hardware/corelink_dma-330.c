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
#include "hardware/corelink_dma-330.h"
//#include "arm9/hardware/cfg9.h"
//#include "arm11/hardware/interrupt.h"
#include "hardware/cache.h"
 //#include "arm11/fmt.h"


#ifdef ARM9
#define DMA330_REGS_BASE          (IO_MEM_ARM9_ONLY + 0xC000)
#elif ARM11
#define DMA330_REGS_BASE          (IO_MEM_ARM11_ONLY + 0x0000)
#endif
#define REG_DMA330_DSR            *((const vu32*)(DMA330_REGS_BASE + 0x000)) // DMA Manager Status Register
#define REG_DMA330_DPC            *((const vu32*)(DMA330_REGS_BASE + 0x004)) // DMA Program Counter Register (manager)
#define REG_DMA330_INTEN          *((      vu32*)(DMA330_REGS_BASE + 0x020)) // Interrupt Enable Register
#define REG_DMA330_INT_EVENT_RIS  *((const vu32*)(DMA330_REGS_BASE + 0x024)) // Event-Interrupt Raw Status Register
#define REG_DMA330_INTMIS         *((const vu32*)(DMA330_REGS_BASE + 0x028)) // Interrupt Status Register
#define REG_DMA330_INTCLR         *((      vu32*)(DMA330_REGS_BASE + 0x02C)) // Interrupt Clear Register (write-only)
#define REG_DMA330_FSRD           *((const vu32*)(DMA330_REGS_BASE + 0x030)) // Fault Status DMA Manager Register
#define REG_DMA330_FSRC           *((const vu32*)(DMA330_REGS_BASE + 0x034)) // Fault Status DMA Channel Register
#define REG_DMA330_FTRD           *((const vu32*)(DMA330_REGS_BASE + 0x038)) // Fault Type DMA Manager Register

// Fault Type DMA Channel Registers
#define REG_DMA330_FTR0           *((const vu32*)(DMA330_REGS_BASE + 0x040))
#define REG_DMA330_FTR1           *((const vu32*)(DMA330_REGS_BASE + 0x044))
#define REG_DMA330_FTR2           *((const vu32*)(DMA330_REGS_BASE + 0x048))
#define REG_DMA330_FTR3           *((const vu32*)(DMA330_REGS_BASE + 0x04C))
#define REG_DMA330_FTR4           *((const vu32*)(DMA330_REGS_BASE + 0x050))
#define REG_DMA330_FTR5           *((const vu32*)(DMA330_REGS_BASE + 0x054))
#define REG_DMA330_FTR6           *((const vu32*)(DMA330_REGS_BASE + 0x058))
#define REG_DMA330_FTR7           *((const vu32*)(DMA330_REGS_BASE + 0x05C))
#define REG_DMA330_FTR(n)         *((const vu32*)(DMA330_REGS_BASE + 0x040 + ((n) * 4)))

// Channel Status Registers
#define REG_DMA330_CSR0           *((const vu32*)(DMA330_REGS_BASE + 0x100))
#define REG_DMA330_CSR1           *((const vu32*)(DMA330_REGS_BASE + 0x108))
#define REG_DMA330_CSR2           *((const vu32*)(DMA330_REGS_BASE + 0x110))
#define REG_DMA330_CSR3           *((const vu32*)(DMA330_REGS_BASE + 0x118))
#define REG_DMA330_CSR4           *((const vu32*)(DMA330_REGS_BASE + 0x120))
#define REG_DMA330_CSR5           *((const vu32*)(DMA330_REGS_BASE + 0x128))
#define REG_DMA330_CSR6           *((const vu32*)(DMA330_REGS_BASE + 0x130))
#define REG_DMA330_CSR7           *((const vu32*)(DMA330_REGS_BASE + 0x138))
#define REG_DMA330_CSR(n)         *((const vu32*)(DMA330_REGS_BASE + 0x100 + ((n) * 8)))

// Channel Program Counter Registers
#define REG_DMA330_CPC0           *((const vu32*)(DMA330_REGS_BASE + 0x104))
#define REG_DMA330_CPC1           *((const vu32*)(DMA330_REGS_BASE + 0x10C))
#define REG_DMA330_CPC2           *((const vu32*)(DMA330_REGS_BASE + 0x114))
#define REG_DMA330_CPC3           *((const vu32*)(DMA330_REGS_BASE + 0x11C))
#define REG_DMA330_CPC4           *((const vu32*)(DMA330_REGS_BASE + 0x124))
#define REG_DMA330_CPC5           *((const vu32*)(DMA330_REGS_BASE + 0x12C))
#define REG_DMA330_CPC6           *((const vu32*)(DMA330_REGS_BASE + 0x134))
#define REG_DMA330_CPC7           *((const vu32*)(DMA330_REGS_BASE + 0x13C))
#define REG_DMA330_CPC(n)         *((const vu32*)(DMA330_REGS_BASE + 0x104 + ((n) * 8)))

// Source Address Registers
#define REG_DMA330_SAR0           *((const vu32*)(DMA330_REGS_BASE + 0x400))
#define REG_DMA330_SAR1           *((const vu32*)(DMA330_REGS_BASE + 0x420))
#define REG_DMA330_SAR2           *((const vu32*)(DMA330_REGS_BASE + 0x440))
#define REG_DMA330_SAR3           *((const vu32*)(DMA330_REGS_BASE + 0x460))
#define REG_DMA330_SAR4           *((const vu32*)(DMA330_REGS_BASE + 0x480))
#define REG_DMA330_SAR5           *((const vu32*)(DMA330_REGS_BASE + 0x4A0))
#define REG_DMA330_SAR6           *((const vu32*)(DMA330_REGS_BASE + 0x4C0))
#define REG_DMA330_SAR7           *((const vu32*)(DMA330_REGS_BASE + 0x4E0))
#define REG_DMA330_SAR(n)         *((const vu32*)(DMA330_REGS_BASE + 0x400 + ((n) * 0x20)))

// Destination Address Registers
#define REG_DMA330_DAR0           *((const vu32*)(DMA330_REGS_BASE + 0x404))
#define REG_DMA330_DAR1           *((const vu32*)(DMA330_REGS_BASE + 0x424))
#define REG_DMA330_DAR2           *((const vu32*)(DMA330_REGS_BASE + 0x444))
#define REG_DMA330_DAR3           *((const vu32*)(DMA330_REGS_BASE + 0x464))
#define REG_DMA330_DAR4           *((const vu32*)(DMA330_REGS_BASE + 0x484))
#define REG_DMA330_DAR5           *((const vu32*)(DMA330_REGS_BASE + 0x4A4))
#define REG_DMA330_DAR6           *((const vu32*)(DMA330_REGS_BASE + 0x4C4))
#define REG_DMA330_DAR7           *((const vu32*)(DMA330_REGS_BASE + 0x4E4))
#define REG_DMA330_DAR(n)         *((const vu32*)(DMA330_REGS_BASE + 0x404 + ((n) * 0x20)))

// Channel Control Registers
#define REG_DMA330_CCR0           *((const vu32*)(DMA330_REGS_BASE + 0x408))
#define REG_DMA330_CCR1           *((const vu32*)(DMA330_REGS_BASE + 0x428))
#define REG_DMA330_CCR2           *((const vu32*)(DMA330_REGS_BASE + 0x448))
#define REG_DMA330_CCR3           *((const vu32*)(DMA330_REGS_BASE + 0x468))
#define REG_DMA330_CCR4           *((const vu32*)(DMA330_REGS_BASE + 0x488))
#define REG_DMA330_CCR5           *((const vu32*)(DMA330_REGS_BASE + 0x4A8))
#define REG_DMA330_CCR6           *((const vu32*)(DMA330_REGS_BASE + 0x4C8))
#define REG_DMA330_CCR7           *((const vu32*)(DMA330_REGS_BASE + 0x4E8))
#define REG_DMA330_CCR(n)         *((const vu32*)(DMA330_REGS_BASE + 0x408 + ((n) * 0x20)))

// Loop Counter 0 Registers
#define REG_DMA330_LC0_0          *((const vu32*)(DMA330_REGS_BASE + 0x40C))
#define REG_DMA330_LC0_1          *((const vu32*)(DMA330_REGS_BASE + 0x42C))
#define REG_DMA330_LC0_2          *((const vu32*)(DMA330_REGS_BASE + 0x44C))
#define REG_DMA330_LC0_3          *((const vu32*)(DMA330_REGS_BASE + 0x46C))
#define REG_DMA330_LC0_4          *((const vu32*)(DMA330_REGS_BASE + 0x48C))
#define REG_DMA330_LC0_5          *((const vu32*)(DMA330_REGS_BASE + 0x4AC))
#define REG_DMA330_LC0_6          *((const vu32*)(DMA330_REGS_BASE + 0x4CC))
#define REG_DMA330_LC0_7          *((const vu32*)(DMA330_REGS_BASE + 0x4EC))
#define REG_DMA330_LC0_(n)        *((const vu32*)(DMA330_REGS_BASE + 0x40C + ((n) * 0x20)))

// Loop Counter 1 Registers
#define REG_DMA330_LC1_0          *((const vu32*)(DMA330_REGS_BASE + 0x410))
#define REG_DMA330_LC1_1          *((const vu32*)(DMA330_REGS_BASE + 0x430))
#define REG_DMA330_LC1_2          *((const vu32*)(DMA330_REGS_BASE + 0x450))
#define REG_DMA330_LC1_3          *((const vu32*)(DMA330_REGS_BASE + 0x470))
#define REG_DMA330_LC1_4          *((const vu32*)(DMA330_REGS_BASE + 0x490))
#define REG_DMA330_LC1_5          *((const vu32*)(DMA330_REGS_BASE + 0x4B0))
#define REG_DMA330_LC1_6          *((const vu32*)(DMA330_REGS_BASE + 0x4D0))
#define REG_DMA330_LC1_7          *((const vu32*)(DMA330_REGS_BASE + 0x4F0))
#define REG_DMA330_LC1_(n)        *((const vu32*)(DMA330_REGS_BASE + 0x410 + ((n) * 0x20)))

#define REG_DMA330_DBGSTATUS      *((const vu32*)(DMA330_REGS_BASE + 0xD00)) // Debug Status Register
#define REG_DMA330_DBGCMD         *((      vu32*)(DMA330_REGS_BASE + 0xD04)) // Debug Command Register (write-only)
#define REG_DMA330_DBGINST0       *((      vu32*)(DMA330_REGS_BASE + 0xD08)) // Debug Instruction-0 Register (write-only)
#define REG_DMA330_DBGINST1       *((      vu32*)(DMA330_REGS_BASE + 0xD0C)) // Debug Instruction-1 Register (write-only)

// Configuration Registers
#define REG_DMA330_CR0            *((const vu32*)(DMA330_REGS_BASE + 0xE00))
#define REG_DMA330_CR1            *((const vu32*)(DMA330_REGS_BASE + 0xE04))
#define REG_DMA330_CR2            *((const vu32*)(DMA330_REGS_BASE + 0xE08))
#define REG_DMA330_CR3            *((const vu32*)(DMA330_REGS_BASE + 0xE0C))
#define REG_DMA330_CR4            *((const vu32*)(DMA330_REGS_BASE + 0xE10))
#define REG_DMA330_CRD            *((const vu32*)(DMA330_REGS_BASE + 0xE14)) // DMA Configuration Register
#define REG_DMA330_CR(n)          *((const vu32*)(DMA330_REGS_BASE + 0xE00 + ((n) * 4)))

#define REG_DMA330_WD             *((      vu32*)(DMA330_REGS_BASE + 0xE80)) // Watchdog Register


//#define CHANNELS   (8u)
//#define PERIPHALS  (18u)
//#define IRQ_LINES  (16u)



static inline void waitForChannelStatus(u8 ch, u8 status)
{
	while((REG_DMA330_CSR(ch) & CSR_STATUS_MASK) != status);
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

	const u32 cr0 = REG_DMA330_CR0;
	const u8 numChannels = (cr0>>4 & 7u) + 1;
	const u8 numPeriphals = (cr0 & 1u ? (cr0>>12 & 0x1Fu) + 1 : 0);
	//const u8 numIrqs = (cr0>>17 & 0x1Fu) + 1;

	// Kill manager thread
	sendDebugCmd(DBGINST0_BYTES01(0x01u) | DBGINST0_THREAD_MGR, 0);

	// Kill all channels
	for(u32 i = 0; i < numChannels; i++)
	{
		// DMAKILL channel
		sendDebugCmd(DBGINST0_BYTES01(0x01u) | DBGINST0_CH_NUM(i) | DBGINST0_THREAD_CH, 0);
	}
	waitForChannelStatus(numChannels - 1, CSR_STATUS_STOPPED);

	// TODO: Hardcode number of IRQs or configurable?
	REG_DMA330_INTEN = 0x1FF;       // First 9 are interrupts and remaining events
	REG_DMA330_INTCLR = 0xFFFFFFFF; // Clear all interrupts
	REG_DMA330_WD = 0;              // Watchdog aborts hanging channels

	if(numPeriphals)
	{
		u8 progBuf[3] = {0x35, 0, 0}; // DMAFLUSHP 0, DMAEND
		for(u32 i = 0; i < numPeriphals; i++)
		{
			waitForChannelStatus(0, CSR_STATUS_STOPPED);

			progBuf[1] = i<<3; // Periphal ID
			flushDCacheRange(progBuf, 3);
			// DMAGO channel 0 non-secure
			sendDebugCmd(DBGINST0_BYTES01(0x00A2u) | DBGINST0_THREAD_MGR, (u32)progBuf);
		}
	}

/*ee_printf("DSR%lX FTRD%lX", REG_DMA330_DSR, REG_DMA330_FTRD);
for(u32 i = 0; i < 8; i++)
{
	ee_printf(" CSR%lX FTR%lX", REG_DMA330_CSR(i), REG_DMA330_FTR(i));
}
ee_printf("\n");*/
}

u8 DMA330_run(u8 ch, const u8 *const prog)
{
	u8 status;
	if((status = (REG_DMA330_CSR(ch) & CSR_STATUS_MASK)) != CSR_STATUS_STOPPED)
		return status;

	// DMAGO non-secure
	sendDebugCmd(DBGINST0_BYTES01(ch<<8 | 0xA2u) | DBGINST0_THREAD_MGR, (u32)prog);

	return status;
}

u8 DMA330_status(u8 ch)
{
	return REG_DMA330_CSR(ch) & CSR_STATUS_MASK;
}

void DMA330_ackIrq(u8 eventIrq)
{
	REG_DMA330_INTCLR = 1u<<eventIrq;
}

void DMA330_kill(u8 ch)
{
	if((REG_DMA330_CSR(ch) & CSR_STATUS_MASK) != CSR_STATUS_STOPPED)
	{
		sendDebugCmd(DBGINST0_BYTES01(0x01u) | DBGINST0_CH_NUM(ch) | DBGINST0_THREAD_CH, 0);
		waitForChannelStatus(ch, CSR_STATUS_STOPPED);
	}
}
