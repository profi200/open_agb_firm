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
#include "hardware/lgy.h"
#include "error_codes.h"
#include "mem_map.h"
#include "mmio.h"
#include "arm9/hardware/ndma.h"
#include "arm9/hardware/timer.h"
#include "arm9/arm7_stub.h"
#include "fsutil.h"
#include "arm9/debug.h"
#include "arm9/hardware/crypto.h"
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
	vu32 gba_rtc_hex_time; // Writing bit 7 completely hangs all(?) GBA hardware.
	vu32 gba_rtc_hex_date;
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

static const u32 biosVectors[8] = {0xEA000018, 0xEA000004, 0xEA00004C, 0xEA000002,
	                               0xEA000001, 0xEA000000, 0xEA000042, 0xE59FD1A0};

static bool bIsSleep = false;

static void setupBiosOverlay(bool biosIntro)
{
	iomemcpy(getLgyRegs()->a7_vector, biosVectors, 0x20);

	NDMA_copy((u32*)ARM7_STUB_LOC9, (u32*)_a7_stub_start, (u32)_a7_stub_size);
	//if(biosIntro) *((vu8*)_a7_stub9_swi) = 0x26; // Patch swi 0x01 (RegisterRamReset) to swi 0x26 (HardReset).
}

static u32 setupSaveType(u16 saveType)
{
	Lgy *const lgy = getLgyRegs();
	lgy->gba_save_type = saveType;
	// The last shift in the table is technically undefined behavior (C standard)
	// but on ARM this will always result in 0.
	// https://developer.arm.com/documentation/dui0489/h/arm-and-thumb-instructions/shift-operations
	static const u8 saveSizeShiftLut[16] = {9, 9, 13, 13, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 15, 32};
	const u32 saveSize = 1u<<saveSizeShiftLut[saveType & 0xFu];
	g_saveSize = saveSize;

	// Flash chip erase, flash sector erase, flash program, EEPROM write.
	static const u32 saveTm512k4k[4] = {0x27C886, 0x8CE35, 0x184, 0x31170}; // Timing 512k/4k.
	static const u32 saveTm1m64k[4] = {0x17D43E, 0x26206, 0x86, 0x2DD13};   // Timing 1m/64k.
	const u32 *saveTm;
	if(saveType < SAVE_TYPE_EEPROM_64k ||
	   (saveType > SAVE_TYPE_EEPROM_64k_2 && saveType < SAVE_TYPE_FLASH_1m_MRX_RTC) ||
	   saveType == SAVE_TYPE_SRAM_256k)
	{
		saveTm = saveTm512k4k;
	}
	else saveTm = saveTm1m64k; // Don't care about save type none.
	iomemcpy(lgy->gba_save_timing, saveTm, 16);

	return saveSize;
}

Result LGY_prepareGbaMode(bool biosIntro, u16 saveType, const char *const savePath)
{
	getLgyRegs()->mode = LGY_MODE_AGB;

	setupBiosOverlay(biosIntro);
	const u32 saveSize = setupSaveType(saveType);
	strncpy_s(g_savePath, savePath, 511, 512);

	Result res = RES_OK;
	if(saveSize != 0)
	{
		res = fsQuickRead(savePath, (void*)SAVE_LOC, MAX_SAVE_SIZE);
		if(res == RES_FR_NO_FILE)
		{
			res = RES_OK; // Ignore a missing save file.
			NDMA_fill((u32*)SAVE_LOC, 0xFFFFFFFFu, saveSize);
		}

		// Hash the savegame so it's only backed up when changed.
		sha((u32*)SAVE_LOC, saveSize, g_saveHash, SHA_IN_BIG | SHA_256_MODE, SHA_OUT_BIG);
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
	lgy->gba_rtc_hex_time = 1u<<15; // Time offset 0 and 24h format.
	lgy->gba_rtc_hex_date = 0;      // Date offset 0.
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
		// Enable savegame mem region.
		Lgy *const lgy = getLgyRegs();
		lgy->gba_save_map = LGY_SAVE_MAP_9;

		u32 newHash[8];
		sha((u32*)SAVE_LOC, saveSize, newHash, SHA_IN_BIG | SHA_256_MODE, SHA_OUT_BIG);
		if(memcmp(g_saveHash, newHash, 32) != 0) // Backup save if changed.
		{
			// Update hash.
			memcpy(g_saveHash, newHash, 32);

			res = fsQuickWrite(g_savePath, (void*)SAVE_LOC, saveSize);
		}

		// Disable savegame mem region.
		lgy->gba_save_map = LGY_SAVE_MAP_7;
	}

	return res;
}

static inline void LGY_delay(u32 count)
{
	u32 i;
	for (i = 0; i < count; i++)
		__asm__ volatile ("");
}

static void LGY_runInstruction(u32 instruction)
{
	const u32 b_loop = 0xEAFFFFFE;
	const u32 b_minus8 = 0xEAFFFFFC;
	const u32 arm_nop = 0;

	// Trap PC at 0008
	getLgyRegs()->a7_vector[2] = b_loop;
	getLgyRegs()->a7_vector[1] = arm_nop;
	getLgyRegs()->a7_vector[0] = arm_nop;
	LGY_delay(1000);

	// Set a trap at 0000, with the instruction we want to run at 0004,
	// then untrap SWI loop. It should branch back to 0000 and wait.
	getLgyRegs()->a7_vector[0] = b_loop;
	getLgyRegs()->a7_vector[1] = instruction;
	getLgyRegs()->a7_vector[2] = b_minus8;
	LGY_delay(1000);

	// Set SWI trap at 0008, and allow instruction to be executed.
	// PC should end at 0008
	getLgyRegs()->a7_vector[2] = b_loop;
	getLgyRegs()->a7_vector[0] = arm_nop;
	LGY_delay(1000);

	// Unset instruction
	getLgyRegs()->a7_vector[1] = arm_nop;
	LGY_delay(1000);
}

static void LGY_mov(u8 reg, u32 val)
{
	u16 val_lo = val & 0xFFFF;
	u16 val_hi = val >> 16;

	LGY_runInstruction(0xE3000000 | (val_lo & 0xFFF) | ((val_lo & 0xF000) << 4) | (reg << 12)); // movw rN, #hival
	LGY_runInstruction(0xE3400000 | (val_hi & 0xFFF) | ((val_hi & 0xF000) << 4) | (reg << 12)); // movt rN, #loval
}

void LGY_sleepGba(void)
{
	if (bIsSleep)
		return;

    // Take over ARM7 by replacing all vectors with infinite loops.
    // The hope is that the game will call an SWI or have an interrupt occur, where
    // we can use the overlay to feed the ARM7 instructions bit-by-bit.
    //
    // Note: Sometimes this can fail, if called mid-interrupt.
	// The fallback is just not sleeping or executing any instructions, for now.
	//
	iomemcpy(getLgyRegs()->a7_vector, (u32*)_a7_overlay_stub_capture, (u32)_a7_overlay_stub_capture_size);

	// Wait for a frame to complete
	TIMER_sleepMs(17);

	// Stash r11 and r12 as our working registers
	LGY_runInstruction(0); // nop
	LGY_runInstruction(0xE92D5800); // stmfd sp!, {r11, r12, lr}

#if 0
	LGY_mov(11, 0x08000000);
	LGY_mov(12, 0xFFFFFFFF);

	for (int i = 0; i < 0x200 / 4; i++)
	{
		LGY_runInstruction(0xe28bb004); // add r11, r11, #0x4
	}

	for (int i = 0; i < 0x1000 / 4; i++)
	{
		LGY_runInstruction(0xe58bc000); // str r12, [r11, #0x0]
		LGY_runInstruction(0xe28bb004); // add r11, r11, #0x4
	}
#endif

	// swiStop equivalent
	LGY_mov(11, 0x04000000);
	LGY_mov(12, 0x80);
	LGY_runInstruction(0xe5cbc301); // strb r12, [r11, #0x301]

	LGY_runInstruction(0); // nop

	// Wait for sleep entry
	TIMER_sleepMs(17);

	bIsSleep = true;
}

void LGY_wakeGba(void)
{
	if (!bIsSleep)
		return;

	// Wait for a frame to complete
	TIMER_sleepMs(17);

#if 0
	// Enable savegame mem region.
	Lgy *const lgy = getLgyRegs();
	lgy->gba_save_map = LGY_SAVE_MAP_9;

	dumpMem((void*)0x8080000u, 0x80000, "sdmc:/test-dump.bin");

	// Disable savegame mem region.
	lgy->gba_save_map = LGY_SAVE_MAP_7;
#endif

	// Remainder of forced sleep routine
	LGY_runInstruction(0); // nop
	LGY_runInstruction(0xE8BD5800); // ldmfd sp!, {r11, r12, lr}
	LGY_runInstruction(0); // nop

	// Clear out these two vectors just in case
	getLgyRegs()->a7_vector[0] = 0;
	getLgyRegs()->a7_vector[1] = 0;
	TIMER_sleepMs(1);

	// Restore original vectors, execution should begin flowing again.
	iomemcpy(&getLgyRegs()->a7_vector[2], &biosVectors[2], 0x20-8);
	TIMER_sleepMs(1);

	// Restore reset vector in case a game uses it.
	getLgyRegs()->a7_vector[0] = biosVectors[0];
	getLgyRegs()->a7_vector[1] = biosVectors[1];

	bIsSleep = false;
}
