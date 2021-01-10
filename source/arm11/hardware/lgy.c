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


#define LGY_REGS_BASE     (IO_MEM_ARM9_ARM11 + 0x41100)
#define REG_LGY_MODE      *((      vu16*)(LGY_REGS_BASE + 0x00))
#define REG_LGY_SLEEP     *((      vu16*)(LGY_REGS_BASE + 0x04))
#define REG_LGY_UNK       *((const vu16*)(LGY_REGS_BASE + 0x08)) // IRQ related?
#define REG_LGY_PADCNT    *((const vu16*)(LGY_REGS_BASE + 0x0A)) // Read-only mirror of ARM7 "KEYCNT".
#define REG_LGY_PAD_SEL   *((      vu16*)(LGY_REGS_BASE + 0x10)) // Select which keys to override. 1 = selected.
#define REG_LGY_PAD_VAL   *((      vu16*)(LGY_REGS_BASE + 0x12)) // Override value. Each bit 0 = pressed.
#define REG_LGY_GPIO_SEL  *((      vu16*)(LGY_REGS_BASE + 0x14)) // Select which GPIOs to override. 1 = selected.
#define REG_LGY_GPIO_VAL  *((      vu16*)(LGY_REGS_BASE + 0x16)) // Override value.
#define REG_LGY_UNK2      *((       vu8*)(LGY_REGS_BASE + 0x18)) // DSi gamecard detection select?
#define REG_LGY_UNK3      *((       vu8*)(LGY_REGS_BASE + 0x19)) // DSi gamecard detection value?
#define REG_LGY_UNK4      *((const  vu8*)(LGY_REGS_BASE + 0x20)) // Some legacy status bits?



static void lgySleepIsr(u32 intSource)
{
	if(intSource == IRQ_LGY_SLEEP)
	{
		// Workaround for The Legend of Zelda - A Link to the Past.
		// This game doesn't set the IRQ enable bit so we force it
		// on the 3DS side. Unknown if other games have this bug.
		REG_HID_PADCNT = REG_LGY_PADCNT | 1u<<14;
	}
	else // IRQ_HID_PADCNT
	{
		// TODO: Synchronize with LCD VBlank.
		REG_HID_PADCNT = 0;
		REG_LGY_SLEEP |= 1u; // Acknowledge and wakeup.
	}
}

static void powerDownFcramForLegacy(u8 mode)
{
	flushDCache();
	// TODO: Unmap FCRAM.
	while(!REG_LGY_MODE); // Wait until legacy mode is ready.

	// For GBA mode we need to additionally apply a bug fix and reset FCRAM.
	if(mode == LGY_MODE_AGB)
	{
		*((vu32*)0x10201000) &= ~1u;                    // Bug fix for the GBA cart emu?
		REG_PDN_FCRAM_CNT = PDN_FCRAM_CNT_CLK_E;        // Set reset low (active) but keep clock on.
	}
	REG_PDN_FCRAM_CNT = PDN_FCRAM_CNT_RST;              // Take it out of reset but disable clock.
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
	rtc.date = __builtin_bswap32(rtc.date)>>8; // TODO: Do we need to set day of week?
	LGY_setGbaRtc(rtc);

	// Setup FCRAM for GBA mode.
	powerDownFcramForLegacy(LGY_MODE_AGB);

	// Setup IRQ handlers and sleep mode handling.
	REG_LGY_SLEEP = 1u<<15;
	IRQ_registerIsr(IRQ_LGY_SLEEP, 14, 0, lgySleepIsr);
	IRQ_registerIsr(IRQ_HID_PADCNT, 14, 0, lgySleepIsr);

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
