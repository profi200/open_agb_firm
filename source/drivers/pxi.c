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
#include "drivers/pxi.h"
#ifdef ARM9
	#include "arm9/drivers/interrupt.h"
	#include "arm9/debug.h"
#elif ARM11
	#include "arm11/drivers/interrupt.h"
	#include "arm11/debug.h"
#endif // #ifdef ARM9
#include "ipc_handler.h"
#include "fb_assert.h"
#include "drivers/cache.h"


static vu32 g_lastResp[2] = {0};



static void pxiIrqHandler(UNUSED u32 id);

static inline void sendWord(Pxi *const pxi, u32 word)
{
	while(pxi->cnt & PXI_CNT_SEND_FULL);
	pxi->send = word;
}

static inline u32 recvWord(const Pxi *const pxi)
{
	while(pxi->cnt & PXI_CNT_RECV_EMPTY);
	return pxi->recv;
}

static inline bool getFifoError(const Pxi *const pxi)
{
	return (pxi->cnt & PXI_CNT_FIFO_ERROR) != 0u;
}

static inline void sendSyncRequest(Pxi *const pxi)
{
	pxi->sync_irq = PXI_SYNC_IRQ_IRQ_EN | PXI_SYNC_IRQ_IRQ;
}

void PXI_init(void)
{
	Pxi *const pxi = getPxiRegs();

	pxi->sync = PXI_SYNC_IRQ_EN;
	pxi->cnt  = PXI_CNT_EN_FIFOS | PXI_CNT_FIFO_ERROR | PXI_CNT_FLUSH_SEND;

	// TODO: Make this handshake IRQ driven.
#ifdef ARM9
	sendWord(pxi, 0x99);
	while(recvWord(pxi) != 0x11);

	IRQ_registerIsr(IRQ_PXI_SYNC, pxiIrqHandler);
#elif ARM11
	while(recvWord(pxi) != 0x99);
	sendWord(pxi, 0x11);

	IRQ_registerIsr(IRQ_PXI_SYNC, 13, 0, pxiIrqHandler);
#endif
}

static void pxiIrqHandler(UNUSED u32 id)
{
	Pxi *const pxi = getPxiRegs();

	const u32 cmdCode = recvWord(pxi);
	if(cmdCode & IPC_CMD_RESP_FLAG)
	{
		g_lastResp[0] = cmdCode;
		g_lastResp[1] = recvWord(pxi);
		return;
	}

	const u32 sendBufs = IPC_CMD_SEND_BUFS_MASK(cmdCode);
	const u32 recvBufs = IPC_CMD_RECV_BUFS_MASK(cmdCode);
	const u32 params   = IPC_CMD_PARAMS_MASK(cmdCode);
	const u32 words    = (sendBufs * 2) + (recvBufs * 2) + params;
	if(words > IPC_MAX_PARAMS) panic();

	u32 buf[IPC_MAX_PARAMS];
	for(u32 i = 0; i < words; i++) buf[i] = recvWord(pxi);
	if(getFifoError(pxi)) panic();

	const u32 res = IPC_handleCmd(IPC_CMD_ID_MASK(cmdCode), sendBufs, recvBufs, buf);
	sendWord(pxi, IPC_CMD_RESP_FLAG | cmdCode);
	sendWord(pxi, res);
	sendSyncRequest(pxi);
}

u32 PXI_sendCmd(u32 cmd, const u32 *buf, u32 words)
{
	fb_assert(words <= IPC_MAX_PARAMS);

	const u32 sendBufs = IPC_CMD_SEND_BUFS_MASK(cmd);
	const u32 recvBufs = IPC_CMD_RECV_BUFS_MASK(cmd);
	for(u32 i = 0; i < sendBufs; i++)
	{
		const IpcBuffer *const sendBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(sendBuf->ptr && sendBuf->size) cleanDCacheRange(sendBuf->ptr, sendBuf->size);
	}
	// Edge case:
	// memset() 256 bytes string buffer, fRead() 256 bytes from 10 bytes file and fWrite() them to another
	// file. The buffer will be filled with garbage where it wasn't overwritten because of the invalidate.
	// TODO: Should we flush here instead?
	for(u32 i = sendBufs; i < sendBufs + recvBufs; i++)
	{
		const IpcBuffer *const recvBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(recvBuf->ptr && recvBuf->size) flushDCacheRange(recvBuf->ptr, recvBuf->size);
	}

	Pxi *const pxi = getPxiRegs();
	sendWord(pxi, cmd);
	sendSyncRequest(pxi);

	for(u32 i = 0; i < words; i++) sendWord(pxi, buf[i]);
	if(getFifoError(pxi)) panic();

	while(g_lastResp[0] != (IPC_CMD_RESP_FLAG | cmd)) __wfi();
	g_lastResp[0] = 0;
	const u32 res = g_lastResp[1];

#ifdef ARM11
	// The CPU may do speculative prefetches of data after the first invalidation
	// so we need to do it again.
	for(u32 i = sendBufs; i < sendBufs + recvBufs; i++)
	{
		const IpcBuffer *const recvBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(recvBuf->ptr && recvBuf->size) invalidateDCacheRange(recvBuf->ptr, recvBuf->size);
	}
#endif

	return res;
}

void PXI_sendPanicCmd(u32 cmd)
{
	Pxi *const pxi = getPxiRegs();

	sendWord(pxi, cmd);
	sendSyncRequest(pxi);
	while(recvWord(pxi) != (IPC_CMD_RESP_FLAG | cmd));
	// We don't care about the result.
}
