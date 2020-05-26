#include <string.h>
#include "types.h"
#include "arm9/hardware/lgy.h"
#include "mem_map.h"
#include "mmio.h"
#include "arm9/hardware/ndma.h"
#include "arm9/arm7_stub.h"
#include "fatfs/ff.h"
#include "arm9/debug.h"
#include "arm9/hardware/crypto.h"


#define LGY_REGS_BASE             (IO_MEM_ARM9_ONLY + 0x18000)
#define REG_LGY_MODE              *((vu16*)(LGY_REGS_BASE + 0x000))
#define REGs_LGY_A7_VECTOR         ((vu32*)(LGY_REGS_BASE + 0x080)) // ARM7 vector override 32 bytes.
#define REG_LGY_GBA_SAVE_TYPE     *((vu16*)(LGY_REGS_BASE + 0x100))
#define REG_LGY_GBA_SAVE_MAP      *((vu8* )(LGY_REGS_BASE + 0x104)) // Remaps the GBA save to ARM9 if set to 1.
#define REG_LGY_GBA_RTC_CNT       *((vu16*)(LGY_REGS_BASE + 0x108))
#define REG_LGY_GBA_RTC_BCD_DATE  *((vu32*)(LGY_REGS_BASE + 0x110))
#define REG_LGY_GBA_RTC_BCD_TIME  *((vu32*)(LGY_REGS_BASE + 0x114))
#define REG_LGY_GBA_RTC_HEX_TIME  *((vu32*)(LGY_REGS_BASE + 0x118))
#define REG_LGY_GBA_RTC_HEX_DATE  *((vu32*)(LGY_REGS_BASE + 0x11C))
#define REGs_LGY_GBA_SAVE_TIMING   ((vu32*)(LGY_REGS_BASE + 0x120))


#define MAX_ROM_SIZE    (1024u * 1024 * 32)
#define MAX_SAVE_SIZE   (1024u * 128)
#define ARM7_STUB_LOC   (0x3007E00u)
#define ARM7_STUB_LOC9  (0x80BFE00u)
#define ROM_LOC         (0x20000000u)
#define SAVE_LOC        (0x08080000u)


static FATFS g_sd = {0};
static u32 g_saveSize = 0;
static u32 g_saveHash[8] = {0};



void LGY_prepareLegacyMode(bool gbaBios)
{
	REG_LGY_MODE = 2; // GBA mode

	// BIOS overlay
	REGs_LGY_A7_VECTOR[0] = 0xE51FF004; //ldr pc, [pc, #-4]
	REGs_LGY_A7_VECTOR[1] = ARM7_STUB_LOC;
	NDMA_copy((u32*)ARM7_STUB_LOC9, _arm7_stub_start, (u32)_arm7_stub_end - (u32)_arm7_stub_start);
	//iomemcpy((u32*)ARM7_STUB_LOC9, _arm7_stub_start, (u32)_arm7_stub_end - (u32)_arm7_stub_start);
	// Patch swi 0x10 (RegisterRamReset) to swi 0x26 (HardReset).
	if(gbaBios) *((u8*)(ARM7_STUB_LOC9 + ((u32)_arm7_stub_swi - (u32)_arm7_stub_start))) = 0x26;

	REG_LGY_GBA_SAVE_TYPE = SAVE_TYPE_SRAM_256k;
	static const u32 saveStuff[4] = {0x27C886, 0x8CE35, 0x184, 0x31170};
	iomemcpy(REGs_LGY_GBA_SAVE_TIMING, saveStuff, 16);

	u32 romSize = 0;
	if(f_mount(&g_sd, "sdmc:", 1) == FR_OK)
	{
		FIL f;
		if(f_open(&f, "sdmc:/rom.gba", FA_OPEN_EXISTING | FA_READ) == FR_OK)
		{
			if((romSize = f_size(&f)) > MAX_ROM_SIZE) panic();

			u8 *ptr = (u8*)ROM_LOC;
			UINT read;
			FRESULT res;
			while((res = f_read(&f, ptr, 0x100000u, &read)) == FR_OK && read == 0x100000u)
				ptr += 0x100000u;

			if(res != FR_OK) panic();

			f_close(&f);
		}

		g_saveSize = 1024 * 32;
		if(f_open(&f, "sdmc:/rom.sav", FA_OPEN_EXISTING | FA_READ) == FR_OK)
		{
			UINT read;
			if(f_read(&f, (void*)SAVE_LOC, MAX_SAVE_SIZE, &read) != FR_OK) panic();
			f_close(&f);

			sha((u32*)SAVE_LOC, g_saveSize, g_saveHash, SHA_INPUT_BIG | SHA_MODE_256, SHA_OUTPUT_BIG);
		}
		else NDMA_fill((u32*)SAVE_LOC, 0xFFFFFFFFu, g_saveSize);
	}

	// Pad ROM area with "open bus" value.
	if(romSize < MAX_ROM_SIZE)
		NDMA_fill((u32*)(ROM_LOC + romSize), 0xFFFFFFFFu, MAX_ROM_SIZE - romSize);
}

bool LGY_setGbaRtc(GbaRtc rtc)
{
	// Set base time and date.
	REG_LGY_GBA_RTC_BCD_TIME = rtc.time;
	REG_LGY_GBA_RTC_BCD_DATE = rtc.date;

	while(REG_LGY_GBA_RTC_CNT & LGY_RTC_CNT_BUSY);
	REG_LGY_GBA_RTC_CNT = 0;
	REG_LGY_GBA_RTC_HEX_TIME = 1<<15; // Time offset 0 and 24h format.
	REG_LGY_GBA_RTC_HEX_DATE = 0;     // Date offset 0.
	REG_LGY_GBA_RTC_CNT = LGY_RTC_CNT_WR;
	while(REG_LGY_GBA_RTC_CNT & LGY_RTC_CNT_BUSY);

	return (REG_LGY_GBA_RTC_CNT & LGY_RTC_CNT_WR_ERR ? false : true);
}

bool LGY_getGbaRtc(GbaRtc *out)
{
	while(REG_LGY_GBA_RTC_CNT & LGY_RTC_CNT_BUSY);
	REG_LGY_GBA_RTC_CNT = 0;
	REG_LGY_GBA_RTC_CNT = LGY_RTC_CNT_RD;
	while(REG_LGY_GBA_RTC_CNT & LGY_RTC_CNT_BUSY);

	if((REG_LGY_GBA_RTC_CNT & LGY_RTC_CNT_WR_ERR) == 0u)
	{
		out->time = REG_LGY_GBA_RTC_BCD_TIME;
		out->date = REG_LGY_GBA_RTC_BCD_DATE;

		return true;
	}

	return false;
}

void LGY_backupGbaSave(void)
{
	if(g_saveSize)
	{
		// Enable savegame mem region.
		REG_LGY_GBA_SAVE_MAP = LGY_SAVE_MAP_9;

		u32 newHash[8];
		sha((u32*)SAVE_LOC, g_saveSize, newHash, SHA_INPUT_BIG | SHA_MODE_256, SHA_OUTPUT_BIG);
		if(memcmp(g_saveHash, newHash, 32) != 0) // Backup save if it changed.
		{
			FIL f;
			if(f_open(&f, "sdmc:/rom.sav", FA_OPEN_ALWAYS | FA_WRITE) == FR_OK)
			{
				UINT written;
				if(f_write(&f, (void*)SAVE_LOC, g_saveSize, &written) != FR_OK) panic();
				f_close(&f);
			}
		}

		// Disable savegame mem region.
		REG_LGY_GBA_SAVE_MAP = LGY_SAVE_MAP_7;
	}

	f_mount(NULL, "sdmc:", 0);
}
