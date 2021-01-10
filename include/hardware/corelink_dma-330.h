#pragma once

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


// REG_DMA330_DSR
#define DSR_WAKE_EVNT_SHIFT       (4u)
#define DSR_WAKE_EVNT_MASK        (0x1Fu<<DSR_WAKEUP_EVNT_SHIFT)
#define DSR_DNS                   (1u<<9) // DMA Manager is non-secure.

enum
{
	DSR_STAT_STOPPED     =  0u,
	DSR_STAT_EXECUTING   =  1u,
	DSR_STAT_CACHE_MISS  =  2u,
	DSR_STAT_UPDATING_PC =  3u, // Updating program counter.
	DSR_STAT_WFE         =  4u, // Waiting for event.
	DSR_STAT_FAULTING    = 15u,

	DSR_STAT_MASK        = DSR_STAT_FAULTING
};

// REG_DMA330_INTEN
#define INTEN_SEL_IRQ(n)          (1u<<(n)) // Select IRQ instead of event.

// REG_DMA330_INT_EVENT_RIS
#define INT_EVENT_RIS_ACTIVE(n)   (1u<<(n)) // Interrupt or event N is active.

// REG_DMA330_INTMIS
#define INTMIS_IRQ_ACTIVE(n)      (1u<<(n)) // Interrupt N is active.

// REG_DMA330_INTCLR
#define INTCLR_IRQ_CLR(n)         (1u<<(n)) // Clear interrupt N.

// REG_DMA330_FSRD
#define FSRD_FAULTING             (1u) // DMA manager is in faulting state.

// REG_DMA330_FSRC
#define FSRC_FAULTING(n)          (1u<<(n)) // DMA channel is in faulting or faulting completing state.

// REG_DMA330_FTRD
#define FTRD_UNDEF_INSTR          (1u)
#define FTRD_OPERAND_INVALID      (1u<<1)
#define FTRD_DMAGO_ERR            (1u<<4)  // Starting a secure channel from a non-secure state.
#define FTRD_MGR_EVNT_ERR         (1u<<5)  // Waiting for or creating secure events/interrupts in no-secure state.
#define FTRD_INSTR_FETCH_ERR      (1u<<16)
#define FTRD_DBG_INSTR            (1u<<30) // The erroneous instruction came from the debug interface.

// REG_DMA330_FTR0-7
#define FTR_UNDEF_INSTR           (1u)
#define FTR_OPERAND_INVALID       (1u<<1)
#define FTR_CH_EVNT_ERR           (1u<<5)  // Waiting for or creating secure events/interrupts in no-secure state.
#define FTR_CH_PERIPH_ERR         (1u<<6)  // Accessing secure periphals in non-secure state (DMAWFP, DMALDP, DMASTP, DMAFLUSHP).
#define FTR_CH_RDWR_ERR           (1u<<7)  // Secure read or write in non-secure state.
#define FTR_CH_MFIFO_ERR          (1u<<12) // MFIFO too small to hold or store the data (DMALD, DMAST).
#define FTR_CH_ST_DATA_UNAVAIL    (1u<<13) // Not enough data in the MFIFO for DMAST to complete.
#define FTR_INSTR_FETCH_ERR       (1u<<16)
#define FTR_DATA_WRITE_ERR        (1u<<17)
#define FTR_DATA_READ_ERR         (1u<<18)
#define FTR_DBG_INSTR             (1u<<30) // The erroneous instruction came from the debug interface.
#define FTR_LOCKUP_ERR            (1u<<31) // Channel locked up because of resource starvation.

// REG_DMA330_CSR0-7
#define CSR_WAKE_EVNT_SHIFT       (4u)
#define CSR_WAKE_EVNT_MASK        (0x1Fu<<CSR_WAKEUP_EVNT_SHIFT)
#define CSR_DMAWFP_B_NS           (1u<<14) // DMAWFP executed with burst operand set.
#define CSR_DMAWFP_PERIPH         (1u<<15) // DMAWFP executed with periph operand set.
#define CSR_CNS                   (1u<<21) // DMA channel is non-secure.

enum
{
	CSR_STAT_STOPPED             =  0u,
	CSR_STAT_EXECUTING           =  1u,
	CSR_STAT_CACHE_MISS          =  2u,
	CSR_STAT_UPDATING_PC         =  3u, // Updating program counter.
	CSR_STAT_WFE                 =  4u, // Waiting for event.
	CSR_STAT_AT_BARRIER          =  5u,
	CSR_STAT_WFP                 =  7u, // Waiting for periphal.
	CSR_STAT_KILLING             =  8u,
	CSR_STAT_COMPLETING          =  9u,
	CSR_STAT_FAULTING_COMPLETING = 14u,
	CSR_STAT_FAULTING            = 15u,

	CSR_STAT_MASK                = CSR_STAT_FAULTING
};

// REG_DMA330_CCR0-7
#define CCR_SRC_INC               (1u)
#define CCR_SRC_BURST_SIZE_SHIFT  (1u)
#define CCR_SRC_BURST_SIZE_MASK   (0x7u<<CCR_SRC_BURST_SIZE_SHIFT)
#define CCR_SRC_BURST_LEN_SHIFT   (4u)
#define CCR_SRC_BURST_LEN_MASK    (0xFu<<CCR_SRC_BURST_LEN_SHIFT)
#define CCR_SRC_PROT_CTRL_SHIFT   (8u)
#define CCR_SRC_PROT_CTRL_MASK    (0x7u<<CCR_SRC_PROT_CTRL_SHIFT)
#define CCR_SRC_CACHE_CTRL_SHIFT  (11u)
#define CCR_SRC_CACHE_CTRL_MASK   (0x7u<<CCR_SRC_CACHE_CTRL_SHIFT)

#define CCR_DST_INC               (1u<<14)
#define CCR_DST_BURST_SIZE_SHIFT  (15u)
#define CCR_DST_BURST_SIZE_MASK   (0x7u<<CCR_DST_BURST_SIZE_SHIFT)
#define CCR_DST_BURST_LEN_SHIFT   (18u)
#define CCR_DST_BURST_LEN_MASK    (0xFu<<CCR_DST_BURST_LEN_SHIFT)
#define CCR_DST_PROT_CTRL_SHIFT   (22u)
#define CCR_DST_PROT_CTRL_MASK    (0x7u<<CCR_DST_PROT_CTRL_SHIFT)
#define CCR_DST_CACHE_CTRL_SHIFT  (25u)
#define CCR_DST_CACHE_CTRL_MASK   (0x7u<<CCR_DST_CACHE_CTRL_SHIFT)

#define CCR_END_SWP_SIZE_SHIFT    (28u) // END_SWP_SIZE = endian swap size.
#define CCR_END_SWP_SIZE_MASK     (0x7u<<CCR_END_SWP_SIZE_SHIFT)

// REG_DMA330_DBGSTATUS
#define DBGSTATUS_BUSY            (1u)

// REG_DMA330_DBGCMD
#define DBGCMD_EXECUTE            (0u)

// REG_DMA330_DBGINST0
#define DBGINST0_THR_MGR          (0u) // Select DMA manager thread.
#define DBGINST0_THR_CH           (1u) // Select DMA channel thread (also needs a channel number).
#define DBGINST0(b10, ch, t)      ((b10)<<16 | (ch)<<8 | (t)) // b10 = byte 1 and 0, ch = channel num, t = thread.
// DBGINST1 stores the remaining 4 instruction bytes.



void DMA330_init(void);
u8 DMA330_run(u8 ch, const u8 *const prog);
u8 DMA330_status(u8 ch);
void DMA330_ackIrq(u8 eventIrq);
void DMA330_sev(u8 event);
void DMA330_kill(u8 ch);

#ifdef ARM11
//void DMA330_dbgPrint(void);
#endif // ifdef ARM11
