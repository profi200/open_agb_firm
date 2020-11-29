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
#include "ipc_handler.h"
#include "hardware/cache.h"
#include "fs.h"
#include "hardware/lgy.h"
#include "arm9/debug.h"



u32 IPC_handleCmd(u8 cmdId, u32 inBufs, u32 outBufs, const u32 *const buf)
{
	for(u32 i = 0; i < inBufs; i++)
	{
		const IpcBuffer *const inBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(inBuf->ptr && inBuf->size) invalidateDCacheRange(inBuf->ptr, inBuf->size);
	}

	u32 result = 0;
	switch(cmdId)
	{
		// Filesystem API.
		case IPC_CMD_ID_MASK(IPC_CMD9_FMOUNT):
			result = fMount(buf[0]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FUNMOUNT):
			result = fUnmount(buf[0]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FGETFREE):
			result = fGetFree(buf[2], (u64*)buf[0]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FOPEN):
			result = fOpen((FHandle*)buf[2], (const char *const)buf[0], buf[4]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FREAD):
			result = fRead(buf[4], (void *const)buf[0], buf[1], (u32 *const)buf[2]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FWRITE):
			result = fWrite(buf[4], (const void *const)buf[0], buf[1], (u32 *const)buf[2]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FSYNC):
			result = fSync(buf[0]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FLSEEK):
			result = fLseek(buf[0], buf[1]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FTELL):
			result = fTell(buf[0]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FSIZE):
			result = fSize(buf[0]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FCLOSE):
			result = fClose(buf[0]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FSTAT):
			result = fStat((const char *const)buf[0], (FILINFO *const)buf[2]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FCHDIR):
			result = fChdir((const char *const)buf[0]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FOPEN_DIR):
			result = fOpenDir((DHandle *const)buf[2], (const char *const)buf[0]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FREAD_DIR):
			result = fReadDir(buf[4], (FILINFO *const)buf[0], buf[5], (u32 *const)buf[2]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FCLOSE_DIR):
			result = fCloseDir(buf[0]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FMKDIR):
			result = fMkdir((const char *const)buf[0]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FRENAME):
			result = fRename((const char *const)buf[0], (const char *const)buf[2]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_FUNLINK):
			result = fUnlink((const char *const)buf[0]);
			break;

		// open_agb_firm specific API.
		case IPC_CMD_ID_MASK(IPC_CMD9_PREPARE_GBA):
			result = LGY_prepareGbaMode(buf[2], buf[3], (const char *const)buf[0]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_SET_GBA_RTC):
			result = LGY_setGbaRtc(*((GbaRtc*)buf));
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_GET_GBA_RTC):
			result = LGY_getGbaRtc((GbaRtc*)buf[0]);
			break;
		case IPC_CMD_ID_MASK(IPC_CMD9_BACKUP_GBA_SAVE):
			result = LGY_backupGbaSave();
			break;

		// Miscellaneous API.
		case IPC_CMD_ID_MASK(IPC_CMD9_PREPARE_POWER):
			fsDeinit();
			break;
		default:
			panic();
	}

	for(u32 i = inBufs; i < inBufs + outBufs; i++)
	{
		const IpcBuffer *const outBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(outBuf->ptr && outBuf->size) flushDCacheRange(outBuf->ptr, outBuf->size);
	}

	return result;
}
