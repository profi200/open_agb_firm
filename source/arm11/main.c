/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
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
#include "arm11/hardware/hid.h"
#include "arm11/hardware/codec.h"
#include "hardware/lgy.h"
#include "arm11/hardware/lgyfb.h"
#include "arm11/console.h"
#include "arm11/fmt.h"
#include "arm11/hardware/mcu.h"
#include "arm11/power.h"
#include "hardware/gfx.h"
#include "fs.h"
#include "arm11/filebrowser.h"
#include "arm.h"
#include "arm11/hardware/lcd.h"
#include "arm11/gpu_cmd_lists.h"
#include "kernel.h"
#include "kevent.h"



static Result loadGbaRom(const char *const path, u32 *const rsOut)
{
	Result res;
	FHandle f;
	if((res = fOpen(&f, path, FA_OPEN_EXISTING | FA_READ)) == RES_OK)
	{
		u32 romSize;
		if((romSize = fSize(f)) <= MAX_ROM_SIZE)
		{
			u8 *ptr = (u8*)ROM_LOC;
			u32 read;
			while((res = fRead(f, ptr, 0x100000u, &read)) == RES_OK && read == 0x100000u)
				ptr += 0x100000u;

			if(res == RES_OK)
			{
				*rsOut = romSize;
				// Pad ROM area with "open bus" value.
				memset((void*)(ROM_LOC + romSize), 0xFFFFFFFFu, MAX_ROM_SIZE - romSize);
			}
		}
		else res = RES_ROM_TOO_BIG;

		fClose(f);
	}

	return res;
}

static u16 checkSaveOverride(u32 gameCode)
{
	if((gameCode & 0xFFu) == 'F') // Classic NES Series.
	{
		return SAVE_TYPE_EEPROM_8k;
	}

	static const struct
	{
		alignas(4) char gameCode[4];
		u16 saveType;
	} overrideLut[] =
	{
		{"\0\0\0\0", SAVE_TYPE_SRAM_256k},  // Homebrew. TODO: Set WAITCNT to 0x4014?
		{"GMB\0",    SAVE_TYPE_SRAM_256k},  // Goomba Color (Homebrew).
		{"AA2\0",    SAVE_TYPE_EEPROM_64k}, // Super Mario Advance 2.
		{"A3A\0",    SAVE_TYPE_EEPROM_64k}, // Super Mario Advance 3.
		{"AZL\0",    SAVE_TYPE_EEPROM_64k}, // Legend of Zelda, The - A Link to the Past & Four Swords.
	};

	for(u32 i = 0; i < sizeof(overrideLut) / sizeof(*overrideLut); i++)
	{
		// Compare Game Code without region.
		if((gameCode & 0xFFFFFFu) == *((u32*)overrideLut[i].gameCode))
		{
			return overrideLut[i].saveType;
		}
	}

	return 0xFF;
}

// Code based on: https://github.com/Gericom/GBARunner2/blob/master/arm9/source/save/Save.vram.cpp
static u16 tryDetectSaveType(u32 romSize)
{
	const u32 *romPtr = (u32*)ROM_LOC;
	u16 saveType;
	if((saveType = checkSaveOverride(romPtr[0xAC / 4])) != 0xFF)
	{
		debug_printf("Game Code in override list. Using save type %" PRIu16 ".\n", saveType);
		return saveType;
	}

	romPtr += 0xE4 / 4; // Skip headers.
	saveType = SAVE_TYPE_NONE;
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
				{"EEPROM_V111", SAVE_TYPE_EEPROM_8k},  // Actually EEPROM 4k.
				{"EEPROM_V120", SAVE_TYPE_EEPROM_8k},  // Confirmed.
				{"EEPROM_V121", SAVE_TYPE_EEPROM_64k}, // Confirmed.
				{"EEPROM_V122", SAVE_TYPE_EEPROM_8k},  // Confirmed. Except Super Mario Advance 2/3.
				{"EEPROM_V124", SAVE_TYPE_EEPROM_64k}, // Confirmed.
				{"EEPROM_V125", SAVE_TYPE_EEPROM_8k},  // Confirmed.
				{"EEPROM_V126", SAVE_TYPE_EEPROM_8k},  // Confirmed.

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
					saveType = tmpSaveType;
					debug_printf("Detected SDK save type '%s'.\n", str);
					goto saveTypeFound;
				}
			}
		}
	}

saveTypeFound:

	return saveType;
}

/*static u16 getSaveTypeFromTable(void)
{
	const u32 gameCode = *(u32*)(ROM_LOC + 0xAC) & ~0xFF000000u;

	u16 saveType = SAVE_TYPE_NONE;
	for(u32 i = 0; i < sizeof(saveTypeLut) / sizeof(*saveTypeLut); i++)
	{
		// Save type in last byte.
		const u32 entry = *((u32*)&saveTypeLut[i]);
		if((entry & ~0xFF000000u) == gameCode)
		{
			saveType = entry>>24;
			break;
		}
	}

	debug_printf("Using save type 0x%" PRIX16 ".\n", saveType);

	return saveType;
}*/

static void adjustGammaTableForGba(void)
{
	const double inGamma = 4.0;
	const double outGamma = 2.2;
	//const double contrast = .74851331406341291833644689906823; // GBA
	//const double brightness = .25148668593658708166355310093177; // GBA
	const double contrast = 1.0; // No-op
	const double brightness = 0.0; // No-op
	//REG_LCD_PDC0_GTBL_IDX = 0;
	for(u32 i = 0; i < 256; i++)
	{
		// Credits for this algo go to Extrems.
		// Originally from Game Boy Interface Standard Edition for the Game Cube.
		//const u32 x = (i & ~7u) | i>>5;
		u32 res = pow(pow(contrast, inGamma) * pow((double)i / 255.0f + brightness / contrast, inGamma),
		              1.0 / outGamma) * 255.0f;
		if(res > 255) res = 255;

		// Same adjustment for red/green/blue.
		REG_LCD_PDC0_GTBL_FIFO = res<<16 | res<<8 | res;
	}
}

#ifndef NDEBUG
#include "fsutil.h"
static void dbgDumpFrame(void)
{
	/*GX_displayTransfer((u32*)0x18200000, 160u<<16 | 256u, (u32*)0x18400000, 160u<<16 | 256u, 1u<<12 | 1u<<8);
	GFX_waitForEvent(GFX_EVENT_PPF, false);
	fsQuickWrite((void*)0x18400000, "sdmc:/lgyfb_dbg_frame.bgr", 256 * 160 * 3);*/
	GX_displayTransfer((u32*)0x18200000, 240u<<16 | 512u, (u32*)0x18400000, 240u<<16 | 512u, 1u<<12 | 1u<<8);
	GFX_waitForEvent(GFX_EVENT_PPF, false);
	fsQuickWrite((void*)0x18400000, "sdmc:/lgyfb_dbg_frame.bgr", 512 * 240 * 3);
}

void debugTests(void)
{
	const u32 kDown = hidKeysDown();

	// Print GBA RTC date/time.
	if(kDown & KEY_X)
	{
		GbaRtc rtc; LGY_getGbaRtc(&rtc);
		ee_printf("RTC: %02X.%02X.%04X %02X:%02X:%02X\n", rtc.d, rtc.mon, rtc.y + 0x2000u, rtc.h, rtc.min, rtc.s);

		// Trigger Game Boy Player enhancements.
		// Needs to be done on the Game Boy Player logo screen.
		// 2 frames nothing pressed and 1 frame all D-Pad buttons pressed.
		/*REG_LGY_PAD_SEL = 0x1FFF; // Override all buttons.
		static u8 gbp = 2;
		if(gbp > 0)
		{
			REG_LGY_PAD_VAL = 0x1FFF; // Force all buttons not pressed.
			gbp--;
		}
		else
		{
			REG_LGY_PAD_VAL = 0x1F0F; // All D-Pad buttons pressed.
			gbp = 2;
		}*/
	}
	//else REG_LGY_PAD_SEL = 0; // Stop overriding buttons.
	if(kDown & KEY_Y) dbgDumpFrame();
}
#endif

static void gbaGfxHandler(void *args)
{
	const KEvent event = (KEvent)args;

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
		GFX_waitForPPF();
		GFX_swapFramebufs();
	}

	taskExit();
}

int main(void)
{
	GFX_init(GFX_BGR8, GFX_RGB565);
	GFX_setBrightness(DEFAULT_BRIGHTNESS, DEFAULT_BRIGHTNESS);
	consoleInit(SCREEN_BOT, NULL);
	//CODEC_init();

	// Show file browser.
	Result res;
	char *romPath = (char*)malloc(512);
	*romPath = '\0';
	if((res = fMount(FS_DRIVE_SDMC)) != RES_OK || (res = browseFiles("sdmc:/", romPath)) != RES_OK || *romPath == '\0')
		goto end;

	ee_puts("Reading ROM and save...");
	u32 romSize;
	res = loadGbaRom(romPath, &romSize);
	if(res != RES_OK) goto end;

	// Detect save type and adjust path for the save file.
	const u16 saveType = tryDetectSaveType(romSize);
	const u32 romPathLen = strlen(romPath);
	strcpy(romPath + romPathLen - 4, ".sav");

	// Prepare ARM9 for GBA mode + settings and save loading.
	if((res = LGY_prepareGbaMode(false, saveType, romPath)) == RES_OK)
	{
#ifdef NDEBUG
		GFX_setForceBlack(false, true);
		// Don't turn the backlight off on 2DS.
		if(MCU_getSystemModel() != 3) GFX_powerOffBacklights(GFX_BLIGHT_BOT);
#endif

		const KEvent frameReadyEvent = createEvent(false);
		LGYFB_init(frameReadyEvent); // Setup Legacy Framebuffer.
		/*const KTask gfxTask =*/ createTask(0x800, 3, gbaGfxHandler, frameReadyEvent);

		// Adjust gamma table and sync LgyFb start with LCD VBlank.
		adjustGammaTableForGba();
		GFX_waitForVBlank0();
		LGY_switchMode();

		do
		{
			hidScanInput();
			if(hidGetExtraKeys(0) & (KEY_POWER_HELD | KEY_POWER)) break;

			LGY_handleOverrides();

			waitForEvent(frameReadyEvent);
		} while(1);

		LGYFB_deinit();
		deleteEvent(frameReadyEvent); // gfxTask() will automatically terminate.
	}

end:
	free(romPath);
	if(res != RES_OK) printErrorWaitInput(res, 0);

	LGY_deinit();
	fUnmount(FS_DRIVE_SDMC);
	CODEC_deinit();
	GFX_deinit();
	power_off();

	return 0;
}
