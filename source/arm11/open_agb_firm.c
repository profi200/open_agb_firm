/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2024 derrek, profi200
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

#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "util.h"
#include "arm11/fast_rom_padding.h"
#include "oaf_error_codes.h"
#include "fs.h"
#include "arm11/fmt.h"
#include "arm11/console.h"
#include "arm11/drivers/mcu.h"
#include "drivers/gfx.h"
#include "arm11/drivers/hid.h"
#include "fsutil.h"
#include "arm11/filebrowser.h"
#include "arm11/config.h"
#include "arm11/save_type.h"
#include "arm11/patch.h"
#include "arm11/drivers/codec.h"
#include "drivers/lgy_common.h"
#include "arm11/oaf_video.h"
#include "arm11/drivers/lgy11.h"
#include "kernel.h"
#include "kevent.h"


static KHandle g_frameReadyEvent = 0;


static void selectColorProfile(const u8 globalProfile, const bool hasGameProfile)
{
	static const char *const profileNames[9] =
	{
		"none", "gba", "gb_micro", "gba_sp101",
		"nds", "ds_lite", "nso", "vba", "identity"
	};

	consoleClear();
	ee_printf("==Color Profile==\n"
	          "Config (global): %s\n"
	          "Config (per-game): ", profileNames[globalProfile]);
	if(hasGameProfile)
		ee_printf("%s", profileNames[g_oafConfig.colorProfile]);
	else
		ee_printf("Not set");
	ee_puts("\n"
	        "\n"
	        " None\n"
	        " GBA\n"
	        " GB Micro\n"
	        " GBA SP (AGS-101)\n"
	        " NDS\n"
	        " DS Lite\n"
	        " NSO\n"
	        " VBA\n"
	        " Identity\n"
	        "\n"
	        "Up/Down: Navigate\n"
	        "A: Select");

	u8 oldCursor = 0;
	u8 cursor = g_oafConfig.colorProfile;
	while(1)
	{
		ee_printf("\x1b[%u;H ", oldCursor + 5);
		ee_printf("\x1b[%u;H>", cursor + 5);
		oldCursor = cursor;
		GFX_flushBuffers();

		u32 kDown;
		do
		{
			GFX_waitForVBlank0();

			hidScanInput();
			if(hidGetExtraKeys(0) & (KEY_POWER_HELD | KEY_POWER)) return;
			kDown = hidKeysDown();
		} while(kDown == 0);

		if((kDown & KEY_DUP) && cursor > 0)        cursor--;
		else if((kDown & KEY_DDOWN) && cursor < 8) cursor++;
		else if(kDown & KEY_A) break;
	}

	g_oafConfig.colorProfile = cursor;
	consoleClear();
}

static void saveGameConfig(const char *const path, const u16 saveType, const u8 colorProfile,
                           const bool writeSave, const bool writeColor)
{
	static const char *const saveTypeNames[16] =
	{
		"eeprom_8k", "rom_256m_eeprom_8k", "eeprom_64k", "rom_256m_eeprom_64k",
		"flash_512k_atmel_rtc", "flash_512k_atmel", "flash_512k_sst_rtc", "flash_512k_sst",
		"flash_512k_panasonic_rtc", "flash_512k_panasonic", "flash_1m_macronix_rtc", "flash_1m_macronix",
		"flash_1m_sanyo_rtc", "flash_1m_sanyo", "sram_256k", "none"
	};
	static const char *const profileNames[9] =
	{
		"none", "gba", "gb_micro", "gba_sp101",
		"nds", "ds_lite", "nso", "vba", "identity"
	};

	char buf[128];
	unsigned len = 0;
	if(writeSave && saveType <= 15)
		len += ee_snprintf(buf + len, sizeof(buf) - len, "[game]\nsaveType=%s\n\n", saveTypeNames[saveType]);
	if(writeColor && colorProfile <= 8)
		len += ee_snprintf(buf + len, sizeof(buf) - len, "[video]\ncolorProfile=%s\n", profileNames[colorProfile]);

	if(len > 0)
		fsQuickWrite(path, buf, len);
}


static u32 fixRomPadding(const u32 romFileSize)
{
	// Pad unused ROM area with 0xFFs (trimmed ROMs).
	// Smallest retail ROM chip is 8 Mbit (1 MiB).
	u32 romSize = nextPow2(romFileSize);
	romSize = (romSize < 0x100000 ? 0x100000 : romSize);
	const uintptr_t romLoc = LGY_ROM_LOC;
	memset((void*)(romLoc + romFileSize), 0xFF, romSize - romFileSize);

	u32 mirroredSize = romSize;
	if(romSize == 0x100000) // 1 MiB.
	{
		// ROM mirroring for Classic NES Series/others with 8 Mbit ROM.
		// The ROM is mirrored exactly 4 times.
		// Thanks to endrift for discovering this.
		mirroredSize = 0x400000; // 4 MiB.
		uintptr_t mirrorLoc = romLoc + romSize;
		do
		{
			memcpy((void*)mirrorLoc, (void*)romLoc, romSize);
			mirrorLoc += romSize;
		} while(mirrorLoc < romLoc + mirroredSize);
	}

	// Fake "open bus" padding.
	if(romSize < LGY_MAX_ROM_SIZE)
		makeOpenBusPaddingFast((u32*)(romLoc + mirroredSize));

	// We don't return the mirrored size because the db hashes are over unmirrored dumps.
	return romSize;
}

static Result loadGbaRom(const char *const path, u32 *const romSizeOut)
{
	FHandle f;
	Result res = fOpen(&f, path, FA_OPEN_EXISTING | FA_READ);
	if(res == RES_OK)
	{
		u32 fileSize = fSize(f);
		if(fileSize > LGY_MAX_ROM_SIZE)
		{
			fileSize = LGY_MAX_ROM_SIZE;
			ee_puts("Warning: ROM file is too big. Expect crashes.");
		}

		u32 read;
		res = fRead(f, (void*)LGY_ROM_LOC, fileSize, &read);
		fClose(f);

		if(read == fileSize) *romSizeOut = fixRomPadding(fileSize);

	}

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

	GFX_setLcdLuminance(newVal);
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
		const GfxBl lcd = (MCU_getSystemModel() != SYS_MODEL_2DS ? GFX_BL_TOP : GFX_BL_BOT);
#ifndef NDEBUG
		if(lcd != GFX_BL_BOT)
#endif
		{
			// Turn off backlight.
			if(backlightOn && kHeld == (KEY_X | KEY_DLEFT))
			{
				backlightOn = false;
				GFX_powerOffBacklight(lcd);
			}

			// Turn on backlight.
			if(!backlightOn && kHeld == (KEY_X | KEY_DRIGHT))
			{
				backlightOn = true;
				GFX_powerOnBacklight(lcd);
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
			res = fsLoadPathFromFile("lastdir.txt", lastDir);
			if(res != RES_OK)
			{
				if(res == RES_FR_NO_FILE) strcpy(lastDir, "sdmc:/");
				else                      break;
			}

			// Show file browser.
			*romAndSavePath = '\0';
			res = browseFiles(lastDir, romAndSavePath);
			if(res == RES_FR_NO_PATH)
			{
				// Second chance in case the last dir has been deleted.
				strcpy(lastDir, "sdmc:/");
				res = browseFiles(lastDir, romAndSavePath);
				if(res != RES_OK) break;
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
	if (g_oafConfig.useSavesFolder)
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
	else
	{
		// Change the extension to .ini.
		strcpy(romPath + strlen(romPath) - 4, ".ini");
	}
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
		res = fsMakePath(OAF_WORK_DIR);
		if(res != RES_OK && res != RES_FR_EXIST) break;

		res = fChdir(OAF_WORK_DIR);
		if(res != RES_OK) break;

		// Create the saves folder.
		res = fMkdir(OAF_SAVE_DIR);
		if(res != RES_OK && res != RES_FR_EXIST) break;

		// Create screenshots folder.
		res = fMkdir(OAF_SCREENSHOT_DIR);
		if(res != RES_OK && res != RES_FR_EXIST) break;

		// Parse the config.
		res = parseOafConfig("config.ini", &g_oafConfig, true);
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
			// Try to load the ROM path from autoboot.txt.
			// If this file doesn't exist show the file browser.
			res = fsLoadPathFromFile("autoboot.txt", filePath);
			if(res == RES_FR_NO_FILE)
			{
				res = showFileBrowser(filePath);
				if(res != RES_OK || *filePath == '\0') break;
				ee_puts("Loading...");
			}
			else if(res != RES_OK) break;

			//make copy of rom path
			char *const romFilePath = (char*)calloc(strlen(filePath)+1, 1);
			if(romFilePath == NULL) { res = RES_OUT_OF_MEM; break; }
			strcpy(romFilePath, filePath);

			// Load the ROM file.
			u32 romSize;
			res = loadGbaRom(filePath, &romSize);
			if(res != RES_OK) break;

			// Save global colorProfile before per-game config may override it.
			const u8 globalColorProfile = g_oafConfig.colorProfile;

			// Load the per-game config.
			rom2GameCfgPath(filePath);
			res = parseOafConfig(filePath, &g_oafConfig, false);
			const bool hasGameConfig = (res == RES_OK);
			if(res != RES_OK && res != RES_FR_NO_FILE) break;

			// Save game config path before it's overwritten.
			char *const gameCfgPath = (char*)calloc(strlen(filePath)+1, 1);
			if(gameCfgPath == NULL) { res = RES_OUT_OF_MEM; break; }
			strcpy(gameCfgPath, filePath);

			// Adjust the path for the save file and get save type.
			gameCfg2SavePath(filePath, g_oafConfig.saveSlot);
			u16 saveType;
			if(g_oafConfig.saveType != 0xFF && !g_oafConfig.saveOverride)
				saveType = g_oafConfig.saveType;
			else if(g_oafConfig.useGbaDb || g_oafConfig.saveOverride)
				saveType = getSaveType(&g_oafConfig, romSize, filePath);
			else
				saveType = detectSaveType(romSize, g_oafConfig.defaultSave);

			patchRom(romFilePath, &romSize);
			free(romFilePath);

			// Set audio output and volume.
			CODEC_setAudioOutput(g_oafConfig.audioOut);
			CODEC_setVolumeOverride(g_oafConfig.volume);

			// Prepare ARM9 for GBA mode + save loading.
			res = LGY_prepareGbaMode(g_oafConfig.directBoot, saveType, filePath);
			if(res == RES_OK)
			{
				if(g_oafConfig.colorOverride)
					selectColorProfile(globalColorProfile,
					                   hasGameConfig && g_oafConfig.colorProfile != globalColorProfile);

				// Save per-game overrides if either menu was shown.
				if(g_oafConfig.saveOverride || g_oafConfig.colorOverride)
					saveGameConfig(gameCfgPath, saveType, g_oafConfig.colorProfile,
					               g_oafConfig.saveOverride, g_oafConfig.colorOverride);

				// Initialize video output (frame capture, post processing ect.).
				g_frameReadyEvent = OAF_videoInit();

				// Setup button overrides.
				const u32 *const maps = g_oafConfig.buttonMaps;
				u16 overrides = 0;
				for(unsigned i = 0; i < 10; i++)
					if(maps[i] != 0) overrides |= 1u<<i;
				LGY11_selectInput(overrides);

				// Sync LgyCap start with LCD VBlank.
				GFX_waitForVBlank0();
				LGY11_switchMode();
			}
			free(gameCfgPath);
		} while(0);
	}
	else res = RES_OUT_OF_MEM;

	free(filePath);

	return res;
}


void oafUpdate(void)
{
	const u32 *const maps = g_oafConfig.buttonMaps;
	const u32 kHeld = hidKeysHeld();
	u16 pressed = 0;
	for(unsigned i = 0; i < 10; i++)
	{
		if((kHeld & maps[i]) != 0)
			pressed |= 1u<<i;
	}
	LGY11_setInputState(pressed);

	CODEC_runHeadphoneDetection();
	updateBacklight();
	waitForEvent(g_frameReadyEvent);
	clearEvent(g_frameReadyEvent);
}

void oafFinish(void)
{
	// frameReadyEvent deleted by this function.
	OAF_videoExit();
	g_frameReadyEvent = 0;
	LGY11_deinit();
}