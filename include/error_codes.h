#pragma once

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
	RES_INVALID_ARG            =  2u,
	RES_OUT_OF_MEM             =  3u,
	RES_OUT_OF_RANGE           =  4u,
	RES_NOT_FOUND              =  5u,

	// fatfs errors.
	// Caution: Update fres2Res() in fs.c on ARM9 if this changes!
	RES_FR_DISK_ERR            =  6u, /* (1) A hard error occurred in the low level disk I/O layer */
	RES_FR_INT_ERR             =  7u, /* (2) Assertion failed */
	RES_FR_NOT_READY           =  8u, /* (3) The physical drive cannot work */
	RES_FR_NO_FILE             =  9u, /* (4) Could not find the file */
	RES_FR_NO_PATH             = 10u, /* (5) Could not find the path */
	RES_FR_INVALID_NAME        = 11u, /* (6) The path name format is invalid */
	RES_FR_DENIED              = 12u, /* (7) Access denied due to prohibited access or directory full */
	RES_FR_EXIST               = 13u, /* (8) Access denied due to prohibited access */
	RES_FR_INVALID_OBJECT      = 14u, /* (9) The file/directory object is invalid */
	RES_FR_WRITE_PROTECTED     = 15u, /* (10) The physical drive is write protected */
	RES_FR_INVALID_DRIVE       = 16u, /* (11) The logical drive number is invalid */
	RES_FR_NOT_ENABLED         = 17u, /* (12) The volume has no work area */
	RES_FR_NO_FILESYSTEM       = 18u, /* (13) There is no valid FAT volume */
	RES_FR_MKFS_ABORTED        = 19u, /* (14) The f_mkfs() aborted due to any problem */
	RES_FR_TIMEOUT             = 20u, /* (15) Could not get a grant to access the volume within defined period */
	RES_FR_LOCKED              = 21u, /* (16) The operation is rejected according to the file sharing policy */
	RES_FR_NOT_ENOUGH_CORE     = 22u, /* (17) LFN working buffer could not be allocated */
	RES_FR_TOO_MANY_OPEN_FILES = 23u, /* (18) Number of open files > FF_FS_LOCK */
	RES_FR_INVALID_PARAMETER   = 24u, /* (19) Given parameter is invalid */

	// Custom errors.
	RES_ROM_TOO_BIG            = MAKE_CUSTOM_ERR(0),
	RES_GBA_RTC_ERR            = MAKE_CUSTOM_ERR(1)
};

#undef MAKE_CUSTOM_ERR



#ifdef ARM11
void printError(Result res);
void printErrorWaitInput(Result res, u32 waitKeys);
#endif
