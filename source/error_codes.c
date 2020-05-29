#include "types.h"
#include "error_codes.h"
#include "hardware/gfx.h"
#ifdef ARM11
	#include "arm11/fmt.h"
	#include "arm11/hardware/hid.h"
#endif



#ifdef ARM11
void printError(Result res)
{
	static const char *const common[] =
	{
		"OK",
		"SD card removed",
		"Failed to mount drive",
		"Failed to open file",
		"Failed to read file",
		"Failed to write file"
	};
	static const char *const custom[] =
	{
		"ROM too big. Max 32 MiB",
		"Failed to set GBA RTC"
	};

	if(res < CUSTOM_ERR_OFFSET) ee_printf("Error: %s.\n", common[res]);
	else
	{
		ee_printf("Error: %s.\n", custom[res - CUSTOM_ERR_OFFSET]);
	}
}

void printErrorWaitInput(Result res, u32 waitKeys)
{
	printError(res);

	do
	{
		GFX_waitForVBlank0();

		hidScanInput();

		if(hidKeysDown() & waitKeys) break;
		if(hidGetExtraKeys(KEY_POWER) & KEY_POWER) break;
	} while(1);
}
#endif // ifdef ARM11
