#include <string.h>
#include "types.h"
#include "hardware/lgy.h"
#include "hardware/pxi.h"
#include "ipc_handler.h"
#include "arm11/hardware/hid.h"
#include "arm11/hardware/interrupt.h"
#include "fs.h"
#include "hardware/cache.h"
#include "arm11/hardware/pdn.h"
#include "arm11/hardware/mcu.h"
#include "arm11/hardware/lgyfb.h"


#define LGY_REGS_BASE     (IO_MEM_ARM9_ARM11 + 0x41100)
#define REG_LGY_MODE      *((vu16*)(LGY_REGS_BASE + 0x00))
#define REG_LGY_SLEEP     *((vu16*)(LGY_REGS_BASE + 0x04))
#define REG_LGY_UNK       *((vu16*)(LGY_REGS_BASE + 0x08)) // IRQ related?
#define REG_LGY_PADCNT    *((vu16*)(LGY_REGS_BASE + 0x0A)) // ARM7 "KEYCNT"
#define REG_LGY_PAD_SEL   *((vu16*)(LGY_REGS_BASE + 0x10)) // Select which keys to override.
#define REG_LGY_PAD_VAL   *((vu16*)(LGY_REGS_BASE + 0x12)) // Override value.
#define REG_LGY_GPIO_SEL  *((vu16*)(LGY_REGS_BASE + 0x14)) // Select which GPIOs to override.
#define REG_LGY_GPIO_VAL  *((vu16*)(LGY_REGS_BASE + 0x16)) // Override value.



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

static Result loadRom(const char *const path)
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
				// Pad ROM area with "open bus" value.
				memset((void*)(ROM_LOC + romSize), 0xFFFFFFFFu, romSize);
			}
		}
		else res = RES_ROM_TOO_BIG;

		fClose(f);
	}

	return res;
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

Result LGY_prepareGbaMode(bool gbaBios, u16 saveType, const char *const romPath, const char *const savePath)
{
	// Load the ROM image.
	Result res = loadRom(romPath);
	if(res != RES_OK) return res;

	// Prepare ARM9 for GBA mode + settings and save loading.
	u32 cmdBuf[2];
	cmdBuf[0] = (u32)savePath;
	cmdBuf[1] = strlen(savePath) + 1;
	cmdBuf[2] = gbaBios;
	cmdBuf[3] = saveType;
	res = PXI_sendCmd(IPC_CMD9_PREPARE_GBA, cmdBuf, 4);
	if(res != RES_OK) return res;

	// Setup GBA Real-Time Clock.
	GbaRtc rtc;
	MCU_getRTCTime((u8*)&rtc);
	rtc.time = __builtin_bswap32(rtc.time)>>8;
	rtc.date = __builtin_bswap32(rtc.date)>>8;
	// TODO: Do we need to set day of week?
	LGY_setGbaRtc(rtc);

	// Setup Legacy Framebuffer.
	LGYFB_init();

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
	return PXI_sendCmd(IPC_CMD9_SET_GBA_RTC, (u32*)&rtc, 2);
}

Result LGY_getGbaRtc(GbaRtc *const out)
{
	const u32 cmdBuf[2] = {(u32)out, sizeof(GbaRtc)};
	return PXI_sendCmd(IPC_CMD9_GET_GBA_RTC, cmdBuf, sizeof(GbaRtc) / 4);
}

void LGY_switchMode(void)
{
	REG_LGY_MODE = LGY_MODE_START;
}

#ifndef NDEBUG
#include "arm11/fmt.h"
void debugTests(void)
{
	const u32 kDown = hidKeysDown();

	// Print GBA RTC date/time.
	if(kDown & KEY_X)
	{
		GbaRtc rtc; LGY_getGbaRtc(&rtc);
		ee_printf("RTC: %02X.%02X.%04X %02X:%02X:%02X\n", rtc.d, rtc.mon, rtc.y + 0x2000u, rtc.h, rtc.min, rtc.s);
	}
}
#endif

void LGY_handleEvents(void)
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

	LGYFB_processFrame();

	// Bit 0 triggers wakeup. Bit 1 sleep state/ack sleep end. Bit 2 unk. Bit 15 IRQ enable (triggers IRQ 89).
	//if(REG_LGY_SLEEP & 2u) REG_HID_PADCNT = REG_LGY_PADCNT;
}

Result LGY_backupGbaSave(void)
{
	return PXI_sendCmd(IPC_CMD9_BACKUP_GBA_SAVE, NULL, 0);
}

void LGY_deinit(void)
{
	//REG_LGY_PAD_VAL = 0x1FFF; // Force all buttons not pressed.
	//REG_LGY_PAD_SEL = 0x1FFF;

	LGY_backupGbaSave();
	LGYFB_deinit();

	IRQ_unregisterIsr(IRQ_LGY_SLEEP);
	IRQ_unregisterIsr(IRQ_HID_PADCNT);
}
