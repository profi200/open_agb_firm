#pragma once

#include "types.h"


#define CUSTOM_ERR_OFFSET  (200u)
#define MAKE_CUSTOM_ERR(e) (CUSTOM_ERR_OFFSET + (e))

typedef u32 Result;

// Keep errors in the range of 0-255.
enum
{
	// Common errors.
	RES_OK              = 0u,
	RES_SD_CARD_REMOVED = 1u,
	RES_MOUNT_ERR       = 2u,
	RES_FILE_OPEN_ERR   = 3u,
	RES_FILE_READ_ERR   = 4u,
	RES_FILE_WRITE_ERR  = 5u,

	// Custom errors.
	RES_ROM_TOO_BIG     = MAKE_CUSTOM_ERR(0),
	RES_GBA_RTC_ERR     = MAKE_CUSTOM_ERR(1)
};

#undef MAKE_CUSTOM_ERR



#ifdef ARM11
void printError(Result res);
void printErrorWaitInput(Result res, u32 waitKeys);
#endif
