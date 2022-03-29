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

#include <assert.h>
#include <string.h>
#include "types.h"
#include "drivers/lgy.h"
#include "error_codes.h"
#include "mem_map.h"
#include "mmio.h"
#include "drivers/cache.h"
#include "arm9/arm7_stub.h"
#include "fsutil.h"
#include "arm9/debug.h"
#include "drivers/sha.h"
#include "util.h"


#define LGY_REGS_BASE  (IO_MEM_ARM9_ONLY + 0x18000)

typedef struct
{
	vu16 mode;
	u8 _0x2[0x7e];
	vu32 a7_vector[8];     // ARM7 vector override 32 bytes. Writable at runtime.
	u8 _0xa0[0x60];
	vu16 gba_save_type;
	u8 _0x102[2];
	vu8 gba_save_map;      // Remaps the GBA save to ARM9 if set to 1.
	u8 _0x105[3];
	vu16 gba_rtc_cnt;
	u8 _0x10a[6];
	vu32 gba_rtc_bcd_date;
	vu32 gba_rtc_bcd_time;
	vu32 gba_rtc_toffset;  // Writing bit 7 completely hangs all(?) GBA hardware.
	vu32 gba_rtc_doffset;
	vu32 gba_save_timing[4];
} Lgy;
static_assert(offsetof(Lgy, gba_save_timing) == 0x120, "Error: Member gba_save_timing of Lgy is not at offset 0x120!");

ALWAYS_INLINE Lgy* getLgyRegs(void)
{
	return (Lgy*)LGY_REGS_BASE;
}


static u32 g_saveSize = 0;
static u32 g_saveHash[8] = {0};
static char g_savePath[512] = {0};



static void setupBiosOverlay(bool directBoot)
{
	iomemcpy(getLgyRegs()->a7_vector, (u32*)_a7_overlay_stub, (u32)_a7_overlay_stub_size);
	//static const u32 biosVectors[8] = {0xEA000018, 0xEA000004, 0xEA00004C, 0xEA000002,
	//                                   0xEA000001, 0xEA000000, 0xEA000042, 0xE59FD1A0};
	//iomemcpy(getLgyRegs()->a7_vector, biosVectors, 32);

	iomemcpy((u32*)ARM7_STUB_LOC9, (u32*)_a7_stub_start, (u32)_a7_stub_size);
	if(!directBoot) *((vu8*)_a7_stub9_swi) = 0x26; // Patch swi 0x01 (RegisterRamReset) to swi 0x26 (HardReset).
	flushDCacheRange((void*)ARM7_STUB_LOC9, (u32)_a7_stub_size);
}

static u32 setupSaveType(u16 saveType)
{
	Lgy *const lgy = getLgyRegs();
	lgy->gba_save_type = saveType;

	static const u8 saveSizeShiftLut[16] = {9, 9, 13, 13, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 15, 0};
	const u32 saveSize = (1u<<saveSizeShiftLut[saveType & SAVE_TYPE_MASK]) & ~1u;
	g_saveSize = saveSize;

	// Flash chip erase, flash sector erase, flash program, EEPROM write.
	static const u32 saveTm512k4k[4] = {0x27C886, 0x8CE35, 0x184, 0x31170}; // Timing 512k/4k.
	static const u32 saveTm1m64k[4] = {0x17D43E, 0x26206, 0x86, 0x2DD13};   // Timing 1m/64k.
	const u32 *saveTm = saveTm512k4k;
	if(saveType == SAVE_TYPE_EEPROM_64k ||
	   saveType == SAVE_TYPE_EEPROM_64k_2 ||
	   (saveType >= SAVE_TYPE_FLASH_1m_MRX_RTC && saveType <= SAVE_TYPE_FLASH_1m_SNO))
	{
		saveTm = saveTm1m64k;
	}
	iomemcpy(lgy->gba_save_timing, saveTm, 16);

	return saveSize;
}

Result LGY_prepareGbaMode(bool directBoot, u16 saveType, const char *const savePath)
{
	getLgyRegs()->mode = LGY_MODE_AGB;

	setupBiosOverlay(directBoot);
	const u32 saveSize = setupSaveType(saveType);
	strncpy_s(g_savePath, savePath, 511, 512);

	Result res = RES_OK;
	if(saveSize != 0)
	{
		res = fsQuickRead(savePath, (void*)SAVE_LOC, saveSize);
		if(res == RES_FR_NO_FILE)
		{
			// Ignore a missing save file and fill the save with 0xFFs.
			res = RES_OK;
			iomemset((u32*)SAVE_LOC, 0xFFFFFFFFu, saveSize);
		}

		// Hash the savegame so it's only backed up when changed.
		// Then flush the savegame region.
		sha((u32*)SAVE_LOC, saveSize, g_saveHash, SHA_IN_BIG | SHA_256_MODE, SHA_OUT_BIG);
		flushDCacheRange((void*)SAVE_LOC, saveSize);
	}

	return res;
}

Result LGY_setGbaRtc(const GbaRtc rtc)
{
	// Set base time and date.
	Lgy *const lgy = getLgyRegs();
	lgy->gba_rtc_bcd_time = rtc.time;
	lgy->gba_rtc_bcd_date = rtc.date;

	//while(lgy->gba_rtc_cnt & LGY_RTC_CNT_BUSY);
	//lgy->gba_rtc_cnt = 0; // Legacy P9 does this. Useless?
	lgy->gba_rtc_toffset = 1u<<15; // Time offset 0 and 24h format.
	lgy->gba_rtc_doffset = 0;      // Date offset 0.
	lgy->gba_rtc_cnt = LGY_RTC_CNT_WR;
	while(lgy->gba_rtc_cnt & LGY_RTC_CNT_BUSY);

	if(lgy->gba_rtc_cnt & LGY_RTC_CNT_WR_ERR) return RES_GBA_RTC_ERR;
	else                                      return RES_OK;
}

Result LGY_getGbaRtc(GbaRtc *const out)
{
	Lgy *const lgy = getLgyRegs();
	//while(lgy->gba_rtc_cnt & LGY_RTC_CNT_BUSY);
	//lgy->gba_rtc_cnt = 0; // Legacy P9 does this. Useless?
	lgy->gba_rtc_cnt = LGY_RTC_CNT_RD;
	while(lgy->gba_rtc_cnt & LGY_RTC_CNT_BUSY);

	if((lgy->gba_rtc_cnt & LGY_RTC_CNT_WR_ERR) == 0u)
	{
		out->time = lgy->gba_rtc_bcd_time;
		out->date = lgy->gba_rtc_bcd_date;

		return RES_OK;
	}

	return RES_GBA_RTC_ERR;
}

/*#include "arm9/hardware/timer.h"
void LGY_gbaReset(void)
{
	// Hook the SVC vector for 17 ms and hope it jumps to the
	// BIOS reset function skipping the "POSTFLG" check.
	Lgy *const lgy = getLgyRegs();
	lgy->a7_vector[2] = 0xEA00001F; // SVC_vector: b 0x8C @ resetHandler
	TIMER_sleepMs(17); // Wait 17 ms.
	// Restore vector.
	lgy->a7_vector[2] = 0xEA00004C; // SVC_vector: b 0x140 @ svcHandler
}*/

Result LGY_backupGbaSave(void)
{
	Result res = RES_OK;
	const u32 saveSize = g_saveSize;
	if(saveSize != 0)
	{
		// Map savegame region to ARM9 side.
		Lgy *const lgy = getLgyRegs();
		lgy->gba_save_map = LGY_SAVE_MAP_9;

		// Make sure there is no stale data in the cache.
		invalidateDCacheRange((void*)SAVE_LOC, saveSize);

		u32 newHash[8];
		sha((u32*)SAVE_LOC, saveSize, newHash, SHA_IN_BIG | SHA_256_MODE, SHA_OUT_BIG);
		if(memcmp(g_saveHash, newHash, 32) != 0) // Backup save if changed.
		{
			// Update hash.
			memcpy(g_saveHash, newHash, 32);

			res = fsQuickWrite(g_savePath, (void*)SAVE_LOC, saveSize);
		}

		// Map savegame region back to ARM7 side.
		lgy->gba_save_map = LGY_SAVE_MAP_7;
	}

	return res;
}
