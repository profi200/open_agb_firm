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

#include <stdlib.h>
#include "types.h"
#include "ipc_handler.h"
#include "drivers/cache.h"
#include "arm11/debug.h"



u32 IPC_handleCmd(u8 cmdId, u32 sendBufs, u32 recvBufs, UNUSED const u32 *const buf)
{
	for(u32 i = 0; i < sendBufs; i++)
	{
		const IpcBuffer *const sendBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(sendBuf->ptr && sendBuf->size) invalidateDCacheRange(sendBuf->ptr, sendBuf->size);
	}

	u32 result = 0;
	switch(cmdId)
	{
		case IPC_CMD_ID_MASK(IPC_CMD11_PRINT_MSG):
		case IPC_CMD_ID_MASK(IPC_CMD11_PANIC):
		case IPC_CMD_ID_MASK(IPC_CMD11_EXCEPTION):
			break;
		default:
			panic();
	}

	for(u32 i = sendBufs; i < sendBufs + recvBufs; i++)
	{
		const IpcBuffer *const recvBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(recvBuf->ptr && recvBuf->size) flushDCacheRange(recvBuf->ptr, recvBuf->size);
	}

	return result;
}
