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
#include "error_codes.h"
#include "fatfs/ff.h"


#define FS_MAX_DRIVES   (FF_VOLUMES)
#define FS_DRIVE_NAMES  "sdmc:/"
#define FS_MAX_FILES    (1u)
#define FS_MAX_DIRS     (1u)


typedef enum
{
	FS_DRIVE_SDMC = 0u
} FsDrive;

typedef u32 FHandle;
typedef u32 DHandle;



Result fMount(FsDrive drive);
Result fUnmount(FsDrive drive);
Result fGetFree(FsDrive drive, u64 *const size);
Result fOpen(FHandle *const hOut, const char *const path, u8 mode);
Result fRead(FHandle h, void *const buf, u32 size, u32 *const bytesRead);
Result fWrite(FHandle h, const void *const buf, u32 size, u32 *const bytesWritten);
Result fSync(FHandle h);
Result fLseek(FHandle h, u32 off);
u32    fTell(FHandle h);
u32    fSize(FHandle h);
Result fClose(FHandle h);
Result fStat(const char *const path, FILINFO *const fi);
Result fChdir(const char *const path);
Result fOpenDir(DHandle *const hOut, const char *const path);
Result fReadDir(DHandle h, FILINFO *const fi, u32 num, u32 *const entriesRead);
Result fCloseDir(DHandle h);
Result fMkdir(const char *const path);
Result fRename(const char *const old, const char *const _new);
Result fUnlink(const char *const path);

#ifdef ARM9
void fsDeinit(void);
#endif // ifdef ARM9
