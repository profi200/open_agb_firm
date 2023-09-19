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
#include "arm11/config.h"
#include "util.h"
#include "arm11/drivers/lgy11.h"
#include "drivers/lgy_common.h"
#include "arm_intrinsic.h"
#include "oaf_error_codes.h"
#include "fs.h"
#include "fsutil.h"
#include "arm11/fmt.h"
#include "arm11/drivers/lcd.h"
#include "arm11/drivers/lgyfb.h"
#include "drivers/gfx.h"
#include "arm11/drivers/mcu.h"
#include "kernel.h"
#include "kevent.h"
#include "arm11/gpu_cmd_lists.h"
#include "arm11/drivers/hid.h"
#include "arm11/filebrowser.h"
#include "arm11/drivers/codec.h"
#include "arm11/save_type.h"
#include "arm11/patch.h"


#define OAF_WORK_DIR  "sdmc:/3ds/open_agb_firm"
#define OAF_SAVE_DIR  "saves" // Relative to work dir.



// Default config.
// Note: Keep this synchronized with DEFAULT_CONFIG in config.c.
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
	127,   // Control via volume slider.

	// [input]
	{      // buttonMaps
		0, // A
		0, // B
		0, // Select
		0, // Start
		0, // Right
		0, // Left
		0, // Up
		0, // Down
		0, // R
		0  // L
	},

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
	memset((void*)(LGY_ROM_LOC + romFileSize), 0xFFFFFFFFu, romSize - romFileSize);
	if(romSize > 0x100000u) // >1 MiB.
	{
		// Fake "open bus" padding.
		u32 padding = (LGY_ROM_LOC + romSize) / 2;
		padding = __pkhbt(padding, padding + 1, 16); // Copy lower half + 1 to upper half.
		for(uintptr_t i = LGY_ROM_LOC + romSize; i < LGY_ROM_LOC + LGY_MAX_ROM_SIZE; i += 4)
		{
			*(u32*)i = padding;
			padding = __uadd16(padding, 0x00020002u); // Unsigned parallel halfword-wise addition.
		}
	}
	else
	{

		// ROM mirroring (Classic NES Series/possibly others with 8 Mbit ROM).
		// Mirror ROM across the entire 32 MiB area.
		for(uintptr_t i = LGY_ROM_LOC + romSize; i < LGY_ROM_LOC + LGY_MAX_ROM_SIZE; i += romSize)
		{
			//memcpy((void*)i, (void*)(i - romSize), romSize); // 0x23A15DD
			memcpy((void*)i, (void*)LGY_ROM_LOC, romSize); // 0x237109B
		}
	}

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
		res = fRead(f, (u8*)LGY_ROM_LOC, fileSize, &read);
		fClose(f);

		if(read == fileSize) *romSizeOut = fixRomPadding(fileSize);

	}

	return res;
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

	GX_displayTransfer((u32*)0x18200000, 240u<<16 | 512, (u32*)0x18400040, 240u<<16 | 512, 1u<<12 | 1u<<8);
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
		// 240x160 no scaling:    184 µs
		// 240x160 bilinear x1.5: 408 µs
		// 360x240 no scaling:    437 µs
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
		GX_displayTransfer((u32*)GPU_RENDER_BUF_ADDR, 400u<<16 | 240, GFX_getFramebuffer(SCREEN_TOP), 400u<<16 | 240, 1u<<12 | 1u<<8);
		GFX_waitForPPF();
		GFX_swapFramebufs();

		// Trigger only if both are held and at least one is detected as newly pressed down.
		if(hidKeysHeld() == (KEY_Y | KEY_SELECT) && hidKeysDown() != 0)
			dumpFrameTex();
	}

	taskExit();
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
		res = fsMakePath(OAF_WORK_DIR);
		if(res != RES_OK && res != RES_FR_EXIST) break;

		res = fChdir(OAF_WORK_DIR);
		if(res != RES_OK) break;

		// Create the saves folder.
		res = fMkdir(OAF_SAVE_DIR);
		if(res != RES_OK && res != RES_FR_EXIST) break;

		// Parse the config.
		res = parseOafConfig("config.ini", &g_oafConfig, true);
	} while(0);

	return res;
}

KHandle setupFrameCapture(const u8 scaler)
{
	const bool is240x160 = scaler < 2;
	static const s16 matrix[12 * 8] =
	{
		// Vertical.
		      0,       0,       0,       0,       0,       0,       0,       0,
		      0,       0,       0,       0,       0,       0,       0,       0,
		      0,  0x24B0,  0x4000,       0,  0x24B0,  0x4000,       0,       0,
		 0x4000,  0x2000,       0,  0x4000,  0x2000,       0,       0,       0,
		      0,  -0x4B0,       0,       0,  -0x4B0,       0,       0,       0,
		      0,       0,       0,       0,       0,       0,       0,       0,

		// Horizontal.
		      0,       0,       0,       0,       0,       0,       0,       0,
		      0,       0,       0,       0,       0,       0,       0,       0,
		      0,       0,  0x24B0,       0,       0,  0x24B0,       0,       0,
		 0x4000,  0x4000,  0x2000,  0x4000,  0x4000,  0x2000,       0,       0,
		      0,       0,  -0x4B0,       0,       0,  -0x4B0,       0,       0,
		      0,       0,       0,       0,       0,       0,       0,       0
	};

	ScalerCfg gbaCfg;
	gbaCfg.w     = (is240x160 ? 240 : 360);
	gbaCfg.h     = (is240x160 ? 160 : 240);
	gbaCfg.vLen  = 6;
	gbaCfg.vPatt = 0b00011011;
	memcpy(gbaCfg.vMatrix, matrix, 6 * 8 * 2);
	gbaCfg.hLen  = 6;
	gbaCfg.hPatt = (is240x160 ? 0b00111111 : 0b00011011);

	if(is240x160)
	{
		memset(gbaCfg.hMatrix, 0, 6 * 8 * 2);
		s16 *const identityRow = &gbaCfg.hMatrix[3 * 8];
		for(unsigned i = 0; i < 6; i++)
		{
			// Set identity entries.
			identityRow[i] = 0x4000;
		}
	}
	else
	{
		memcpy(gbaCfg.hMatrix, &matrix[6 * 8], 6 * 8 * 2);
	}

	return LGYFB_init(&gbaCfg);
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

			// Load the per-game config.
			rom2GameCfgPath(filePath);
			res = parseOafConfig(filePath, &g_oafConfig, false);
			if(res != RES_OK && res != RES_FR_NO_FILE) break;

			// Adjust the path for the save file and get save type.
			gameCfg2SavePath(filePath, g_oafConfig.saveSlot);
			u16 saveType;
			if(g_oafConfig.saveType != 0xFF)
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
#ifdef NDEBUG
				// Force black and turn the backlight off on the bottom screen.
				// Don't turn the backlight off on 2DS (1 panel).
				GFX_setForceBlack(false, true);
				if(MCU_getSystemModel() != 3) GFX_powerOffBacklights(GFX_BLIGHT_BOT);
#endif

				// Initialize frame capture and frame handler.
				const KHandle frameReadyEvent = setupFrameCapture(g_oafConfig.scaler);
				patchGbaGpuCmdList(g_oafConfig.scaler);
				createTask(0x800, 3, gbaGfxHandler, (void*)frameReadyEvent);
				g_frameReadyEvent = frameReadyEvent;

				// Adjust gamma table and setup button overrides.
				adjustGammaTableForGba();
				const u32 *const maps = g_oafConfig.buttonMaps;
				u16 overrides = 0;
				for(unsigned i = 0; i < 10; i++)
					if(maps[i] != 0) overrides |= 1u<<i;
				LGY11_selectInput(overrides);

				// Load border if any exists.
				if(g_oafConfig.scaler == 0) // No borders for scaled modes.
				{
					// Abuse currently invisible frame buffer as temporary buffer.
					void *const borderBuf = GFX_getFramebuffer(SCREEN_TOP);
					if(fsQuickRead("border.bgr", borderBuf, 400 * 240 * 3) == RES_OK)
					{
						// Copy border in swizzled form to GPU render buffer.
						GX_displayTransfer(borderBuf, 400u<<16 | 240, (u32*)GPU_RENDER_BUF_ADDR, 400u<<16 | 240, 1u<<12 | 1u<<8 | 1u<<1);
						GFX_waitForPPF();
					}
				}

				// Sync LgyFb start with LCD VBlank.
				GFX_waitForVBlank0();
				LGY11_switchMode();
			}
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
}

void oafFinish(void)
{
	// frameReadyEvent deleted by this function.
	// gbaGfxHandler() will automatically terminate.
	LGYFB_deinit();
	g_frameReadyEvent = 0;
	LGY11_deinit();
}
