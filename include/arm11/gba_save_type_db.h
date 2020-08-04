#pragma once

#include <assert.h>
#include "types.h"


// Temporary define for all incomplete entries.
#define UNSPECIFIED 0xFF


typedef struct
{
	char gameCode[3]; // Without the region letter.
	u8 type;
} SaveTypeLut;
static_assert(offsetof(SaveTypeLut, type) == 3, "Error: Member 'type' of SaveTypeLut is not at offset 3!");


/*
 * 0x0 = 16 MiB or smaller ROM + EEPROM 4k/8k (512/1024 bytes)
 * 0x1 = 32 MiB ROM + EEPROM 4k/8k (512/1024 bytes)
 * 0x2 = 16 MiB or smaller ROM + EEPROM 64k (8 KiB)
 * 0x3 = 32 MiB ROM + EEPROM 64k (8 KiB)
 * 0x4 = Flash 512k (64 KiB) with RTC, ID=0x3D1F, Atmel
 * 0x5 = Flash 512k (64 KiB) without RTC, ID=0x3D1F, Atmel
 * 0x6 = Flash 512k (64 KiB) with RTC, ID=0xD4BF, SST
 * 0x7 = Flash 512k (64 KiB) without RTC, ID=0xD4BF, SST
 * 0x8 = Flash 512k (64 KiB) with RTC, ID=0x1B32, Panasonic
 * 0x9 = Flash 512k (64 KiB) without RTC, ID=0x1B32, Panasonic
 * 0xA = Flash 1M (128 KiB) with RTC, ID=0x09C2, Macronix
 * 0xB = Flash 1M (128 KiB) without RTC, ID=0x09C2, Macronix
 * 0xC = Flash 1M (128 KiB) with RTC, ID=0x1362, Sanyo
 * 0xD = Flash 1M (128 KiB) without RTC, ID=0x1362, Sanyo
 * 0xE = SRAM/FRAM/FeRAM 256k (32 KiB)
 * 0xF = No save chip
 */

/*
 * [] = Optional
 * <> = Required
 *
 * Format:
 * // [SDK save string if any]
 * // <no-intro game release names sepearated by newlines>
 * {"<game code without region (last letter)>", <save type>},
 *
 * All entries ordered by release number.
 */
alignas(4) static const SaveTypeLut saveTypeLut[] =
{
	// EEPROM_V120
	// 0002 - Super Mario Advance - Super Mario USA + Mario Brothers (Japan)
	// 0049 - Super Mario Advance (USA, Europe)
	// 1570 - Chaoji Maliou 2 (China)
	// x116 - Super Mario Advance (USA, Europe) (Wii U Virtual Console)
	{"AMA", 0x0},

	// EEPROM_V122
	// 0237 - Super Mario Advance 2 - Super Mario World + Mario Brothers (Japan)
	// 0288 - Super Mario Advance 2 - Super Mario World (USA, Australia)
	// 0389 - Super Mario Advance 2 - Super Mario World (Europe) (En,Fr,De,Es)
	// 2328 - Chaoji Maliou Shijie (China)
	{"AA2", 0x2},

	// EEPROM_V122
	// 0578 - Super Mario Advance 3 - Yoshi's Island (USA)
	// 0580 - Super Mario Advance 3 - Yoshi's Island + Mario Brothers (Japan)
	// 0608 - Super Mario Advance 3 - Yoshi's Island (Europe) (En,Fr,De,Es,It)
	// 2299 - Yaoxi Dao (China)
	// x115 - Super Mario Advance 3 - Yoshi's Island (USA) (Wii U Virtual Console)
	// x161 - Super Mario Advance 3 - Yoshi's Island (Europe) (En,Fr,De,Es,It) (Wii U Virtual Console)
	{"A3A", 0x2},
};
