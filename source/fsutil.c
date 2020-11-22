/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2020 derrek, profi200
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
#include "fsutil.h"
#include "fs.h"
#include "util.h"



Result fsQuickRead(const char *const path, void *const buf, u32 size)
{
	Result res;
	FHandle f;
	if((res = fOpen(&f, path, FA_OPEN_EXISTING | FA_READ)) == RES_OK)
	{
		res = fRead(f, buf, size, NULL);

		fClose(f);
	}

	return res;
}

Result fsQuickWrite(const char *const path, const void *const buf, u32 size)
{
	Result res;
	FHandle f;
	if((res = fOpen(&f, path, FA_OPEN_ALWAYS | FA_WRITE)) == RES_OK)
	{
		res = fWrite(f, buf, size, NULL);

		fClose(f);
	}

	return res;
}

Result fsMakePath(const char *const path)
{
	char tmpPath[512];
	safeStrcpy(tmpPath, path, 512);

	char *str;
	if((str = strchr(tmpPath, ':')) == NULL) str = tmpPath;
	else                                     str++;

	// Path without any dir.
	if(*str == '\0') return RES_INVALID_ARG;

	Result res = RES_OK;
	while((str = strchr(str + 1, '/')) != NULL)
	{
		*str = '\0';
		if((res = fMkdir(tmpPath)) != RES_OK && res != RES_FR_EXIST) break;
		*str = '/';
	}

	// Only create the last dir in the path if the
	// previous error code is not an unexpected one.
	if(res == RES_OK || res == RES_FR_EXIST) res = fMkdir(tmpPath);

	return res;
}
