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
		if((res = fLseek(f, size)) == RES_OK && fTell(f) == size)
		{
			fLseek(f, 0);
			res = fWrite(f, buf, size, NULL);
		}
		else if(res == RES_OK) res = RES_DISK_FULL; // Seek pre-allocation fail.

		fClose(f);
	}

	return res;
}
