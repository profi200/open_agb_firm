#pragma once

#include "types.h"


typedef struct
{
	char gameCode[3]; // Without the region letter.
	u8 type;
} SaveTypeLut;
static_assert(offsetof(SaveTypeLut, type) == 3, "Error: Member type of SaveTypeLut is not at offset 3!");


/*
 * 0x0 = EEPROM 4k/8k (512/1024 bytes)
 * 0x2 = EEPROM 64k (8 KiB)
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

// TODO: Decide how each entry should look like.
alignas(4) static const SaveTypeLut saveTypeLut[] =
{
	{"AMA", 0x0}, // EEPROM_V120 Super Mario Advance
	{"AA2", 0x2}, // EEPROM_V122 Super Mario Advance 2 - Super Mario World
	{"A3A", 0x2}, // EEPROM_V122 Super Mario Advance 3 - Yoshi's Island
};
