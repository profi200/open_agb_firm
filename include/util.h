#pragma once

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

#define min(a,b)	((size_t) (a) <= (size_t) (b) ? (size_t) (a) : (size_t) (b))

#define arrayEntries(array)	sizeof(array)/sizeof(*array)



NAKED void wait(u32 cycles);

// case insensitive string compare function
int strnicmp(const char *str1, const char *str2, u32 len);

// custom safe strncpy, string is always 0-terminated for buflen > 0
void strncpy_s(char *dest, const char *src, u32 nchars, const u32 buflen);

void memcpy_s(void *dstBuf, size_t dstBufSize, size_t dstBufOffset,
              void *srcBuf, size_t srcBufSize, size_t srcBufOffset, bool reverse);

u32 getleu32(const void* ptr);

u32 swap32(u32 val);

static inline u32 intLog2(u32 val)
{
	// The result is undefined if __builtin_clz() is called with 0.
	return (val ? 31u - __builtin_clz(val) : 0u);
}

// Round up to the next power of 2.
static inline u32 nextPow2(u32 val)
{
	// Portable variant:
	// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
	/*val--;
	val |= val>>1;
	val |= val>>2;
	val |= val>>4;
	val |= val>>8;
	val |= val>>16;
	val++;

	return val;*/

	// Warning: Allowed range is 2 - 2147483648.
	// Everything else is undefined behavior.
	return 1u<<(32u - __builtin_clz(val - 1));
}
