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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "oaf_error_codes.h"
#include "arm_intrinsic.h"
#include "util.h"
#include "drivers/sha.h"
#include "arm11/drivers/hid.h"
#include "drivers/lgy.h"
#include "arm11/drivers/lgyfb.h"
#include "arm11/console.h"
#include "arm11/fmt.h"
#include "drivers/gfx.h"
#include "fs.h"
#include "fsutil.h"
#include "inih/ini.h"
#include "arm11/filebrowser.h"
#include "arm11/drivers/lcd.h"
#include "arm11/gpu_cmd_lists.h"
#include "arm11/drivers/codec.h"
#include "arm11/drivers/mcu.h"
#include "arm11/patch.h"
#include "kernel.h"
#include "kevent.h"


#define OAF_WORK_DIR    "sdmc:/3ds/open_agb_firm"
#define OAF_SAVE_DIR    "saves"                   // Relative to work dir.
#define INI_BUF_SIZE    (1024u)
#define DEFAULT_CONFIG  "[general]\n"             \
                        "backlight=64\n"          \
                        "backlightSteps=5\n"      \
                        "directBoot=false\n"      \
                        "useGbaDb=true\n\n"       \
                        "[video]\n"               \
                        "scaler=2\n"              \
                        "gbaGamma=2.2\n"          \
                        "lcdGamma=1.54\n"         \
                        "contrast=1.0\n"          \
                        "brightness=0.0\n\n"      \
                        "[audio]\n"               \
                        "audioOut=0\n\n"          \
                        "[advanced]\n"            \
                        "saveOverride=false\n"    \
                        "defaultSave=14"

typedef struct
{
	// [general]
	u8 backlight;      // Both LCDs.
	u8 backlightSteps;
	bool directBoot;
	bool useGbaDb;

	// [video]
	u8 scaler;         // 0 = 1:1, 1 = bilinear (GPU) x1.5, 2 = matrix (hardware) x1.5.
	float gbaGamma;
	float lcdGamma;
	float contrast;
	float brightness;

	// [audio]
	u8 audioOut;       // 0 = auto, 1 = speakers, 2 = headphones.

	// [game]
	u8 saveSlot;
	u8 saveType;

	// [advanced]
	bool saveOverride;
	u16 defaultSave;
} OafConfig;

typedef struct
{
	char name[200];
	char serial[4];
	u8 sha1[20];
	u32 attr;
} GameDbEntry;


// Default config.
static OafConfig g_oafConfig =
{
	// [general]
	64,    // backlight
	5,     // backlightSteps
	false, // directBoot
	true,  // useGbaDb

	// [video]
	2,     // scaler
	2.2f,  // gbaGamma
	1.54f, // lcdGamma
	1.f,   // contrast
	0.f,   // brightness

	// [audio]
	0,     // Automatic audio output.

	// [game]
	0,     // saveSlot
	0xFF,  // saveType

	// [advanced]
	false, // saveOverride
	14     // defaultSave
};
static KHandle g_frameReadyEvent = 0;

static u32 fixRomPadding(u32 romFileSize)
{
	// Pad unused ROM area with 0xFFs (trimmed ROMs).
	// Smallest retail ROM chip is 8 Mbit (1 MiB).
	u32 romSize = nextPow2(romFileSize);
	if(romSize < 0x100000u) romSize = 0x100000u;
	memset((void*)(ROM_LOC + romFileSize), 0xFFFFFFFFu, romSize - romFileSize);
	if(romSize > 0x100000u) // >1 MiB.
	{
		// Fake "open bus" padding.
		u32 padding = (ROM_LOC + romSize) / 2;
		padding = __pkhbt(padding, padding + 1, 16); // Copy lower half + 1 to upper half.
		for(uintptr_t i = ROM_LOC + romSize; i < ROM_LOC + MAX_ROM_SIZE; i += 4)
		{
			*(u32*)i = padding;
			padding = __uadd16(padding, 0x00020002u); // Unsigned parallel halfword-wise addition.
		}
	}
	else
	{

		// ROM mirroring (Classic NES Series/possibly others with 8 Mbit ROM).
		// Mirror ROM across the entire 32 MiB area.
		for(uintptr_t i = ROM_LOC + romSize; i < ROM_LOC + MAX_ROM_SIZE; i += romSize)
		{
			//memcpy((void*)i, (void*)(i - romSize), romSize); // 0x23A15DD
			memcpy((void*)i, (void*)ROM_LOC, romSize); // 0x237109B
		}
	}

	return romSize;
}

static Result loadGbaRom(const char *const path, u32 *const romSizeOut)
{
	Result res;
	FHandle f;
	if((res = fOpen(&f, path, FA_OPEN_EXISTING | FA_READ)) == RES_OK)
	{
		u32 fileSize = fSize(f);
		if(fileSize > MAX_ROM_SIZE)
		{
			fileSize = MAX_ROM_SIZE;
			ee_puts("Warning: ROM file is too big. Expect crashes.");
		}

		u32 read;
		res = fRead(f, (u8*)ROM_LOC, fileSize, &read);
		fClose(f);

		if(read == fileSize) *romSizeOut = fixRomPadding(fileSize); //, path);

	}

	return res;
}

static u16 checkSaveOverride(u32 gameCode) // Save type overrides for modern homebrew.
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

static u16 detectSaveType(u32 romSize)
{
	const u32 *romPtr = (u32*)ROM_LOC;
	u16 saveType;
	if((saveType = checkSaveOverride(romPtr[0xAC / 4])) != 0xFF)
	{
		debug_printf("Serial in override list.\n"
		             "saveType: %u\n", saveType);
		return saveType;
	}

	// Code based on: https://github.com/Gericom/GBARunner2/blob/master/arm9/source/save/Save.vram.cpp
	romPtr += 0xE4 / 4; // Skip headers.
	const u16 defaultSave = g_oafConfig.defaultSave;
	if(defaultSave > SAVE_TYPE_NONE)
		saveType = SAVE_TYPE_NONE;
	else
		saveType = defaultSave;

	for(; romPtr < (u32*)(ROM_LOC + romSize); romPtr++)
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
static Result searchGbaDb(u64 x, GameDbEntry *const db, s32 *const entryPos)
{
	debug_printf("Database search: '%016" PRIX64 "'\n", __builtin_bswap64(x));

	Result res;
	FHandle f;
	if((res = fOpen(&f, "gba_db.bin", FA_OPEN_EXISTING | FA_READ)) == RES_OK)
	{
		s32 l = 0;
		s32 r = fSize(f) / sizeof(GameDbEntry) - 1; // TODO: Check for 0!
		while(1)
		{
			const s32 mid = l + (r - l) / 2;
			debug_printf("l: %ld r: %ld mid: %ld\n", l, r, mid);

			if((res = fLseek(f, sizeof(GameDbEntry) * mid)) != RES_OK) break;
			if((res = fRead(f, db, sizeof(GameDbEntry), NULL)) != RES_OK) break;
			const u64 tmp = *(u64*)db->sha1; // Unaligned access.
			if(tmp == x)
			{
				*entryPos = mid; // TODO: Remove.
				break;
			}

			if(r <= l)
			{
				debug_printf("Not found!");
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

static u16 getSaveType(u32 romSize, const char *const savePath)
{
	FILINFO fi;
	const bool saveOverride = g_oafConfig.saveOverride;
	const u16 autoSaveType = detectSaveType(romSize);
	const bool saveExists = fStat(savePath, &fi) == RES_OK;

	u64 sha1[3];
	sha((u32*)ROM_LOC, romSize, (u32*)sha1, SHA_IN_BIG | SHA_1_MODE, SHA_OUT_BIG);

	Result res;
	GameDbEntry dbEntry;
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

	if(saveOverride)
	{
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
		if(!g_oafConfig.useGbaDb || res == RES_NOT_FOUND)
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
	}

end:
	return saveType;
}

static void adjustGammaTableForGba(void)
{
	const float gbaGamma = g_oafConfig.gbaGamma;
	const float lcdGamma = g_oafConfig.lcdGamma;
	const float contrast = g_oafConfig.contrast;
	const float brightness = g_oafConfig.brightness;
	for(u32 i = 0; i < 256; i++)
	{
		// Credits for this algo go to Extrems.
		// Originally from Game Boy Interface Standard Edition for the GameCube.
		u32 res = powf(powf(contrast, gbaGamma) * powf((float)i / 255.0f + brightness / contrast, gbaGamma),
		               1.0f / lcdGamma) * 255.0f;

		// Same adjustment for red/green/blue.
		REG_LCD_PDC0_GTBL_FIFO = res<<16 | res<<8 | res;
	}
}

static Result dumpFrameTex(void)
{
	// Stop LgyFb before dumping the frame to prevent glitches.
	LGYFB_stop();

	// 512x-512 (hight negative to flip vertically).
	// Pixels at offset 0x40.
	alignas(4) static const u8 bmpHeader[54] =
	{
		0x42, 0x4D, 0x40, 0x00, 0x0C, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x28, 0x00,
		0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xFE,
		0xFF, 0xFF, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x13, 0x0B,
		0x00, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	/*GX_displayTransfer((u32*)0x18200000, 160u<<16 | 256u, (u32*)0x18400000, 160u<<16 | 256u, 1u<<12 | 1u<<8);
	GFX_waitForPPF();
	//fsQuickWrite("sdmc:/lgyfb_dbg_frame.bgr", (void*)0x18400000, 256 * 160 * 3);*/
	GX_displayTransfer((u32*)0x18200000, 240u<<16 | 512u, (u32*)0x18400040, 240u<<16 | 512u, 1u<<12 | 1u<<8);
	GFX_waitForPPF();
	memcpy((void*)0x18400000, bmpHeader, sizeof(bmpHeader));

	RtcTimeDate td;
	char fn[32];
	MCU_getRtcTimeDate(&td);
	ee_sprintf(fn, "texture_dump_%04X%02X%02X%02X%02X%02X.bmp", td.y + 0x2000, td.mon, td.d, td.h, td.min, td.s);
	const Result res = fsQuickWrite(fn, (void*)0x18400000, 0x40 + 512 * 512 * 3);

	// Restart LgyFb.
	LGYFB_start();

	return res;
}

static void gbaGfxHandler(void *args)
{
	const KHandle event = (KHandle)args;

	while(1)
	{
		if(waitForEvent(event) != KRES_OK) break;
		clearEvent(event);

		// Rotate the frame using the GPU.
		// 240x160: TODO.
		// 360x240: about 0.623620315 ms.
		static bool inited = false;
		u32 listSize;
		const u32 *list;
		if(inited == false)
		{
			inited = true;

			listSize = sizeof(gbaGpuInitList);
			list = (u32*)gbaGpuInitList;
		}
		else
		{
			listSize = sizeof(gbaGpuList2);
			list = (u32*)gbaGpuList2;
		}
		GX_processCommandList(listSize, list);
		GFX_waitForP3D();
		GX_displayTransfer((u32*)(0x18180000 + (16 * 240 * 3)), 368u<<16 | 240u,
		                   GFX_getFramebuffer(SCREEN_TOP) + (16 * 240 * 3), 368u<<16 | 240u, 1u<<12 | 1u<<8);
		CODEC_runHeadphoneDetection(); // Run headphone detection while PPF is busy.
		GFX_waitForPPF();
		GFX_swapFramebufs();

		// Trigger only if both are held and at least one is detected as newly pressed down.
		if(hidKeysHeld() == (KEY_Y | KEY_SELECT) && hidKeysDown() != 0)
			dumpFrameTex();
	}

	taskExit();
}

static int cfgIniCallback(void* user, const char* section, const char* name, const char* value)
{
	OafConfig *const config = (OafConfig*)user;

	if(strcmp(section, "general") == 0)
	{
		if(strcmp(name, "backlight") == 0)
			config->backlight = (u8)strtoul(value, NULL, 10);
		else if(strcmp(name, "backlightSteps") == 0)
			config->backlightSteps = (u8)strtoul(value, NULL, 10);
		else if(strcmp(name, "directBoot") == 0)
			config->directBoot = (strcmp(value, "false") == 0 ? false : true);
		else if(strcmp(name, "useGbaDb") == 0)
			config->useGbaDb = (strcmp(value, "true") == 0 ? true : false);
	}
	else if(strcmp(section, "video") == 0)
	{
		if(strcmp(name, "scaler") == 0)
			config->scaler = (u8)strtoul(value, NULL, 10);
		else if(strcmp(name, "gbaGamma") == 0)
			config->gbaGamma = str2float(value);
		else if(strcmp(name, "lcdGamma") == 0)
			config->lcdGamma = str2float(value);
		else if(strcmp(name, "contrast") == 0)
			config->contrast = str2float(value);
		else if(strcmp(name, "brightness") == 0)
			config->brightness = str2float(value);
	}
	else if(strcmp(section, "audio") == 0)
	{
		if(strcmp(name, "audioOut") == 0)
			config->audioOut = (u8)strtoul(value, NULL, 10);
	}
	else if(strcmp(section, "game") == 0)
	{
		if(strcmp(name, "saveSlot") == 0)
			config->saveSlot = (u8)strtoul(value, NULL, 10);
		if(strcmp(name, "saveType") == 0)
			config->saveType = (u8)strtoul(value, NULL, 10);
	}
	else if(strcmp(section, "advanced") == 0)
	{
		if(strcmp(name, "saveOverride") == 0)
			config->saveOverride = (strcmp(value, "false") == 0 ? false : true);
		if(strcmp(name, "defaultSave") == 0)
			config->defaultSave = (u16)strtoul(value, NULL, 10);
	}
	else return 0; // Error.

	return 1; // 1 is no error? Really?
}

static Result parseOafConfig(const char *const path, const bool writeDefaultCfg)
{
	char *iniBuf = (char*)calloc(INI_BUF_SIZE, 1);
	if(iniBuf == NULL) return RES_OUT_OF_MEM;

	Result res = fsQuickRead(path, iniBuf, INI_BUF_SIZE - 1);
	if(res == RES_OK) ini_parse_string(iniBuf, cfgIniCallback, &g_oafConfig);
	else if(writeDefaultCfg)
	{
		const char *const defaultConfig = DEFAULT_CONFIG;
		res = fsQuickWrite(path, defaultConfig, strlen(defaultConfig));
	}

	free(iniBuf);

	return res;
}

void changeBacklight(s16 amount)
{
	u8 min, max;
	if(MCU_getSystemModel() >= 4)
	{
		min = 16;
		max = 142;
	}
	else
	{
		min = 20;
		max = 117;
	}

	s16 newVal = g_oafConfig.backlight + amount;
	newVal = (newVal > max ? max : newVal);
	newVal = (newVal < min ? min : newVal);
	g_oafConfig.backlight = (u8)newVal;

	GFX_setBrightness((u8)newVal, (u8)newVal);
}

static void updateBacklight(void)
{
	// Check for special button combos.
	const u32 kHeld = hidKeysHeld();
	static bool backlightOn = true;
	if(hidKeysDown() && kHeld)
	{
		// Adjust LCD brightness up.
		const s16 steps = g_oafConfig.backlightSteps;
		if(kHeld == (KEY_X | KEY_DUP))
			changeBacklight(steps);

		// Adjust LCD brightness down.
		if(kHeld == (KEY_X | KEY_DDOWN))
			changeBacklight(-steps);

		// Disable backlight switching in debug builds on 2DS.
		const GfxBlight lcd = (MCU_getSystemModel() != 3 ? GFX_BLIGHT_TOP : GFX_BLIGHT_BOT);
#ifndef NDEBUG
		if(lcd != GFX_BLIGHT_BOT)
#endif
		{
			// Turn off backlight.
			if(backlightOn && kHeld == (KEY_X | KEY_DLEFT))
			{
				backlightOn = false;
				GFX_powerOffBacklights(lcd);
			}

			// Turn on backlight.
			if(!backlightOn && kHeld == (KEY_X | KEY_DRIGHT))
			{
				backlightOn = true;
				GFX_powerOnBacklights(lcd);
			}
		}
	}
}

static Result showFileBrowser(char romAndSavePath[512])
{
	Result res;
	char *lastDir = (char*)calloc(512, 1);
	if(lastDir != NULL)
	{
		do
		{
			// Get last ROM launch path.
			if((res = fsLoadPathFromFile("lastdir.txt", lastDir)) != RES_OK)
			{
				if(res == RES_FR_NO_FILE) strcpy(lastDir, "sdmc:/");
				else                      break;
			}

			// Show file browser.
			*romAndSavePath = '\0';
			if((res = browseFiles(lastDir, romAndSavePath)) == RES_FR_NO_PATH)
			{
				// Second chance in case the last dir has been deleted.
				strcpy(lastDir, "sdmc:/");
				if((res = browseFiles(lastDir, romAndSavePath)) != RES_OK) break;
			}
			else if(res != RES_OK) break;

			size_t cmpLen = strrchr(romAndSavePath, '/') - romAndSavePath;
			if((size_t)(strchr(romAndSavePath, '/') - romAndSavePath) == cmpLen) cmpLen++; // Keep the first '/'.
			if(cmpLen < 512)
			{
				if(cmpLen < strlen(lastDir) || strncmp(lastDir, romAndSavePath, cmpLen) != 0)
				{
					strncpy(lastDir, romAndSavePath, cmpLen);
					lastDir[cmpLen] = '\0';
					res = fsQuickWrite("lastdir.txt", lastDir, cmpLen + 1);
				}
			}
		} while(0);

		free(lastDir);
	}
	else res = RES_OUT_OF_MEM;

	return res;
}

static void rom2GameCfgPath(char romPath[512])
{
	// Extract the file name and change the extension.
	// For cfg2SavePath() we need to reserve 2 extra bytes/chars.
	char tmpIniFileName[256];
	safeStrcpy(tmpIniFileName, strrchr(romPath, '/') + 1, 256 - 2);
	strcpy(tmpIniFileName + strlen(tmpIniFileName) - 4, ".ini");

	// Construct the new path.
	strcpy(romPath, OAF_SAVE_DIR "/");
	strcat(romPath, tmpIniFileName);
}

static void gameCfg2SavePath(char cfgPath[512], const u8 saveSlot)
{
	if(saveSlot > 9)
	{
		*cfgPath = '\0'; // Prevent using the ROM as save file.
		return;
	}

	static char numberedExt[7] = {'.', 'X', '.', 's', 'a', 'v', '\0'};

	// Change the extension.
	// This relies on rom2GameCfgPath() to reserve 2 extra bytes/chars.
	numberedExt[1] = '0' + saveSlot;
	strcpy(cfgPath + strlen(cfgPath) - 4, (saveSlot == 0 ? ".sav" : numberedExt));
}

Result oafParseConfigEarly(void)
{
	Result res;
	do
	{
		// Create the work dir and switch to it.
		if((res = fsMakePath(OAF_WORK_DIR)) != RES_OK && res != RES_FR_EXIST) break;
		if((res = fChdir(OAF_WORK_DIR)) != RES_OK) break;

		// Create the saves folder.
		if((res = fMkdir(OAF_SAVE_DIR)) != RES_OK && res != RES_FR_EXIST) break;

		// Parse the config.
		res = parseOafConfig("config.ini", true);
	} while(0);

	return res;
}

Result oafInitAndRun(void)
{
	Result res;
	char *const filePath = (char*)calloc(512, 1);
	if(filePath != NULL)
	{
		do
		{
			// Set audio output.
			CODEC_setAudioOutput(g_oafConfig.audioOut);

			// Try to load the ROM path from autoboot.txt.
			// If this file doesn't exist show the file browser.
			if((res = fsLoadPathFromFile("autoboot.txt", filePath)) == RES_FR_NO_FILE)
			{
				if((res = showFileBrowser(filePath)) != RES_OK || *filePath == '\0') break;
				ee_puts("Loading...");
			}
			else if(res != RES_OK) break;

			//make copy of rom path
			char *const romFilePath = (char*)calloc(strlen(filePath)+1, 1);
			if(romFilePath == NULL) { res = RES_OUT_OF_MEM; break; }
			strcpy(romFilePath, filePath);

			// Load the ROM file.
			u32 romSize;
			if((res = loadGbaRom(filePath, &romSize)) != RES_OK) break;

			// Load the per-game config.
			rom2GameCfgPath(filePath);
			if((res = parseOafConfig(filePath, false)) != RES_OK && res != RES_FR_NO_FILE) break;

			// Adjust the path for the save file and get save type.
			gameCfg2SavePath(filePath, g_oafConfig.saveSlot);
			u16 saveType;
			if(g_oafConfig.saveType != 0xFF)
				saveType = g_oafConfig.saveType;
			else if(g_oafConfig.useGbaDb || g_oafConfig.saveOverride)
				saveType = getSaveType(romSize, filePath);
			else
				saveType = detectSaveType(romSize);

			patchRom(romFilePath, &romSize);
			free(romFilePath);

			// Prepare ARM9 for GBA mode + save loading.
			if((res = LGY_prepareGbaMode(g_oafConfig.directBoot, saveType, filePath)) == RES_OK)
			{
#ifdef NDEBUG
				// Force black and turn the backlight off on the bottom screen.
				// Don't turn the backlight off on 2DS (1 panel).
				GFX_setForceBlack(false, true);
				if(MCU_getSystemModel() != 3) GFX_powerOffBacklights(GFX_BLIGHT_BOT);
#endif

				// Initialize the legacy frame buffer and frame handler.
				const KHandle frameReadyEvent = createEvent(false);
				LGYFB_init(frameReadyEvent, g_oafConfig.scaler); // Setup Legacy Framebuffer.
				patchGbaGpuCmdList(g_oafConfig.scaler);
				createTask(0x800, 3, gbaGfxHandler, (void*)frameReadyEvent);
				g_frameReadyEvent = frameReadyEvent;

				// Adjust gamma table and sync LgyFb start with LCD VBlank.
				adjustGammaTableForGba();
				GFX_waitForVBlank0();
				LGY_switchMode();
			}
		} while(0);
	}
	else res = RES_OUT_OF_MEM;

	free(filePath);

	return res;
}

void oafUpdate(void)
{
	LGY_handleOverrides();
	updateBacklight();
	waitForEvent(g_frameReadyEvent);
}

void oafFinish(void)
{
	LGYFB_deinit();
	if(g_frameReadyEvent != 0)
	{
		deleteEvent(g_frameReadyEvent); // gbaGfxHandler() will automatically terminate.
		g_frameReadyEvent = 0;
	}
	LGY_deinit();
}
