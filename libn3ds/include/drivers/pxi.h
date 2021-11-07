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

#include <assert.h>
#include "types.h"
#include "mem_map.h"


#ifdef ARM9
#define PXI_REGS_BASE                   (IO_MEM_ARM9_ONLY + 0x8000)
#elif ARM11
#define PXI_REGS_BASE                   (IO_MEM_ARM9_ARM11 + 0x63000)
#endif // #ifdef ARM9

typedef struct
{
	union
	{
		struct
		{
			vu8 sync_recvd; // 0x0 Received. Set by remote CPU via SENT.
			vu8 sync_sent;  // 0x1 Write-only, sent.
			u8 _0x2;
			vu8 sync_irq;   // 0x3
		};
		vu32 sync;          // 0x0
	};
	vu32 cnt;               // 0x4
	vu32 send;              // 0x8 Send FIFO.
	const vu32 recv;        // 0xC Receive FIFO.
} Pxi;
static_assert(offsetof(Pxi, recv) == 0xC, "Error: Member recv of Pxi is not at offset 0xC!");

ALWAYS_INLINE Pxi* getPxiRegs(void)
{
	return (Pxi*)PXI_REGS_BASE;
}


// REG_PXI_SYNC
// Note: SENT and RECV in REG_PXI_SYNC do not count
//       the amount of bytes sent or received through the FIFOs!
//       They are simply CPU read-/writable fields.
#define PXI_SYNC_RECVD                  (REG_PXI_SYNC & 0xFFu)
#define PXI_SYNC_SENT(sent)             ((REG_PXI_SYNC & ~(0xFFu<<8)) | (sent)<<8)
#ifdef ARM9
#define PXI_SYNC_IRQ                    (1u<<29) // Trigger interrupt on ARM11.
#define PXI_SYNC_IRQ2                   (1u<<30) // Another, separate interrupt trigger for ARM11.
#elif ARM11
// 29 unused unlike ARM9 side.
#define PXI_SYNC_IRQ                    (1u<<30) // Trigger interrupt on ARM9.
#endif // #ifdef ARM9
#define PXI_SYNC_IRQ_EN                 (1u<<31) // Enable interrupt(s) from remote CPU.

// REG_PXI_SYNC_IRQ (byte 3 of REG_PXI_SYNC)
#ifdef ARM9
#define PXI_SYNC_IRQ_IRQ                (1u<<5) // Trigger interrupt on ARM11.
#define PXI_SYNC_IRQ_IRQ2               (1u<<6) // Another, separate interrupt trigger for ARM11.
#elif ARM11
// 29 unused unlike ARM9 side.
#define PXI_SYNC_IRQ_IRQ                (1u<<6) // Trigger interrupt on ARM9.
#endif // #ifdef ARM9
#define PXI_SYNC_IRQ_IRQ_EN             (1u<<7) // Enable interrupt(s) from remote CPU.

// REG_PXI_CNT
// Status bits: 0 = no, 1 = yes.
#define PXI_CNT_SEND_EMPTY              (1u<<0)  // Send FIFO empty status.
#define PXI_CNT_SEND_FULL               (1u<<1)  // Send FIFO full status.
#define PXI_CNT_SEND_NOT_FULL_IRQ_EN    (1u<<2)  // Send FIFO not full interrupt enable. TODO: Test the behavior.
#define PXI_CNT_FLUSH_SEND              (1u<<3)  // Flush send FIFO.
#define PXI_CNT_RECV_EMPTY              (1u<<8)  // Receive FIFO empty status.
#define PXI_CNT_RECV_FULL               (1u<<9)  // Receive FIFO full status.
#define PXI_CNT_RECV_NOT_EMPTY_IRQ_EN   (1u<<10) // Receive FIFO not empty interrupt enable. TODO: Test the behavior.
#define PXI_CNT_FIFO_ERROR              (1u<<14) // Receive FIFO underrun or send FIFO overrun error flag. Acknowledge by writing 1.
#define PXI_CNT_EN_FIFOS                (1u<<15) // Enables REG_PXI_SEND and REG_PXI_RECV FIFOs.



void PXI_init(void);
u32 PXI_sendCmd(u32 cmd, const u32 *buf, u32 words);
void PXI_sendPanicCmd(u32 cmd); // Not intended for normal use!
