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
#include <string.h>
#include "fsutil.h"
#include "fs.h"
#include "util.h"



Result fsQuickRead(const char *const path, void *const buf, u32 size) // TODO: Output how many bytes we read?
{
	FHandle f;
	Result res = fOpen(&f, path, FA_OPEN_EXISTING | FA_READ);
	if(res == RES_OK)
	{
		res = fRead(f, buf, size, NULL);

		fClose(f);
	}

	return res;
}

Result fsQuickWrite(const char *const path, const void *const buf, u32 size)
{
	FHandle f;
	Result res = fOpen(&f, path, FA_OPEN_ALWAYS | FA_WRITE);
	Result closeRes;
	if(res == RES_OK)
	{
		res = fWrite(f, buf, size, NULL);

		// Because of the disk cache errors on small writes
		// happen on close when the cache is written back to disk.
		closeRes = fClose(f);
	}

	return (res != RES_OK ? res : closeRes);;
}

Result fsMakePath(const char *const path)
{
	Result res = fMkdir(path);
	if(res != RES_FR_NO_PATH) return res;

	char *tmpPath = (char*)malloc(512);
	if(tmpPath == NULL) return RES_OUT_OF_MEM;
	safeStrcpy(tmpPath, path, 512);

	char *str;
	if((str = strchr(tmpPath, ':')) == NULL) str = tmpPath;
	else                                     str++;

	// Empty path.
	if(*str == '\0')
	{
		free(tmpPath);
		return RES_INVALID_ARG;
	}

	while((str = strchr(str + 1, '/')) != NULL)
	{
		*str = '\0';
		if((res = fMkdir(tmpPath)) != RES_OK && res != RES_FR_EXIST) break;
		*str = '/';
	}

	// Only create the last dir in the path if the
	// previous error code is not an unexpected one.
	if(res == RES_OK || res == RES_FR_EXIST) res = fMkdir(tmpPath);

	free(tmpPath);

	return res;
}

Result fsLoadPathFromFile(const char *const path, char outPath[512])
{
	Result res = fsQuickRead(path, outPath, 511);
	if(res == RES_OK)
	{
		char *const invalidChar = strpbrk(outPath, "\n\r\t");
		if(invalidChar != NULL) *invalidChar = '\0';
	}

	return res;
}
