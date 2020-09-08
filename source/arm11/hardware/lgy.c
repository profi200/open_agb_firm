#include <string.h>
#include "types.h"
#include "hardware/lgy.h"
#include "hardware/pxi.h"
#include "ipc_handler.h"
#include "arm11/hardware/hid.h"
#include "arm11/hardware/interrupt.h"
#include "hardware/cache.h"
#include "arm11/hardware/pdn.h"
#include "arm11/hardware/mcu.h"
#include "arm11/fmt.h"


#define LGY_REGS_BASE     (IO_MEM_ARM9_ARM11 + 0x41100)
#define REG_LGY_MODE      *((      vu16*)(LGY_REGS_BASE + 0x00))
#define REG_LGY_SLEEP     *((      vu16*)(LGY_REGS_BASE + 0x04))
#define REG_LGY_UNK       *((const vu16*)(LGY_REGS_BASE + 0x08)) // IRQ related?
#define REG_LGY_PADCNT    *((const vu16*)(LGY_REGS_BASE + 0x0A)) // ARM7 "KEYCNT"
#define REG_LGY_PAD_SEL   *((      vu16*)(LGY_REGS_BASE + 0x10)) // Select which keys to override.
#define REG_LGY_PAD_VAL   *((      vu16*)(LGY_REGS_BASE + 0x12)) // Override value.
#define REG_LGY_GPIO_SEL  *((      vu16*)(LGY_REGS_BASE + 0x14)) // Select which GPIOs to override.
#define REG_LGY_GPIO_VAL  *((      vu16*)(LGY_REGS_BASE + 0x16)) // Override value.
#define REG_LGY_UNK2      *((       vu8*)(LGY_REGS_BASE + 0x18)) // DSi gamecard detection select?
#define REG_LGY_UNK3      *((       vu8*)(LGY_REGS_BASE + 0x19)) // DSi gamecard detection value?
#define REG_LGY_UNK4      *((const  vu8*)(LGY_REGS_BASE + 0x20)) // Some legacy status bits?



static void lgySleepIrqHandler(u32 intSource)
{
	if(intSource == IRQ_LGY_SLEEP)
	{
		REG_HID_PADCNT = REG_LGY_PADCNT;
	}
	else // IRQ_HID_PADCNT
	{
		// TODO: Synchronize with LCD VBlank.
		REG_HID_PADCNT = 0;
		REG_LGY_SLEEP |= 1u; // Acknowledge and wakeup.
	}
}

static void setupFcramForGbaMode(void)
{
	// FCRAM reset and clock disable.
	flushDCache();
	// TODO: Unmap FCRAM.
	while(!REG_LGY_MODE); // Wait until legacy mode is ready.
	*((vu32*)0x10201000) &= ~1u;                        // Some kind of bug fix for the GBA cart emu?
	REG_PDN_FCRAM_CNT = 0;                              // Set reset low (active).
	REG_PDN_FCRAM_CNT = PDN_FCRAM_CNT_RST;              // Take it out of reset.
	while(REG_PDN_FCRAM_CNT & PDN_FCRAM_CNT_CLK_E_ACK); // Wait until clock is disabled.
}

Result LGY_prepareGbaMode(bool biosIntro, u16 saveType, const char *const savePath)
{
	u32 cmdBuf[4];
	cmdBuf[0] = (u32)savePath;
	cmdBuf[1] = strlen(savePath) + 1;
	cmdBuf[2] = biosIntro;
	cmdBuf[3] = saveType;
	Result res = PXI_sendCmd(IPC_CMD9_PREPARE_GBA, cmdBuf, 4);
	if(res != RES_OK) return res;

	// Setup GBA Real-Time Clock.
	GbaRtc rtc;
	MCU_getRTCTime((u8*)&rtc);
	rtc.time = __builtin_bswap32(rtc.time)>>8;
	rtc.date = __builtin_bswap32(rtc.date)>>8;
	// TODO: Do we need to set day of week?
	LGY_setGbaRtc(rtc);

	// Setup FCRAM for GBA mode.
	setupFcramForGbaMode();

	// Setup IRQ handlers and sleep mode handling.
	REG_LGY_SLEEP = 1u<<15;
	IRQ_registerIsr(IRQ_LGY_SLEEP, 14, 0, lgySleepIrqHandler);
	IRQ_registerIsr(IRQ_HID_PADCNT, 14, 0, lgySleepIrqHandler);

	return RES_OK;
}

Result LGY_setGbaRtc(const GbaRtc rtc)
{
	return PXI_sendCmd(IPC_CMD9_SET_GBA_RTC, (u32*)&rtc, sizeof(GbaRtc) / 4);
}

Result LGY_getGbaRtc(GbaRtc *const out)
{
	const u32 cmdBuf[2] = {(u32)out, sizeof(GbaRtc)};
	return PXI_sendCmd(IPC_CMD9_GET_GBA_RTC, cmdBuf, 2);
}

void LGY_switchMode(void)
{
	REG_LGY_MODE = LGY_MODE_START;
}

#ifndef NDEBUG
#include "arm11/hardware/gx.h"
#include "arm11/hardware/gpu_regs.h"
void debugTests(void)
{
	const u32 kDown = hidKeysDown();

	// Print GBA RTC date/time.
	if(kDown & KEY_X)
	{
		GbaRtc rtc; LGY_getGbaRtc(&rtc);
		ee_printf("RTC: %02X.%02X.%04X %02X:%02X:%02X\n", rtc.d, rtc.mon, rtc.y + 0x2000u, rtc.h, rtc.min, rtc.s);

		/*static u8 filter = 1;
		filter ^= 1;
		u32 texEnvSource = 0x000F000F;
		u32 texEnvCombiner = 0x00000000;
		if(filter == 1)
		{
			texEnvSource = 0x00FF00FFu;
			texEnvCombiner = 0x00010001u;
		}
		REG_GX_P3D(GPUREG_TEXENV1_SOURCE) = texEnvSource;
		REG_GX_P3D(GPUREG_TEXENV1_COMBINER) = texEnvCombiner;*/

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
	//if(kDown & KEY_Y) LGYFB_dbgDumpFrame();
}
#endif

void LGY_handleOverrides(void)
{
	// Override D-Pad if Circle-Pad is used.
	const u32 kHeld = hidKeysHeld();
	u16 padSel;
	if(kHeld & KEY_CPAD_MASK)
	{
		REG_LGY_PAD_VAL = (kHeld>>24) ^ KEY_DPAD_MASK;
		padSel = KEY_DPAD_MASK;
	}
	else padSel = 0;
	REG_LGY_PAD_SEL = padSel;

#ifndef NDEBUG
	debugTests();
#endif
}

Result LGY_backupGbaSave(void)
{
	return PXI_sendCmd(IPC_CMD9_BACKUP_GBA_SAVE, NULL, 0);
}

void LGY_deinit(void)
{
	LGY_backupGbaSave();

	IRQ_unregisterIsr(IRQ_LGY_SLEEP);
	IRQ_unregisterIsr(IRQ_HID_PADCNT);
}
