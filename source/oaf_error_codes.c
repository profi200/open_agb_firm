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

#include "oaf_error_codes.h"
#include "drivers/gfx.h"
#ifdef ARM11
	#include "arm11/fmt.h"
	#include "arm11/drivers/hid.h"
#endif



const char* oafResult2String(Result res)
{
	static const char *const oafResultStrings[] =
	{
		"ROM too big. Max 32 MiB",
		"Invalid patch file"
	};

	return (res < CUSTOM_ERR_OFFSET ? result2String(res) : oafResultStrings[res - CUSTOM_ERR_OFFSET]);
}

#ifdef ARM11
void printError(Result res)
{
	ee_printf("Error: %s.\n", oafResult2String(res));
}

void printErrorWaitInput(Result res, u32 waitKeys)
{
	printError(res);

	// In case we were already in the process of powering off
	// don't do so now. Ask the user to press power again.
	// Error messages will get lost otherwise.
	// Do not clear the power held flag here because the system
	// is powering off soon.
	(void)hidGetExtraKeys(KEY_POWER);

	while(1)
	{
		GFX_waitForVBlank0();

		hidScanInput();

		if(hidKeysDown() & waitKeys) break;
		if(hidGetExtraKeys(0) & (KEY_POWER_HELD | KEY_POWER)) break;
	}
}
#endif // ifdef ARM11
