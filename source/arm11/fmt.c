/*
*   This file is part of Luma3DS
*   Copyright (C) 2016-2020 Aurora Wright, TuxSH
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
*
*   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
*       * Requiring preservation of specified reasonable legal notices or
*         author attributions in that material or in the Appropriate Legal
*         Notices displayed by works containing it.
*       * Prohibiting misrepresentation of the origin of that material,
*         or requiring that modified versions of such material be marked in
*         reasonable ways as different from the original version.
*/

/* File : barebones/ee_printf.c
	This file contains an implementation of ee_printf that only requires a method to output a char to a UART without pulling in library code.

This code is based on a file that contains the following:
 Copyright (C) 2002 Michael Ringgaard. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.
 3. Neither the name of the project nor the names of its contributors
	may be used to endorse or promote products derived from this software
	without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

*/

//TuxSH's changes: add support for 64-bit numbers, remove floating-point code

#include <stdarg.h>
#include <string.h>
#include "types.h"
#include "arm11/fmt.h"
#include "arm11/console.h"

#define ZEROPAD   (1<<0) //Pad with zero
#define SIGN      (1<<1) //Unsigned/signed long
#define PLUS      (1<<2) //Show plus
#define SPACE     (1<<3) //Spacer
#define LEFT      (1<<4) //Left justified
#define HEX_PREP  (1<<5) //0x
#define UPPERCASE (1<<6) //'ABCDEF'

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')

static s32 skipAtoi(const char **s)
{
	s32 i = 0;

	while(IS_DIGIT(**s)) i = i * 10 + *((*s)++) - '0';

	return i;
}

static char *processNumber(char *str, const char *const strEnd, s64 num, bool isHex, s32 size, s32 precision, u32 type)
{
	char sign = 0;

	if(type & SIGN)
	{
		if(num < 0)
		{
			sign = '-';
			num = -num;
			size--;
		}
		else if(type & PLUS)
		{
			sign = '+';
			size--;
		}
		else if(type & SPACE)
		{
			sign = ' ';
			size--;
		}
	}

	static const char *lowerDigits = "0123456789abcdef",
					  *upperDigits = "0123456789ABCDEF";

	s32 i = 0;
	char tmp[20];
	const char *dig = (type & UPPERCASE) ? upperDigits : lowerDigits;

	if(num == 0)
	{
		if(precision != 0) tmp[i++] = '0';
		type &= ~HEX_PREP;
	}
	else
	{
		while(num != 0)
		{
			u64 base = isHex ? 16ULL : 10ULL;
			tmp[i++] = dig[(u64)num % base];
			num = (s64)((u64)num / base);
		}
	}

	if(type & LEFT || precision != -1) type &= ~ZEROPAD;
	if(type & HEX_PREP && isHex) size -= 2;
	if(i > precision) precision = i;
	size -= precision;
	if(!(type & (ZEROPAD | LEFT)))
		while(size-- > 0)
		{
			if(str >= strEnd) goto end;
			*str++ = ' ';
		}
	if(sign)
	{
		if(str >= strEnd) goto end;
		*str++ = sign;
	}

	if(type & HEX_PREP && isHex)
	{
		if(str + 2 >= strEnd) goto end;
		*str++ = '0';
		*str++ = 'x';
	}

	if(type & ZEROPAD)
		while(size-- > 0)
		{
			if(str >= strEnd) goto end;
			*str++ = '0';
		}
	while(i < precision--)
	{
		if(str >= strEnd) goto end;
		*str++ = '0';
	}
	while(i-- > 0)
	{
		if(str >= strEnd) goto end;
		*str++ = tmp[i];
	}
	while(size-- > 0)
	{
		if(str >= strEnd) goto end;
		*str++ = ' ';
	}

end:
	return str;
}

u32 ee_vsnprintf(char *buf, u32 size, const char *fmt, va_list args)
{
	if(size == 0) return 0;
	const char *const strEnd = buf + size - 1;

	char *str;
	for(str = buf; *fmt; fmt++)
	{
		if(*fmt != '%')
		{
			if(str >= strEnd) break;
			*str++ = *fmt;
			continue;
		}

		//Process flags
		u32 flags = 0; //Flags to number()
		bool loop = true;

		while(loop)
		{
			switch(*++fmt)
			{
				case '-': flags |= LEFT; break;
				case '+': flags |= PLUS; break;
				case ' ': flags |= SPACE; break;
				case '#': flags |= HEX_PREP; break;
				case '0': flags |= ZEROPAD; break;
				default: loop = false; break;
			}
		}

		//Get field width
		s32 fieldWidth = -1; //Width of output field
		if(IS_DIGIT(*fmt)) fieldWidth = skipAtoi(&fmt);
		else if(*fmt == '*')
		{
			fmt++;

			fieldWidth = va_arg(args, s32);

			if(fieldWidth < 0)
			{
				fieldWidth = -fieldWidth;
				flags |= LEFT;
			}
		}

		//Get the precision
		s32 precision = -1; //Min. # of digits for integers; max number of chars for from string
		if(*fmt == '.')
		{
			fmt++;

			if(IS_DIGIT(*fmt)) precision = skipAtoi(&fmt);
			else if(*fmt == '*')
			{
				fmt++;
				precision = va_arg(args, s32);
			}

			if(precision < 0) precision = 0;
		}

		//Get the conversion qualifier
		u32 integerType = 0;
		if(*fmt == 'l')
		{
			if(*++fmt == 'l')
			{
				fmt++;
				integerType = 1;
			}
			
		}
		else if(*fmt == 'h')
		{
			if(*++fmt == 'h')
			{
				fmt++;
				integerType = 3;
			}
			else integerType = 2;
		}

		bool isHex;

		switch(*fmt)
		{
			case 'c':
				if(!(flags & LEFT))
					while(--fieldWidth > 0)
					{
						if(str >= strEnd) goto end;
						*str++ = ' ';
					}
				if(str >= strEnd) goto end;
				*str++ = (u8)va_arg(args, s32);
				while(--fieldWidth > 0)
				{
					if(str >= strEnd) goto end;
					*str++ = ' ';
				}
				continue;

			case 's':
			{
				char *s = va_arg(args, char *);
				if(!s) s = "<NULL>";
				u32 len = (precision != -1) ? strnlen(s, precision) : strlen(s);
				if(!(flags & LEFT))
					while((s32)len < fieldWidth--)
					{
						if(str >= strEnd) goto end;
						*str++ = ' ';
					}
				for(u32 i = 0; i < len; i++)
				{
					if(str >= strEnd) goto end;
					*str++ = *s++;
				}
				while((s32)len < fieldWidth--)
				{
					if(str >= strEnd) goto end;
					*str++ = ' ';
				}
				continue;
			}

			case 'p':
				if(fieldWidth == -1)
				{
					fieldWidth = 8;
					flags |= ZEROPAD;
				}
				str = processNumber(str, strEnd, va_arg(args, u32), true, fieldWidth, precision, flags);
				continue;

			//Integer number formats - set up the flags and "break"
			case 'X':
				flags |= UPPERCASE;
				//Falls through
			case 'x':
				isHex = true;
				break;

			case 'd':
			case 'i':
				flags |= SIGN;
				//Falls through
			case 'u':
				isHex = false;
				break;

			default:
				if(*fmt != '%')
				{
					if(str >= strEnd) goto end;
					*str++ = '%';
				}
				if(*fmt)
				{
					if(str >= strEnd) goto end;
					*str++ = *fmt;
				}
				else fmt--;
				continue;
		}

		s64 num;

		if(flags & SIGN)
		{
			if(integerType == 1) num = va_arg(args, s64);
			else num = va_arg(args, s32);

			if(integerType == 2) num = (s16)num;
			else if(integerType == 3) num = (s8)num;
		}
		else
		{
			if(integerType == 1) num = va_arg(args, u64);
			else num = va_arg(args, u32);

			if(integerType == 2) num = (u16)num;
			else if(integerType == 3) num = (u8)num;
		}

		str = processNumber(str, strEnd, num, isHex, fieldWidth, precision, flags);
	}

end:
	*str = 0;
	return str - buf;
}

u32 ee_vsprintf(char *const buf, const char *const fmt, va_list arg)
{
	return ee_vsnprintf(buf, 0x1000, fmt, arg);
}

__attribute__ ((format (printf, 2, 3))) u32 ee_sprintf(char *const buf, const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	u32 res = ee_vsnprintf(buf, 0x1000, fmt, args);
	va_end(args);

	return res;
}

__attribute__ ((format (printf, 3, 4))) u32 ee_snprintf(char *const buf, u32 size, const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	u32 res = ee_vsnprintf(buf, size, fmt, args);
	va_end(args);

	return res;
}

__attribute__ ((format (printf, 1, 2))) u32 ee_printf(const char *const fmt, ...)
{
	char buf[512];
	va_list args;
	va_start(args, fmt);
	u32 res = ee_vsnprintf(buf, 512, fmt, args);
	va_end(args);

	con_write(NULL, NULL, buf, res);

	return res;
}

u32 ee_puts(const char *const str)
{
	con_write(NULL, NULL, str, strnlen(str, 512));
	con_write(NULL, NULL, "\n", 1);
	return 0;
}
