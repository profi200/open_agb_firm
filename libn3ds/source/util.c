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

#include <ctype.h>
#include <string.h>
#include "types.h"
#include "util.h"



NAKED void wait_cycles(u32 cycles)
{
#ifdef ARM9
	__asm__("1: subs %0, %0, #4\n\t"
#elif ARM11
	__asm__("1: subs %0, %0, #2\n\t"
	        "yield\n\t"
#endif
	        "bhi 1b\n\t"
	        "bx lr\n\t" : : "r" (cycles) : "cc");
}

size_t safeStrcpy(char *const dst, const char *const src, size_t num)
{
	if(num == 0) return 0;

	const size_t len = strlen(src) + 1;
	if(len > num)
	{
		*dst = '\0';
		return 1;
	}

	memcpy(dst, src, len);

	return len;
}

// Limited to 6 decimal places. Doesn't support exponents.
// Based on: https://codereview.stackexchange.com/a/158724
float str2float(const char *str)
{
	for(; isspace((unsigned char)*str) != 0; str++); // Skip whitespaces.

	const float sign = (*str == '-' ? -1.f : 1.f);
	if(*str == '-' || *str == '+') str++;

	float val = 0.f;
	while(isdigit((unsigned char)*str) != 0)
	{
		val = val * 10.f + (*str - '0');
		str++;
	}

	if(*str == '.') str++;

	u32 place = 1;
	while(isdigit((unsigned char)*str) != 0)
	{
		val = val * 10.f + (*str - '0');
		place *= 10;
		str++;
	}

	return val * sign / place;
}

// case insensitive string compare function
int strnicmp(const char *str1, const char *str2, u32 len)
{
	int diff;
	
	if(len == 0)
		return 0;
	
	for(;; str1++, str2++)
	{	
		if(len != 0) len --;
		else return 0;
		diff = tolower(*str1) - tolower(*str2);
		if(diff != 0 || *str1 == '\0')
			return diff;
	}
	
	return 0;
}

// custom safe strncpy, string is always 0-terminated for buflen > 0
void strncpy_s(char *dest, const char *src, u32 nchars, const u32 buflen)
{
	char c;

	if(!buflen)
		return;
		
	if(buflen > 1)
	{
		if(nchars >= buflen)
			nchars = buflen - 1;
		
		while(nchars--)
		{
			c = *src++;
			
			if(c == '\0')
				break;
			
			*dest++ = c;
		}
	}
	
	*dest = '\0';
}

// custom safe memcpy
void memcpy_s(void *dstBuf, size_t dstBufSize, size_t dstBufOffset,
				void *srcBuf, size_t srcBufSize, size_t srcBufOffset, bool reverse)
{
	size_t dstRemaining, srcRemaining, actualSize;
	
	if(dstBufOffset >= dstBufSize)
		return;
	if(srcBufOffset >= srcBufSize)
		return;
	
	dstRemaining = dstBufSize - dstBufOffset;
	srcRemaining = srcBufSize - srcBufOffset;
	
	actualSize = min(dstRemaining, srcRemaining);
	
	if(reverse)
	{
		u8 *src = srcBuf + srcBufOffset + actualSize - 1;
		u8 *dst = dstBuf + dstBufOffset + actualSize - 1;
		
		do {
			*dst = *src;
			dst--;
			src--;
			actualSize --;
		} while(actualSize);
	}
	else
		memcpy(dstBuf+dstBufOffset, srcBuf+srcBufOffset, actualSize);
}

u32 getleu32(const void* ptr)
{
	const u8* p = (const u8*)ptr;
	
	return (u32)((p[0]<<0) | (p[1]<<8) | (p[2]<<16) | (p[3]<<24));
}

u32 swap32(u32 val)
{
	return ((val>>24) |
	        (val>>8  & 0x0000FF00) |
	        (val<<8  & 0x00FF0000) |
	        (val<<24));
}
