#include "types.h"
#include "hardware/lgy.h"
#include "hardware/pxi.h"
#include "ipc_handler.h"
#include "arm11/hardware/hid.h"
#include "arm11/hardware/interrupt.h"
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

Result LGY_prepareGbaMode(bool gbaBios, u16 saveType)
{
	const u32 cmdBuf[2] = {gbaBios, saveType};
	Result res = PXI_sendCmd(IPC_CMD9_PREPARE_GBA, cmdBuf, 2);
	if(res != RES_OK) return res;

	GbaRtc rtc;
	MCU_getRTCTime((u8*)&rtc);
	rtc.time = __builtin_bswap32(rtc.time)>>8;
	rtc.date = __builtin_bswap32(rtc.date)>>8;
	// TODO: Do we need to set day of week?
	LGY_setGbaRtc(rtc);

	LGYFB_init();

	//flushInvalidateDCache();
	// Unmap FCRAM if mapped.

	while(!REG_LGY_MODE); // Wait until legacy mode is ready.
	// FCRAM reset and disable.
	*((vu32*)0x10201000) &= ~1u;     // Disable DRAM controller? If bit 0 set below reg pokes do nothing.
	*((vu8*)0x10141210) = 0;         // Set reset low (active).
	*((vu8*)0x10141210) = 1;         // Take it out of reset.
	while(*((vu8*)0x10141210) & 4u); // Wait for acknowledge?

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
	//ee_puts("Done. Doing final switch...");
	//updateScreens();
	/*do
	{
		while(*((vu16*)0x10008004) & 0x100u); // ARM9 reg pokes
	} while(*((vu32*)0x1000800C) != 0x...);*/
	REG_LGY_MODE = 0x8000u;
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

void LGY_deinit(void)
{
	//REG_LGY_PAD_VAL = 0x1FFF; // Force all buttons not pressed.
	//REG_LGY_PAD_SEL = 0x1FFF;

	// TODO: Tell ARM9 to backup the savegame instead of handling it on poweroff.
	LGYFB_deinit();

	IRQ_unregisterIsr(IRQ_LGY_SLEEP);
	IRQ_unregisterIsr(IRQ_HID_PADCNT);
}
