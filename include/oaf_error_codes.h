#pragma once

/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2022 derrek, profi200
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

#include "error_codes.h"


#define MAKE_CUSTOM_ERR(e) (CUSTOM_ERR_OFFSET + (e))

// Keep errors in the range of 0-CUSTOM_ERR_OFFSET - 1.
enum
{
	// Custom errors.
	RES_ROM_TOO_BIG            = MAKE_CUSTOM_ERR(0u),
	RES_INVALID_PATCH          = MAKE_CUSTOM_ERR(1u),

	MAX_OAF_RES_VALUE          = RES_ROM_TOO_BIG
};

#undef MAKE_CUSTOM_ERR



const char* oafResult2String(Result res);
#ifdef ARM11
void printError(Result res);
void printErrorWaitInput(Result res, u32 waitKeys);
#endif // ifdef ARM11
