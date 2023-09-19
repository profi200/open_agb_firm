#include <string.h>
#include "types.h"
#include "arm11/save_type.h"
#include "drivers/lgy_common.h"
#include "arm11/fmt.h"
#include "fs.h"
#include "drivers/sha.h"
#include "oaf_error_codes.h"
#include "arm11/console.h"
#include "drivers/gfx.h"
#include "arm11/drivers/hid.h"



static u16 checkSaveOverride(const u32 gameCode) // Save type overrides for modern homebrew.
{
	switch (gameCode & 0xFFu)
	{
		case '1': return SAVE_TYPE_EEPROM_64k;         // Homebrew using EEPROM.
		case '2': return SAVE_TYPE_SRAM_256k;          // Homebrew using SRAM.
		case '3': return SAVE_TYPE_FLASH_512k_PSC_RTC; // Homebrew using FLASH-64.
		case '4': return SAVE_TYPE_FLASH_1m_MRX_RTC;   // Homebrew using FLASH-128.
		case 'F': return SAVE_TYPE_EEPROM_8k;          // Classic NES Series.
		case 'S': return SAVE_TYPE_SRAM_256k;          // Homebrew using SRAM (Butano games).
	}

	return 0xFF;
}

u16 detectSaveType(const u32 romSize, const u16 defaultSave)
{
	const u32 *romPtr = (u32*)LGY_ROM_LOC;
	u16 saveType = checkSaveOverride(romPtr[0xAC / 4]);
	if(saveType != 0xFF)
	{
		debug_printf("Serial in override list.\n"
		             "saveType: %u\n", saveType);
		return saveType;
	}

	// Code based on: https://github.com/Gericom/GBARunner2/blob/master/arm9/source/save/Save.vram.cpp
	romPtr += 0xE4 / 4; // Skip headers.
	if(defaultSave > SAVE_TYPE_NONE)
		saveType = SAVE_TYPE_NONE;
	else
		saveType = defaultSave;

	for(; romPtr < (u32*)(LGY_ROM_LOC + romSize); romPtr++)
	{
		u32 tmp = *romPtr;

		// "EEPR" "FLAS" "SRAM"
		if(tmp == 0x52504545u || tmp == 0x53414C46u || tmp == 0x4D415253u)
		{
			static const struct
			{
				const char *str;
				u16 saveType;
			} saveTypeLut[25] =
			{
				// EEPROM
				// Assume common sizes for popular games to aid ROM hacks.
				{"EEPROM_V111", SAVE_TYPE_EEPROM_8k},
				{"EEPROM_V120", SAVE_TYPE_EEPROM_8k},
				{"EEPROM_V121", SAVE_TYPE_EEPROM_64k},
				{"EEPROM_V122", SAVE_TYPE_EEPROM_8k},
				{"EEPROM_V124", SAVE_TYPE_EEPROM_64k},
				{"EEPROM_V125", SAVE_TYPE_EEPROM_8k},
				{"EEPROM_V126", SAVE_TYPE_EEPROM_8k},

				// FLASH
				// Assume they all have RTC.
				{"FLASH_V120",    SAVE_TYPE_FLASH_512k_PSC_RTC},
				{"FLASH_V121",    SAVE_TYPE_FLASH_512k_PSC_RTC},
				{"FLASH_V123",    SAVE_TYPE_FLASH_512k_PSC_RTC},
				{"FLASH_V124",    SAVE_TYPE_FLASH_512k_PSC_RTC},
				{"FLASH_V125",    SAVE_TYPE_FLASH_512k_PSC_RTC},
				{"FLASH_V126",    SAVE_TYPE_FLASH_512k_PSC_RTC},
				{"FLASH512_V130", SAVE_TYPE_FLASH_512k_PSC_RTC},
				{"FLASH512_V131", SAVE_TYPE_FLASH_512k_PSC_RTC},
				{"FLASH512_V133", SAVE_TYPE_FLASH_512k_PSC_RTC},
				{"FLASH1M_V102",  SAVE_TYPE_FLASH_1m_MRX_RTC},
				{"FLASH1M_V103",  SAVE_TYPE_FLASH_1m_MRX_RTC},

				// FRAM & SRAM
				{"SRAM_F_V100", SAVE_TYPE_SRAM_256k},
				{"SRAM_F_V102", SAVE_TYPE_SRAM_256k},
				{"SRAM_F_V103", SAVE_TYPE_SRAM_256k},

				{"SRAM_V110",   SAVE_TYPE_SRAM_256k},
				{"SRAM_V111",   SAVE_TYPE_SRAM_256k},
				{"SRAM_V112",   SAVE_TYPE_SRAM_256k},
				{"SRAM_V113",   SAVE_TYPE_SRAM_256k}
			};

			for(u32 i = 0; i < 25; i++)
			{
				const char *const str = saveTypeLut[i].str;
				u16 tmpSaveType = saveTypeLut[i].saveType;

				if(memcmp(romPtr, str, strlen(str)) == 0)
				{
					if(tmpSaveType == SAVE_TYPE_EEPROM_8k || tmpSaveType == SAVE_TYPE_EEPROM_64k)
					{
						// If ROM bigger than 16 MiB --> SAVE_TYPE_EEPROM_8k_2 or SAVE_TYPE_EEPROM_64k_2.
						if(romSize > 0x1000000) tmpSaveType++;
					}
					debug_printf("SDK save string: %s\n"
					             "saveType: %u\n", str, tmpSaveType);
					return tmpSaveType;
				}
			}
		}
	}

	debug_printf("saveType: %u\n", saveType);
	return saveType;
}

// Search for entry with first u64 of the SHA1 = x using binary search.
static Result searchGbaDb(const u64 x, GbaDbEntry *const db, s32 *const entryPos)
{
	debug_printf("Database search: '%016" PRIX64 "'\n", __builtin_bswap64(x));

	Result res;
	FHandle f;
	if((res = fOpen(&f, "gba_db.bin", FA_OPEN_EXISTING | FA_READ)) == RES_OK)
	{
		s32 l = 0;
		s32 r = fSize(f) / sizeof(GbaDbEntry) - 1; // TODO: Check for 0!
		while(1)
		{
			const s32 mid = l + (r - l) / 2;
			debug_printf("l: %ld r: %ld mid: %ld\n", l, r, mid);

			if((res = fLseek(f, sizeof(GbaDbEntry) * mid)) != RES_OK) break;
			if((res = fRead(f, db, sizeof(GbaDbEntry), NULL)) != RES_OK) break;
			const u64 tmp = *(u64*)db->sha1; // Unaligned access.
			if(tmp == x)
			{
				*entryPos = mid; // TODO: Remove.
				break;
			}

			if(r <= l)
			{
				debug_printf("Not found!\n");
				res = RES_NOT_FOUND;
				break;
			}

			if(tmp > x) r = mid - 1;
			else        l = mid + 1;
		}

		fClose(f);
	}

	return res;
}

u16 getSaveType(const OafConfig *const cfg, const u32 romSize, const char *const savePath)
{
	FILINFO fi;
	const bool saveOverride = cfg->saveOverride;
	const u16 autoSaveType = detectSaveType(romSize, cfg->defaultSave);
	const bool saveExists = fStat(savePath, &fi) == RES_OK;

	u64 sha1[3];
	sha((u32*)LGY_ROM_LOC, romSize, (u32*)sha1, SHA_IN_BIG | SHA_1_MODE, SHA_OUT_BIG);

	Result res;
	GbaDbEntry dbEntry;
	s32 dbPos = -1;
	u16 saveType = SAVE_TYPE_NONE;
	res = searchGbaDb(*sha1, &dbEntry, &dbPos);
	if(res == RES_OK) saveType = dbEntry.attr & 0xFu;
	else if(!saveOverride && res == RES_NOT_FOUND) return autoSaveType;
	else if(res != RES_NOT_FOUND)
	{
		ee_puts("Could not access gba_db.bin! Press any button to continue.");
		printErrorWaitInput(res, 0);
		return autoSaveType;
	}
	debug_printf("saveType: %u\n", saveType);

	if(!saveOverride) goto end;

	consoleClear();
	ee_printf("==Save Type Override Menu==\n"
	          "Save file: %s\n"
	          "Save type (autodetected): %u\n"
			  "Save type (from gba_db.bin): ", (saveExists ? "Found" : "Not found"), autoSaveType);
	if(res == RES_NOT_FOUND)
		ee_puts("Not found");
	else
		ee_printf("%u\n", saveType);
	ee_puts("\n"
	        "=Save Types=\n"
	        " EEPROM 8k (0, 1)\n"
	        " EEPROM 64k (2, 3)\n"
	        " Flash 512k RTC (4, 6, 8)\n"
	        " Flash 512k (5, 7, 9)\n"
	        " Flash 1m RTC (10, 12)\n"
	        " Flash 1m (11, 13)\n"
	        " SRAM 256k (14)\n"
	        " None (15)\n\n"
	        "=Controls=\n"
	        "Up/Down: Navigate\n"
	        "A: Select\n"
	        "X: Delete save file");

	static const u8 saveTypeCursorLut[16] = {0, 0, 1, 1, 2, 3, 2, 3, 2, 3, 4, 5, 4, 5, 6, 7};
	u8 oldCursor = 0;
	u8 cursor;
	if(!cfg->useGbaDb || res == RES_NOT_FOUND)
		cursor = saveTypeCursorLut[autoSaveType];
	else
		cursor = saveTypeCursorLut[saveType];
	while(1)
	{
		ee_printf("\x1b[%u;H ", oldCursor + 6);
		ee_printf("\x1b[%u;H>", cursor + 6);
		oldCursor = cursor;

		u32 kDown;
		do
		{
			GFX_waitForVBlank0();

			hidScanInput();
			if(hidGetExtraKeys(0) & (KEY_POWER_HELD | KEY_POWER)) goto end;
			kDown = hidKeysDown();
		} while(kDown == 0);

		if((kDown & KEY_DUP) && cursor > 0)        cursor--;
		else if((kDown & KEY_DDOWN) && cursor < 7) cursor++;
		else if(kDown & KEY_X)
		{
			fUnlink(savePath);
			ee_printf("\x1b[1;11HDeleted  ");
		}
		else if(kDown & KEY_A) break;
	}

	static const u8 cursorSaveTypeLut[8] = {0, 2, 8, 9, 10, 11, 14, 15};
	saveType = cursorSaveTypeLut[cursor];
	if(saveType == SAVE_TYPE_EEPROM_8k || saveType == SAVE_TYPE_EEPROM_64k)
	{
		// If ROM bigger than 16 MiB --> SAVE_TYPE_EEPROM_8k_2 or SAVE_TYPE_EEPROM_64k_2.
		if(romSize > 0x1000000) saveType++;
	}

end:
	return saveType;
}