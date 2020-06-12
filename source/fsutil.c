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

#include "types.h"
#include "fsutil.h"
#include "fs.h"


#define BLOCK_SIZE (1024u * 1024)



Result fsQuickRead(const char *const path, u32 off, void *buf, u32 size)
{
	Result res;
	FHandle f;
	if((res = fOpen(&f, path, FA_OPEN_EXISTING | FA_READ)) == RES_OK)
	{
		if((res = fLseek(f, off)) == RES_OK)
		{
			u32 totalRead = 0;
			u32 read;
			while(size > totalRead && (res = fRead(f, buf, BLOCK_SIZE, &read)) == RES_OK && read == BLOCK_SIZE)
			{
				buf += read;
				totalRead += read;
			}
		}

		fClose(f);
	}

	return res;
}
