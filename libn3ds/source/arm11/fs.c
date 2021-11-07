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

#include <string.h>
#include "types.h"
#include "error_codes.h"
#include "fs.h"
#include "ipc_handler.h"
#include "drivers/pxi.h"



Result fMount(FsDrive drive)
{
	const u32 cmdBuf = drive;
	return PXI_sendCmd(IPC_CMD9_FMOUNT, &cmdBuf, 1);
}

Result fUnmount(FsDrive drive)
{
	const u32 cmdBuf = drive;
	return PXI_sendCmd(IPC_CMD9_FUNMOUNT, &cmdBuf, 1);
}

Result fGetFree(FsDrive drive, u64 *const size)
{
	u32 cmdBuf[3];
	cmdBuf[0] = (u32)size;
	cmdBuf[1] = sizeof(u64);
	cmdBuf[2] = drive;

	return PXI_sendCmd(IPC_CMD9_FGETFREE, cmdBuf, 3);
}

Result fOpen(FHandle *const hOut, const char *const path, u8 mode)
{
	u32 cmdBuf[5];
	cmdBuf[0] = (u32)path;
	cmdBuf[1] = strlen(path) + 1;
	cmdBuf[2] = (u32)hOut;
	cmdBuf[3] = sizeof(FHandle);
	cmdBuf[4] = mode;

	return PXI_sendCmd(IPC_CMD9_FOPEN, cmdBuf, 5);
}

Result fRead(FHandle h, void *const buf, u32 size, u32 *const bytesRead)
{
	u32 cmdBuf[5];
	cmdBuf[0] = (u32)buf;
	cmdBuf[1] = size;
	cmdBuf[2] = (u32)bytesRead;
	cmdBuf[3] = sizeof(u32);
	cmdBuf[4] = h;

	return PXI_sendCmd(IPC_CMD9_FREAD, cmdBuf, 5);
}

Result fWrite(FHandle h, const void *const buf, u32 size, u32 *const bytesWritten)
{
	u32 cmdBuf[5];
	cmdBuf[0] = (u32)buf;
	cmdBuf[1] = size;
	cmdBuf[2] = (u32)bytesWritten;
	cmdBuf[3] = sizeof(u32);
	cmdBuf[4] = h;

	return PXI_sendCmd(IPC_CMD9_FWRITE, cmdBuf, 5);
}

Result fSync(FHandle h)
{
	const u32 cmdBuf = h;
	return PXI_sendCmd(IPC_CMD9_FSYNC, &cmdBuf, 1);
}

Result fLseek(FHandle h, u32 off)
{
	u32 cmdBuf[2];
	cmdBuf[0] = h;
	cmdBuf[1] = off;

	return PXI_sendCmd(IPC_CMD9_FLSEEK, cmdBuf, 2);
}

u32 fTell(FHandle h)
{
	const u32 cmdBuf = h;
	return PXI_sendCmd(IPC_CMD9_FTELL, &cmdBuf, 1);
}

u32 fSize(FHandle h)
{
	const u32 cmdBuf = h;
	return PXI_sendCmd(IPC_CMD9_FSIZE, &cmdBuf, 1);
}

Result fClose(FHandle h)
{
	const u32 cmdBuf = h;
	return PXI_sendCmd(IPC_CMD9_FCLOSE, &cmdBuf, 1);
}

Result fStat(const char *const path, FILINFO *const fi)
{
	u32 cmdBuf[4];
	cmdBuf[0] = (u32)path;
	cmdBuf[1] = strlen(path) + 1;
	cmdBuf[2] = (u32)fi;
	cmdBuf[3] = sizeof(FILINFO);

	return PXI_sendCmd(IPC_CMD9_FSTAT, cmdBuf, 4);
}

Result fChdir(const char *const path)
{
	u32 cmdBuf[2];
	cmdBuf[0] = (u32)path;
	cmdBuf[1] = strlen(path) + 1;

	return PXI_sendCmd(IPC_CMD9_FCHDIR, cmdBuf, 2);
}

Result fOpenDir(DHandle *const hOut, const char *const path)
{
	u32 cmdBuf[4];
	cmdBuf[0] = (u32)path;
	cmdBuf[1] = strlen(path) + 1;
	cmdBuf[2] = (u32)hOut;
	cmdBuf[3] = sizeof(DHandle);

	return PXI_sendCmd(IPC_CMD9_FOPEN_DIR, cmdBuf, 4);
}

Result fReadDir(DHandle h, FILINFO *const fi, u32 num, u32 *const entriesRead)
{
	u32 cmdBuf[6];
	cmdBuf[0] = (u32)fi;
	cmdBuf[1] = sizeof(FILINFO) * num;
	cmdBuf[2] = (u32)entriesRead;
	cmdBuf[3] = sizeof(u32);
	cmdBuf[4] = h;
	cmdBuf[5] = num;

	return PXI_sendCmd(IPC_CMD9_FREAD_DIR, cmdBuf, 6);
}

Result fCloseDir(DHandle h)
{
	const u32 cmdBuf = h;
	return PXI_sendCmd(IPC_CMD9_FCLOSE_DIR, &cmdBuf, 1);
}

Result fMkdir(const char *const path)
{
	u32 cmdBuf[2];
	cmdBuf[0] = (u32)path;
	cmdBuf[1] = strlen(path) + 1;

	return PXI_sendCmd(IPC_CMD9_FMKDIR, cmdBuf, 2);
}

Result fRename(const char *const old, const char *const _new)
{
	u32 cmdBuf[4];
	cmdBuf[0] = (u32)old;
	cmdBuf[1] = strlen(old) + 1;
	cmdBuf[2] = (u32)_new;
	cmdBuf[3] = strlen(_new) + 1;

	return PXI_sendCmd(IPC_CMD9_FRENAME, cmdBuf, 4);
}

Result fUnlink(const char *const path)
{
	u32 cmdBuf[2];
	cmdBuf[0] = (u32)path;
	cmdBuf[1] = strlen(path) + 1;

	return PXI_sendCmd(IPC_CMD9_FUNLINK, cmdBuf, 2);
}
