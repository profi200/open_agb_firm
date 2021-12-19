#pragma once

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


#define CUSTOM_ERR_OFFSET  (200u)
#define MAKE_CUSTOM_ERR(e) (CUSTOM_ERR_OFFSET + (e))

typedef u32 Result;

// Keep errors in the range of 0-255.
enum
{
	// Common errors.
	RES_OK                     =  0u,
	RES_SD_CARD_REMOVED        =  1u,
	RES_DISK_FULL              =  2u,
	RES_INVALID_ARG            =  3u,
	RES_OUT_OF_MEM             =  4u,
	RES_OUT_OF_RANGE           =  5u,
	RES_NOT_FOUND              =  6u,
	RES_PATH_TOO_LONG          =  7u,

	// fatfs errors.
	// Caution: Update fres2Res() in fs.c on ARM9 if this changes!
	RES_FR_DISK_ERR            =  8u, /* (1) A hard error occurred in the low level disk I/O layer */
	RES_FR_INT_ERR             =  9u, /* (2) Assertion failed */
	RES_FR_NOT_READY           = 10u, /* (3) The physical drive cannot work */
	RES_FR_NO_FILE             = 11u, /* (4) Could not find the file */
	RES_FR_NO_PATH             = 12u, /* (5) Could not find the path */
	RES_FR_INVALID_NAME        = 13u, /* (6) The path name format is invalid */
	RES_FR_DENIED              = 14u, /* (7) Access denied due to prohibited access or directory full */
	RES_FR_EXIST               = 15u, /* (8) Access denied due to prohibited access */
	RES_FR_INVALID_OBJECT      = 16u, /* (9) The file/directory object is invalid */
	RES_FR_WRITE_PROTECTED     = 17u, /* (10) The physical drive is write protected */
	RES_FR_INVALID_DRIVE       = 18u, /* (11) The logical drive number is invalid */
	RES_FR_NOT_ENABLED         = 19u, /* (12) The volume has no work area */
	RES_FR_NO_FILESYSTEM       = 20u, /* (13) There is no valid FAT volume */
	RES_FR_MKFS_ABORTED        = 21u, /* (14) The f_mkfs() aborted due to any problem */
	RES_FR_TIMEOUT             = 22u, /* (15) Could not get a grant to access the volume within defined period */
	RES_FR_LOCKED              = 23u, /* (16) The operation is rejected according to the file sharing policy */
	RES_FR_NOT_ENOUGH_CORE     = 24u, /* (17) LFN working buffer could not be allocated */
	RES_FR_TOO_MANY_OPEN_FILES = 25u, /* (18) Number of open files > FF_FS_LOCK */
	RES_FR_INVALID_PARAMETER   = 26u, /* (19) Given parameter is invalid */

	// Custom errors.
	RES_ROM_TOO_BIG            = MAKE_CUSTOM_ERR(0),
	RES_GBA_RTC_ERR            = MAKE_CUSTOM_ERR(1)
};

#undef MAKE_CUSTOM_ERR



#ifdef ARM11
void printError(Result res);
void printErrorWaitInput(Result res, u32 waitKeys);
#endif
