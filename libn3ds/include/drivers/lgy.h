#pragma once

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

#include "types.h"
#include "error_codes.h"


#define MAX_ROM_SIZE    (1024u * 1024 * 32)
#define MAX_SAVE_SIZE   (1024u * 128)
#define ARM7_STUB_LOC   (0x3007E00u)
#define ARM7_STUB_LOC9  (0x80BFE00u)
#define ROM_LOC         (0x20000000u)
#define SAVE_LOC        (0x8080000u)


// REG_LGY_MODE
#define LGY_MODE_TWL           (1u)
#define LGY_MODE_AGB           (2u)
#define LGY_MODE_START         (1u<<15)

// REG_LGY_GBA_SAVE_TYPE
enum
{
	SAVE_TYPE_EEPROM_8k          = 0x0u, // "[save] in upper 16Mbyte of ROM area"
	SAVE_TYPE_EEPROM_8k_2        = 0x1u, // "[save] in upper 100h byte of ROM area"
	SAVE_TYPE_EEPROM_64k         = 0x2u, // "[save] in upper 16Mbyte of ROM area"
	SAVE_TYPE_EEPROM_64k_2       = 0x3u, // "[save] in upper 100h byte of ROM area"
	SAVE_TYPE_FLASH_512k_AML_RTC = 0x4u, // "FLASH ID=3D1Fh, Atmel"
	SAVE_TYPE_FLASH_512k_AML     = 0x5u, // "FLASH ID=3D1Fh, Atmel"
	SAVE_TYPE_FLASH_512k_SST_RTC = 0x6u, // "FLASH ID=D4BFh, SST"
	SAVE_TYPE_FLASH_512k_SST     = 0x7u, // "FLASH ID=D4BFh, SST"
	SAVE_TYPE_FLASH_512k_PSC_RTC = 0x8u, // "FLASH ID=1B32h, Panasonic"
	SAVE_TYPE_FLASH_512k_PSC     = 0x9u, // "FLASH ID=1B32h, Panasonic"
	SAVE_TYPE_FLASH_1m_MRX_RTC   = 0xAu, // "FLASH ID=09C2h, Macronix"
	SAVE_TYPE_FLASH_1m_MRX       = 0xBu, // "FLASH ID=09C2h, Macronix"
	SAVE_TYPE_FLASH_1m_SNO_RTC   = 0xCu, // "FLASH ID=1362h, Sanyo"
	SAVE_TYPE_FLASH_1m_SNO       = 0xDu, // "FLASH ID=1362h, Sanyo"
	SAVE_TYPE_SRAM_256k          = 0xEu,
	SAVE_TYPE_NONE               = 0xFu,

	SAVE_TYPE_MASK               = SAVE_TYPE_NONE
};

// REG_LGY_GBA_SAVE_MAP
#define LGY_SAVE_MAP_7         (0u)
#define LGY_SAVE_MAP_9         (1u)

// REG_LGY_GBA_RTC_CNT
#define LGY_RTC_CNT_WR         (1u)     // Write date and time.
#define LGY_RTC_CNT_RD         (1u<<1)  // Read date and time.
#define LGY_RTC_CNT_WR_ERR     (1u<<14) // Write error (wrong date/time).
#define LGY_RTC_CNT_BUSY       (1u<<15)

// REG_LGY_GBA_RTC_BCD_DATE
// Shifts
#define LGY_RTC_BCD_Y_SHIFT    (0u)
#define LGY_RTC_BCD_MON_SHIFT  (8u)
#define LGY_RTC_BCD_D_SHIFT    (16u)
#define LGY_RTC_BCD_W_SHIFT    (24u)
// Masks
#define LGY_RTC_BCD_Y_MASK     (0xFFu<<LGY_RTC_BCD_Y_SHIFT)
#define LGY_RTC_BCD_MON_MASK   (0x1Fu<<LGY_RTC_BCD_M_SHIFT)
#define LGY_RTC_BCD_D_MASK     (0x3Fu<<LGY_RTC_BCD_D_SHIFT)
#define LGY_RTC_BCD_W_MASK     (0x07u<<LGY_RTC_BCD_W_SHIFT)

// REG_LGY_GBA_RTC_BCD_TIME
// Shifts
#define LGY_RTC_BCD_H_SHIFT    (0u)
#define LGY_RTC_BCD_MIN_SHIFT  (8u)
#define LGY_RTC_BCD_S_SHIFT    (16u)
// Masks
#define LGY_RTC_BCD_H_MASK     (0x3Fu<<LGY_RTC_BCD_H_SHIFT)
#define LGY_RTC_BCD_MIN_MASK   (0x7Fu<<LGY_RTC_BCD_MIN_SHIFT)
#define LGY_RTC_BCD_S_MASK     (0x7Fu<<LGY_RTC_BCD_S_SHIFT)

// All values in BCD.
typedef struct
{
	union
	{
		struct
		{
			u8 h;
			u8 min;
			u8 s;
			u8 unused;
		};
		u32 time;
	};
	union
	{
		struct
		{
			u8 y;
			u8 mon;
			u8 d;
			u8 dow; // Day of week.
		};
		u32 date;
	};
} ALIGN(8) GbaRtc; // Workaround: Poor optimization on pass by value.

// REGs_LGY_GBA_SAVE_TIMING



Result LGY_prepareGbaMode(bool directBoot, u16 saveType, const char *const savePath);
Result LGY_setGbaRtc(const GbaRtc rtc);
Result LGY_getGbaRtc(GbaRtc *const out);
Result LGY_backupGbaSave(void);
#ifdef ARM11
void LGY_switchMode(void);
void LGY_handleOverrides(void);
void LGY_deinit(void);
#endif
