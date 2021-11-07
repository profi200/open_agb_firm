#pragma once

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

#include <stdarg.h>
#include "types.h"



u32 ee_vsnprintf(char *const buf, u32 size, const char *const fmt, va_list arg);
u32 ee_vsprintf(char *const buf, const char *const fmt, va_list arg);
__attribute__ ((format (printf, 2, 3))) u32 ee_sprintf(char *const buf, const char *const fmt, ...);
__attribute__ ((format (printf, 3, 4))) u32 ee_snprintf(char *const buf, u32 size, const char *const fmt, ...);
__attribute__ ((format (printf, 1, 2))) u32 ee_printf(const char *const fmt, ...);
u32 ee_puts(const char *const str);


#ifdef NDEBUG
#define debug_printf(fmt, ...) ((void)0)
#define debug_puts(str) ((void)0)
#else
#define debug_printf(fmt, ...) ee_printf(fmt, ##__VA_ARGS__)
#define debug_puts(str) ee_puts(str)
#endif
