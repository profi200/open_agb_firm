/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2026 Dominik Leicht
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
#include "arm.h"
#include "drivers/gfx.h"
#include "drivers/lgy_common.h"
#include "arm11/config.h"
#include "arm11/oaf_video.h"
#include "arm11/power.h"
#include "arm11/drivers/codec.h"
#include "arm11/drivers/gpio.h"
#include "arm11/drivers/lgy11.h"
#include "arm11/drivers/lgycap.h"
#include "arm11/drivers/mcu.h"



typedef enum
{
	GBA_STOP_FAILED = 0,
	GBA_STOP_NATIVE,
	GBA_STOP_HOST
} GbaStopOwner;

static GbaStopOwner enterGbaStop(void)
{
	/* Preserve a game's own in-game STOP state across a host lid cycle. */
	if(LGY11_isGbaSleeping())
		return GBA_STOP_NATIVE;

	/*
	 * Host-controlled lid sleep wakes through LGY_SLEEP_CNT, not the game's
	 * KEYCNT configuration. Disable the normal keypad wake bridge only while
	 * OAF owns this STOP transition.
	 */
	LGY11_setGbaKeypadWakeBridge(false);

	/* Redirect exactly one normal GBA IRQ to the overlay STOP trampoline. */
	if(LGY_setGbaStopVector(true) != RES_OK)
	{
		LGY11_setGbaKeypadWakeBridge(true);
		return GBA_STOP_FAILED;
	}

	/* Cancel cleanly if the lid opens before the GBA reaches STOP. */
	while(!LGY11_isGbaSleeping() && GPIO_read(GPIO_1_SHELL))
		__wfi();

	/* The normal BIOS IRQ path must be restored before the GBA wakes. */
	if(LGY_setGbaStopVector(false) != RES_OK)
	{
		LGY11_setGbaKeypadWakeBridge(true);
		return GBA_STOP_FAILED;
	}

	if(!LGY11_isGbaSleeping())
	{
		LGY11_setGbaKeypadWakeBridge(true);
		return GBA_STOP_FAILED;
	}

	return GBA_STOP_HOST;
}

static void wakeGba(const GbaStopOwner owner)
{
	if(owner != GBA_STOP_HOST)
		return;

	LGY11_wakeGba();
	LGY11_setGbaKeypadWakeBridge(true);
}

void OAF_sleep(void)
{
	if(!GPIO_read(GPIO_1_SHELL))
		return;

	const GbaStopOwner gbaStopOwner = enterGbaStop();
	if(gbaStopOwner == GBA_STOP_FAILED)
		return;

	/* Quiesce OAF-owned devices before whole-system PDN sleep. */
	CODEC_deinit();
	OAF_videoSuspend();
	GFX_sleep();
	MCU_setPowerLedPattern(MCU_PWR_LED_SLEEP);

	/*
	 * power_sleep() owns the ARM11/PDN transition. If a prerequisite is not
	 * available, stay quiesced until the lid opens and use the same cold
	 * graphics reconstruction path.
	 */
	if(!power_sleep())
	{
		while(GPIO_read(GPIO_1_SHELL))
			__wfi();
	}

	GFX_sleepAwakeCold();
	OAF_videoResume();

#ifdef NDEBUG
	/* Cold graphics resume clears force-black; restore OAF's release policy. */
	GFX_setForceBlack(false, true);
#endif

	wakeGba(gbaStopOwner);

	MCU_setPowerLedPattern(MCU_PWR_LED_AUTO);
	LGYCAP_start(LGYCAP_DEV_TOP);
	CODEC_wakeup();
	CODEC_setVolumeOverride(g_oafConfig.volume);
}
