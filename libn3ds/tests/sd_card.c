#include "drivers/gfx.h"
#include "arm11/console.h"
#include "arm11/drivers/timer.h"
#include "drivers/toshsd.h"
#include "arm11/fmt.h"
#include "arm11/drivers/hid.h"
#include "drivers/mmc/sdmmc.h"
#include "arm11/power.h"



// 2 sector SDIO3 DMA:
/*
# 4 bytes burst with 16 transfers. Total 64 bytes per burst.
# Source fixed address and destination incrementing.
# Source and destination unprivileged, non-secure data access.
MOV CCR, SB16 SS32 SAF SP2 DB16 DS32 DAI DP2
MOV SAR, 0x10300000
MOV DAR, 0x20000000

FLUSHP 5


# Wait for a burst request.
WFP 5, burst
LP 7
	LD
	ST
LPEND
LDPB 5
ST
WFP 5, burst
LP 7
	LD
	ST
LPEND
LDPB 5
ST
WMB
END
*/
void printSlotCardInfos(void)
{
	SdmmcInfo info;
	SDMMC_getCardInfo(SDMMC_DEV_SLOT, &info);

	ee_printf("Card infos:\n type: %u\n spec_vers: %u\n rca: 0x%X\n ccc: 0x%X\n sectors: %lu\n CID: ",
	          info.type, info.spec_vers, info.rca, info.ccc, info.sectors);
	for(u32 i = 0; i < 4; i++)
	{
		ee_printf("%08lX", info.cid[i]);
	}
	ee_printf("\n Bus width: %u bit\n Clock: %lu Hz\n", info.busWidth, info.clock);
}

int main(void)
{
	GFX_init(GFX_BGR8, GFX_RGB565);
	GFX_setBrightness(15, 15);
	consoleInit(SCREEN_BOT, NULL);

	TIMER_sleepMs(1000);
	TOSHSD_init();

	ee_puts("Insert SD/MMC and press A.");
	do
	{
		do
		{
			hidScanInput();
			if(hidGetExtraKeys(0) & (KEY_POWER | KEY_POWER_HELD)) goto pooff;
			if(hidKeysDown() & KEY_A) break;
			GFX_waitForVBlank0();
		} while(1);

		ee_printf("\x1b[1;0H\x1b[0J\x1b[1;0H");
		ee_printf("Card inserted/unlocked: %u/%u\n", TOSHSD_cardDetected(), TOSHSD_cardSliderUnlocked());
		const u32 initRes = SDMMC_init(SDMMC_DEV_SLOT);
		u32 sect[1024 / 4] = {0};
		const u32 read = SDMMC_readSectors(SDMMC_DEV_SLOT, 0, sect, 1);
		const u32 read2 = SDMMC_readSectors(SDMMC_DEV_SLOT, 1, &sect[512 / 4], 1);
		printSlotCardInfos();
		SDMMC_deinit(SDMMC_DEV_SLOT);
		ee_printf("init: %lu, read: %lu, read2: %lu, sector[127]: 0x%lX\n", initRes, read, read2, sect[511 / 4]);
	} while(1);

pooff:
	GFX_deinit();
	power_off();

	return 0;
}
