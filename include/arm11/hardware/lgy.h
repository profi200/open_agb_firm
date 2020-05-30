#pragma once

#include "types.h"
#include "error_codes.h"


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
	SAVE_TYPE_NONE               = 0xFu
};

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
} GbaRtc;



Result LGY_prepareGbaMode(bool gbaBios, u16 saveType);
Result LGY_setGbaRtc(GbaRtc rtc);
Result LGY_getGbaRtc(GbaRtc *out);
void LGY_switchMode(void);
void LGY_handleEvents(void);
void LGY_deinit(void);
