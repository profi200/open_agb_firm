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

#include "types.h"
#include "error_codes.h"
#include "drivers/gfx.h"
#ifdef ARM11
	#include "arm11/fmt.h"
	#include "arm11/drivers/hid.h"
#endif



#ifdef ARM11
void printError(Result res)
{
	static const char *const common[] =
	{
		"OK",
		"SD card removed",
		"Disk full",
		"Invalid argument",
		"Out of memory",
		"Out of range",
		"Not found",
		"Path too long",

		// FatFs errors.
		"FatFs disk error",
		"FatFs assertion failed",
		"FatFs disk not ready",
		"FatFs file not found",
		"FatFs path not found",
		"FatFs invalid path name",
		"FatFs access denied",
		"FatFs already exists",
		"FatFs invalid file/directory object",
		"FatFs drive write protected",
		"FatFs invalid drive",
		"FatFs drive not mounted",
		"FatFs no filesystem",
		"FatFs f_mkfs() aborted",
		"FatFs thread lock timeout",
		"FatFs file locked",
		"FatFs not enough memory",
		"FatFs too many open objects",
		"FatFs invalid parameter"
	};
	static const char *const custom[] =
	{
		"ROM too big. Max 32 MiB",
		"Failed to set GBA RTC"
	};

	const char *const errStr = (res < CUSTOM_ERR_OFFSET ? common[res] : custom[res - CUSTOM_ERR_OFFSET]);
	ee_printf("Error: %s.\n", errStr);
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
