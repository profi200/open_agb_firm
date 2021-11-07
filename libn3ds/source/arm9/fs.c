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
#include "error_codes.h"
#include "fs.h"
#include "fatfs/ff.h"


static const char *const g_fsPathTable[FS_MAX_DRIVES] = {FS_DRIVE_NAMES};
static struct
{
	FATFS fsTable[FS_MAX_DRIVES];

	FIL fTable[FS_MAX_FILES];
	u32 fBitmap;
	u32 fHandles;

	DIR dTable[FS_MAX_DIRS];
	u32 dBitmap;
	u32 dHandles;
} g_fsState = {0};



static Result fres2Res(FRESULT fr)
{
	if(fr != FR_OK) return fr + RES_FR_DISK_ERR - 1;
	else            return RES_OK;
}

static u32 findUnusedFileSlot(void)
{
	if(g_fsState.fHandles >= FS_MAX_FILES) return (u32)-1;

	u32 i = 0;
	do
	{
		if((g_fsState.fBitmap & 1u<<i) == 0) break;
	} while(++i < FS_MAX_FILES);

	return i;
}

static bool isFileHandleValid(FHandle h)
{
	if(h > g_fsState.fHandles) return false;
	else                       return true;
}

static u32 findUnusedDirSlot(void)
{
	if(g_fsState.dHandles >= FS_MAX_DIRS) return (u32)-1;

	u32 i = 0;
	do
	{
		if((g_fsState.dBitmap & 1u<<i) == 0) break;
	} while(++i < FS_MAX_DIRS);

	return i;
}

static bool isDirHandleValid(DHandle h)
{
	if(h > g_fsState.dHandles) return false;
	else                       return true;
}

Result fMount(FsDrive drive)
{
	if(drive >= FS_MAX_DRIVES) return RES_FR_INVALID_DRIVE;

	return fres2Res(f_mount(&g_fsState.fsTable[drive], g_fsPathTable[drive], 1));
}

Result fUnmount(FsDrive drive)
{
	if(drive >= FS_MAX_DRIVES) return RES_FR_INVALID_DRIVE;

	return fres2Res(f_mount(NULL, g_fsPathTable[drive], 0));
}

Result fGetFree(FsDrive drive, u64 *const size)
{
	if(drive >= FS_MAX_DRIVES) return RES_FR_INVALID_DRIVE;

	DWORD freeClusters;
	FATFS *fs;
	Result res = fres2Res(f_getfree(g_fsPathTable[drive], &freeClusters, &fs));
	if(res == RES_OK)
	{
		if(size) *size = (u64)(freeClusters * fs->csize) * 512u;
	}

	return res;
}

Result fOpen(FHandle *const hOut, const char *const path, u8 mode)
{
	if(hOut == NULL) return RES_INVALID_ARG;
	const u32 slot = findUnusedFileSlot();
	if(slot == (u32)-1) return RES_FR_TOO_MANY_OPEN_FILES;

	Result res = fres2Res(f_open(&g_fsState.fTable[slot], path, mode));
	if(res == RES_OK)
	{
		g_fsState.fBitmap |= 1u<<slot;
		g_fsState.fHandles++;
		*hOut = (FHandle)slot;
	}

	return res;
}

Result fRead(FHandle h, void *const buf, u32 size, u32 *const bytesRead)
{
	if(!isFileHandleValid(h)) return RES_FR_INVALID_OBJECT;

	UINT tmpBytesRead;
	Result res = fres2Res(f_read(&g_fsState.fTable[h], buf, size, &tmpBytesRead));

	if(bytesRead != NULL) *bytesRead = tmpBytesRead;

	return res;
}

Result fWrite(FHandle h, const void *const buf, u32 size, u32 *const bytesWritten)
{
	if(!isFileHandleValid(h)) return RES_FR_INVALID_OBJECT;

	UINT tmpBytesWritten;
	Result res = fres2Res(f_write(&g_fsState.fTable[h], buf, size, &tmpBytesWritten));

	if(bytesWritten != NULL) *bytesWritten = tmpBytesWritten;

	return res;
}

Result fSync(FHandle h)
{
	if(!isFileHandleValid(h)) return RES_FR_INVALID_OBJECT;

	return fres2Res(f_sync(&g_fsState.fTable[h]));
}

Result fLseek(FHandle h, u32 off)
{
	if(!isFileHandleValid(h)) return RES_FR_INVALID_OBJECT;

	return fres2Res(f_lseek(&g_fsState.fTable[h], off));
}

u32 fTell(FHandle h)
{
	if(!isFileHandleValid(h)) return 0;

	return f_tell(&g_fsState.fTable[h]);
}

u32 fSize(FHandle h)
{
	if(!isFileHandleValid(h)) return 0;

	return f_size(&g_fsState.fTable[h]);
}

Result fClose(FHandle h)
{
	if(g_fsState.fHandles == 0 || !isFileHandleValid(h))
		return RES_FR_INVALID_OBJECT;

	Result res = fres2Res(f_close(&g_fsState.fTable[h]));
	g_fsState.fBitmap &= ~(1u<<h);
	g_fsState.fHandles--;

	return res;
}

Result fStat(const char *const path, FILINFO *const fi)
{
	return fres2Res(f_stat(path, fi));
}

Result fChdir(const char *const path)
{
	return fres2Res(f_chdir(path));
}

Result fOpenDir(DHandle *const hOut, const char *const path)
{
	if(hOut == NULL) return RES_INVALID_ARG;
	const u32 slot = findUnusedDirSlot();
	if(slot == (u32)-1) return RES_FR_TOO_MANY_OPEN_FILES;

	Result res = fres2Res(f_opendir(&g_fsState.dTable[slot], path));
	if(res == RES_OK)
	{
		g_fsState.dBitmap |= 1u<<slot;
		g_fsState.dHandles++;
		*hOut = (DHandle)slot;
	}

	return res;
}

Result fReadDir(DHandle h, FILINFO *const fi, u32 num, u32 *const entriesRead)
{
	if(!isDirHandleValid(h)) return RES_FR_INVALID_OBJECT;
	// TODO: Check for insanely high num?

	u32 i;
	DIR *const dir = &g_fsState.dTable[h];
	Result res;
	for(i = 0; i < num; i++)
	{
		res = fres2Res(f_readdir(dir, &fi[i]));

		if(res != RES_OK || fi[i].fname[0] == 0) break;
	}

	if(entriesRead != NULL) *entriesRead = i;

	return res;
}

Result fCloseDir(DHandle h)
{
	if(g_fsState.dHandles == 0 || !isDirHandleValid(h))
		return RES_FR_INVALID_OBJECT;

	Result res = fres2Res(f_closedir(&g_fsState.dTable[h]));
	g_fsState.dBitmap &= ~(1u<<h);
	g_fsState.dHandles--;

	return res;
}

Result fMkdir(const char *const path)
{
	return fres2Res(f_mkdir(path));
}

Result fRename(const char *const old, const char *const _new)
{
	return fres2Res(f_rename(old, _new));
}

Result fUnlink(const char *const path)
{
	return fres2Res(f_unlink(path));
}

void fsDeinit(void)
{
	for(u32 i = 0; i < FS_MAX_FILES; i++)  fClose(i);
	for(u32 i = 0; i < FS_MAX_DRIVES; i++) fUnmount(i);

	// TODO: Deinit drives.
}
