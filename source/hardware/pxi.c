/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
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
#include "hardware/pxi.h"
#ifdef ARM9
	#include "arm9/hardware/interrupt.h"
	#include "arm9/debug.h"
#elif ARM11
	#include "arm11/hardware/interrupt.h"
	#include "arm11/debug.h"
#endif
#include "ipc_handler.h"
#include "fb_assert.h"
#include "hardware/cache.h"


static vu32 g_lastResp[2] = {0};



static void pxiIrqHandler(UNUSED u32 id);

static inline void pxiSendWord(u32 word)
{
	while(REG_PXI_CNT & PXI_CNT_SFIFO_FULL);
	REG_PXI_SFIFO = word;
}

static inline u32 pxiRecvWord(void)
{
	while(REG_PXI_CNT & PXI_CNT_RFIFO_EMPTY);
	return REG_PXI_RFIFO;
}

static inline bool pxiFifoError(void)
{
	return (REG_PXI_CNT & PXI_CNT_FIFO_ERROR) != 0;
}

static inline void pxiSyncRequest(void)
{
	REG_PXI_SYNC_IRQ |= PXI_SYNC_IRQ_IRQ;
}

void PXI_init(void)
{
	REG_PXI_SYNC = PXI_SYNC_IRQ_ENABLE;
	REG_PXI_CNT = PXI_CNT_ENABLE_SRFIFO | PXI_CNT_FIFO_ERROR | PXI_CNT_FLUSH_SFIFO;

#ifdef ARM9
	pxiSendWord(0x99);
	while(pxiRecvWord() != 0x11);

	IRQ_registerIsr(IRQ_PXI_SYNC, pxiIrqHandler);
#elif ARM11
	while(pxiRecvWord() != 0x99);
	pxiSendWord(0x11);

	IRQ_registerIsr(IRQ_PXI_SYNC, 13, 0, pxiIrqHandler);
#endif
}

static void pxiIrqHandler(UNUSED u32 id)
{
	const u32 cmdCode = pxiRecvWord();
	if(cmdCode & IPC_CMD_RESP_FLAG)
	{
		g_lastResp[0] = cmdCode;
		g_lastResp[1] = pxiRecvWord();
		return;
	}

	const u32 inBufs = IPC_CMD_IN_BUFS_MASK(cmdCode);
	const u32 outBufs = IPC_CMD_OUT_BUFS_MASK(cmdCode);
	const u32 params = IPC_CMD_PARAMS_MASK(cmdCode);
	const u32 words = (inBufs * 2) + (outBufs * 2) + params;
	if(words > IPC_MAX_PARAMS) panic();

	u32 buf[IPC_MAX_PARAMS];
	for(u32 i = 0; i < words; i++) buf[i] = pxiRecvWord();
	if(pxiFifoError()) panic();

	const u32 res = IPC_handleCmd(IPC_CMD_ID_MASK(cmdCode), inBufs, outBufs, buf);
	pxiSendWord(IPC_CMD_RESP_FLAG | cmdCode);
	pxiSendWord(res);
	pxiSyncRequest();
}

u32 PXI_sendCmd(u32 cmd, const u32 *buf, u32 words)
{
	fb_assert(words <= IPC_MAX_PARAMS);


	const u32 inBufs = IPC_CMD_IN_BUFS_MASK(cmd);
	const u32 outBufs = IPC_CMD_OUT_BUFS_MASK(cmd);
	for(u32 i = 0; i < inBufs; i++)
	{
		const IpcBuffer *const inBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(inBuf->ptr && inBuf->size) cleanDCacheRange(inBuf->ptr, inBuf->size);
	}
	// Edge case:
	// memset() 256 bytes string buffer, fRead() 256 bytes from 10 bytes file and fWrite() them to another
	// file. The buffer will be filled with garbage where it wasn't overwritten because of the invalidate.
	// TODO: Should we flush here instead?
	for(u32 i = inBufs; i < inBufs + outBufs; i++)
	{
		const IpcBuffer *const outBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(outBuf->ptr && outBuf->size) invalidateDCacheRange(outBuf->ptr, outBuf->size);
	}

	pxiSendWord(cmd);
	pxiSyncRequest();

	for(u32 i = 0; i < words; i++) pxiSendWord(buf[i]);
	if(pxiFifoError()) panic();

	while(g_lastResp[0] != (IPC_CMD_RESP_FLAG | cmd)) __wfi();
	g_lastResp[0] = 0;
	const u32 res = g_lastResp[1];

#ifdef ARM11
	// The CPU may do speculative prefetches of data after the first invalidation
	// so we need to do it again.
	for(u32 i = inBufs; i < inBufs + outBufs; i++)
	{
		const IpcBuffer *const outBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(outBuf->ptr && outBuf->size) invalidateDCacheRange(outBuf->ptr, outBuf->size);
	}
#endif

	return res;
}

void PXI_sendPanicCmd(u32 cmd)
{
	pxiSendWord(cmd);
	pxiSyncRequest();
	while(pxiRecvWord() != (IPC_CMD_RESP_FLAG | cmd));
	// We don't care about the result
}
