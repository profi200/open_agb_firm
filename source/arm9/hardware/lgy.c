#include <string.h>
#include "types.h"
#include "hardware/lgy.h"
#include "error_codes.h"
#include "mem_map.h"
#include "mmio.h"
#include "arm9/hardware/ndma.h"
#include "arm9/arm7_stub.h"
#include "fs.h"
#include "arm9/debug.h"
#include "arm9/hardware/crypto.h"
#include "util.h"


#define LGY_REGS_BASE             (IO_MEM_ARM9_ONLY + 0x18000)
#define REG_LGY_MODE              *((vu16*)(LGY_REGS_BASE + 0x000))
#define REGs_LGY_A7_VECTOR         ((vu32*)(LGY_REGS_BASE + 0x080)) // ARM7 vector override 32 bytes. Writable at runtime.
#define REG_LGY_GBA_SAVE_TYPE     *((vu16*)(LGY_REGS_BASE + 0x100))
#define REG_LGY_GBA_SAVE_MAP      *((vu8* )(LGY_REGS_BASE + 0x104)) // Remaps the GBA save to ARM9 if set to 1.
#define REG_LGY_GBA_RTC_CNT       *((vu16*)(LGY_REGS_BASE + 0x108))
#define REG_LGY_GBA_RTC_BCD_DATE  *((vu32*)(LGY_REGS_BASE + 0x110))
#define REG_LGY_GBA_RTC_BCD_TIME  *((vu32*)(LGY_REGS_BASE + 0x114))
#define REG_LGY_GBA_RTC_HEX_TIME  *((vu32*)(LGY_REGS_BASE + 0x118)) // Writing bit 7 at runtime completely deadlocks the ARM7.
#define REG_LGY_GBA_RTC_HEX_DATE  *((vu32*)(LGY_REGS_BASE + 0x11C))
#define REGs_LGY_GBA_SAVE_TIMING   ((vu32*)(LGY_REGS_BASE + 0x120))


static u32 g_saveSize = 0;
static u32 g_saveHash[8] = {0};
static char g_savePath[256] = {0};



#define STRINGIFY(s) #s
#define STR(s) STRINGIFY(s)
NAKED static void _overlay_stub(void)
{
	__asm__("mov r0, #0x4000000\n\t"
	        "mov r1, #1\n\t"
	        "strb r1, [r0, #0x300]\n\t"   // "POSTFLG"
	        "ldr pc, _overlay_stub_jmp\n\t"
	        "_overlay_stub_jmp: .4byte " STR(ARM7_STUB_LOC) "\n\t"
	        "_overlay_stub_size = . - _overlay_stub\n\t" : : : );
}
extern const u32 _overlay_stub_size[];

static void setupBiosOverlay(bool biosIntro)
{
	iomemcpy(REGs_LGY_A7_VECTOR, (u32*)_overlay_stub, (u32)_overlay_stub_size);
	//static const u32 biosVectors[8] = {0xEA000018, 0xEA000004, 0xEA00004C, 0xEA000002,
	//                                   0xEA000001, 0xEA000000, 0xEA000042, 0xE59FD1A0};
	//iomemcpy(REGs_LGY_A7_VECTOR, biosVectors, 32);

	NDMA_copy((u32*)ARM7_STUB_LOC9, _arm7_stub_start, (u32)_arm7_stub_size);
	// Patch swi 0x01 (RegisterRamReset) to swi 0x26 (HardReset).
	if(biosIntro) *((u8*)_arm7_stub_swi) = 0x26;
}

static void setupSaveType(u16 saveType)
{
	REG_LGY_GBA_SAVE_TYPE = saveType;
	static const u8 saveSizeLut[16] = {1, 1, 8, 8, 64, 64, 64, 64, 64, 64, 128, 128, 128, 128, 32, 0};
	g_saveSize = 1024u * saveSizeLut[saveType & 0xFu];

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
	iomemcpy(REGs_LGY_GBA_SAVE_TIMING, saveTm, 16);
}

Result LGY_prepareGbaMode(bool biosIntro, u16 saveType, const char *const savePath)
{
	REG_LGY_MODE = LGY_MODE_AGB;

	setupBiosOverlay(biosIntro);
	setupSaveType(saveType);
	strncpy_s(g_savePath, savePath, 255, 256);

	Result res = RES_OK;
	if(g_saveSize != 0)
	{
		FHandle f;
		if(fOpen(&f, savePath, FA_OPEN_EXISTING | FA_READ) == RES_OK)
		{
			res = fRead(f, (void*)SAVE_LOC, MAX_SAVE_SIZE, NULL);
			fClose(f);
		}
		else NDMA_fill((u32*)SAVE_LOC, 0xFFFFFFFFu, g_saveSize);

		// Hash the savegame so it's only backed up when changed.
		sha((u32*)SAVE_LOC, g_saveSize, g_saveHash, SHA_INPUT_BIG | SHA_MODE_256, SHA_OUTPUT_BIG);
	}

	return res;
}

Result LGY_setGbaRtc(const GbaRtc rtc)
{
	// Set base time and date.
	REG_LGY_GBA_RTC_BCD_TIME = rtc.time;
	REG_LGY_GBA_RTC_BCD_DATE = rtc.date;

	//while(REG_LGY_GBA_RTC_CNT & LGY_RTC_CNT_BUSY);
	//REG_LGY_GBA_RTC_CNT = 0; // Legacy P9 does this. Useless?
	REG_LGY_GBA_RTC_HEX_TIME = 1u<<15; // Time offset 0 and 24h format.
	REG_LGY_GBA_RTC_HEX_DATE = 0;      // Date offset 0.
	REG_LGY_GBA_RTC_CNT = LGY_RTC_CNT_WR;
	while(REG_LGY_GBA_RTC_CNT & LGY_RTC_CNT_BUSY);

	if(REG_LGY_GBA_RTC_CNT & LGY_RTC_CNT_WR_ERR) return RES_GBA_RTC_ERR;
	else                                         return RES_OK;
}

Result LGY_getGbaRtc(GbaRtc *const out)
{
	//while(REG_LGY_GBA_RTC_CNT & LGY_RTC_CNT_BUSY);
	//REG_LGY_GBA_RTC_CNT = 0; // Legacy P9 does this. Useless?
	REG_LGY_GBA_RTC_CNT = LGY_RTC_CNT_RD;
	while(REG_LGY_GBA_RTC_CNT & LGY_RTC_CNT_BUSY);

	if((REG_LGY_GBA_RTC_CNT & LGY_RTC_CNT_WR_ERR) == 0u)
	{
		out->time = REG_LGY_GBA_RTC_BCD_TIME;
		out->date = REG_LGY_GBA_RTC_BCD_DATE;

		return RES_OK;
	}

	return RES_GBA_RTC_ERR;
}

/*#include "arm9/hardware/timer.h"
void LGY_gbaReset(void)
{
	// Hook the SVC vector for 17 ms and hope it jumps to the
	// BIOS reset function skipping the "POSTFLG" check.
	REGs_LGY_A7_VECTOR[2] = 0xEA00001F; // SVC_vector: b 0x8C @ resetHandler
	TIMER_sleep(17); // Wait 17 ms.
	// Restore vector.
	REGs_LGY_A7_VECTOR[2] = 0xEA00004C; // SVC_vector: b 0x140 @ svcHandler
}*/

Result LGY_backupGbaSave(void)
{
	Result res = RES_OK;
	if(g_saveSize != 0)
	{
		// Enable savegame mem region.
		REG_LGY_GBA_SAVE_MAP = LGY_SAVE_MAP_9;

		u32 newHash[8];
		sha((u32*)SAVE_LOC, g_saveSize, newHash, SHA_INPUT_BIG | SHA_MODE_256, SHA_OUTPUT_BIG);
		if(memcmp(g_saveHash, newHash, 32) != 0) // Backup save if changed.
		{
			// Update hash.
			memcpy(g_saveHash, newHash, 32);

			FHandle f;
			if((res = fOpen(&f, g_savePath, FA_OPEN_ALWAYS | FA_WRITE)) == RES_OK)
			{
				res = fWrite(f, (void*)SAVE_LOC, g_saveSize, NULL);
				fClose(f);
			}
		}

		// Disable savegame mem region.
		REG_LGY_GBA_SAVE_MAP = LGY_SAVE_MAP_7;
	}

	return res;
}
