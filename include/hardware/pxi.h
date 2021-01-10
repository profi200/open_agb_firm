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
#include "mem_map.h"


#ifdef ARM9
#define PXI_REGS_BASE                       (IO_MEM_ARM9_ONLY + 0x8000)
#elif ARM11
#define PXI_REGS_BASE                       (IO_MEM_ARM9_ARM11 + 0x63000)
#endif
#define REG_PXI_SYNC_RECVD                  *((const vu8*)(PXI_REGS_BASE + 0x00))
#define REG_PXI_SYNC_SENT                   *((vu8 *)(PXI_REGS_BASE + 0x01)) // Write-only
#define REG_PXI_SYNC_IRQ                    *((vu8 *)(PXI_REGS_BASE + 0x03))
#define REG_PXI_SYNC                        *((vu32*)(PXI_REGS_BASE + 0x00))
#define REG_PXI_CNT                         *((vu16*)(PXI_REGS_BASE + 0x04))
#define REG_PXI_SFIFO                       *((vu32*)(PXI_REGS_BASE + 0x08))
#define REG_PXI_RFIFO                       *((const vu32*)(PXI_REGS_BASE + 0x0C))


// REG_PXI_SYNC
#define PXI_SYNC_RECVD                      (REG_PXI_SYNC & 0xFFu)
#define PXI_SYNC_SENT(sent)                 ((REG_PXI_SYNC & ~(0xFFu<<8)) | (sent)<<8)
#ifdef ARM9
#define PXI_SYNC_IRQ                        (1u<<29)
#elif ARM11
#define PXI_SYNC_IRQ                        (1u<<30)
#endif
#define PXI_SYNC_IRQ_ENABLE                 (1u<<31)

// REG_PXI_SYNC_IRQ
#ifdef ARM9
#define PXI_SYNC_IRQ_IRQ                    (1u<<5)
#define PXI_SYNC_IRQ_IRQ2                   (1u<<6)
#elif ARM11
#define PXI_SYNC_IRQ_IRQ                    (1u<<6)
#endif
#define PXI_SYNC_IRQ_IRQ_ENABLE             (1u<<7)

// REG_PXI_CNT
#define PXI_CNT_SFIFO_EMPTY                 (1u<<0)
#define PXI_CNT_SFIFO_FULL                  (1u<<1)
#define PXI_CNT_SFIFO_NOT_FULL_IRQ_ENABLE   (1u<<2)
#define PXI_CNT_FLUSH_SFIFO                 (1u<<3)
#define PXI_CNT_RFIFO_EMPTY                 (1u<<8)
#define PXI_CNT_RFIFO_FULL                  (1u<<9)
#define PXI_CNT_RFIFO_NOT_EMPTY_IRQ_ENABLE  (1u<<10)
#define PXI_CNT_FIFO_ERROR                  (1u<<14) // Also used for aknowledge
#define PXI_CNT_ENABLE_SRFIFO               (1u<<15)



void PXI_init(void);
u32 PXI_sendCmd(u32 cmd, const u32 *buf, u32 words);
void PXI_sendPanicCmd(u32 cmd); // Not intended for normal use!
