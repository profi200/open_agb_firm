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

#include <math.h>
#include <stdatomic.h>
#include "arm11/drivers/mcu.h"
#include "arm11/drivers/i2c.h"
#include "arm11/debug.h"
#include "arm11/drivers/interrupt.h"
#include "arm11/drivers/gpio.h"


static bool g_mcuNeedsIrqRead = false;
static u32 g_mcuIrqs = 0;
static struct
{
	u16 version;         // MCU firmware version ((MCU_REG_VERS_MAJOR - 0x10)<<8 | MCU_REG_VERS_MINOR).
	// TODO: Cache IRQ mask?
	u8 conType;          // Console type (MCU_REG_RAW_STATE[0]).
	u8 systemModel;      // System model (MCU_REG_RAW_STATE[9]).
	u8 earlyButtonsHeld; // Early button state (MCU_REG_RAW_STATE[18]);
} g_mcuRegCache;



static void mcuIrqHandler(UNUSED u32 intSource);

static bool updateRegisterCache(void)
{
	// Read major and minor version at once.
	u16 version;
	if(!MCU_readRegBuf(MCU_REG_VERS_MAJOR, (u8*)&version, sizeof(version))) return false;
	g_mcuRegCache.version = __builtin_bswap16(version - 0x10);

	u8 tmp[19];
	if(!MCU_readRegBuf(MCU_REG_RAW_STATE, tmp, sizeof(tmp))) return false;
	g_mcuRegCache.conType          = tmp[0];
	g_mcuRegCache.systemModel      = tmp[9];
	g_mcuRegCache.earlyButtonsHeld = tmp[18];

	return true;
}

void MCU_init(void)
{
	static bool mcuDriverInitialized = false;
	if(mcuDriverInitialized) return;
	mcuDriverInitialized = true;

	// Make sure I2C is initialized.
	I2C_init();

	// Configure GPIO for MCU IRQs.
	GPIO_config(GPIO_3_MCU, GPIO_IRQ_FALLING | GPIO_INPUT);

	// TODO: Clear alarm regs here like mcu module? Is this really needed?

	// Enable MCU IRQs.
	IRQ_registerIsr(IRQ_CTR_MCU, 14, 0, mcuIrqHandler);

	// Do first MCU IRQ read to clear all bits.
	// Discard IRQs we don't care about.
	atomic_store_explicit(&g_mcuNeedsIrqRead, true, memory_order_relaxed);
	(void)MCU_getIrqs(~DEFAULT_MCU_IRQ_MASK);

	// Set IRQ mask so we only get IRQs we are interested in.
	if(!MCU_setIrqMask(DEFAULT_MCU_IRQ_MASK)) panic();

	// Initialize register cache.
	if(!updateRegisterCache()) panic();
}

/*bool MCU_reboot(void)
{
	// TODO: GPIO bitmask 0x40000 handling.

	// Enters MCU update mode but since no firmware data
	// is incoming it will just reboot.
	if(!MCU_writeRegBuf(MCU_REG_FW_UPDATE, (const u8*)"jhl", 3)) return false;

	// We need to wait 1 second for the MCU to reboot.
	TIMER_sleepMs(1000);

	// TODO: Some state needs to be restored after a reboot.

	return true;
}*/

static void mcuIrqHandler(UNUSED u32 intSource)
{
	atomic_store_explicit(&g_mcuNeedsIrqRead, true, memory_order_relaxed);
}

// TODO: Rewrite using events (needs timeout support).
u32 MCU_getIrqs(u32 mask)
{
	u32 irqs = g_mcuIrqs;

	if(atomic_load_explicit(&g_mcuNeedsIrqRead, memory_order_relaxed))
	{
		atomic_store_explicit(&g_mcuNeedsIrqRead, false, memory_order_relaxed);

		u32 newIrqs;
		if(!MCU_readRegBuf(MCU_REG_IRQ, (u8*)&newIrqs, sizeof(newIrqs))) return 0;

		irqs |= newIrqs;
	}

	g_mcuIrqs = irqs & ~mask;

	return irqs & mask;
}

// TODO: Rewrite using events (needs timeout support).
u32 MCU_waitIrqs(u32 mask)
{
	u32 irqs;

	while((irqs = MCU_getIrqs(mask)) == 0u)
	{
		__wfi();
	}

	return irqs;
}


u16 MCU_getFirmwareVersion(void)
{
	return g_mcuRegCache.version;
}

u8 MCU_getStatus(void)
{
	return MCU_readReg(MCU_REG_STAT);
}

bool MCU_setStatus(u8 status)
{
	return MCU_writeReg(MCU_REG_STAT, status);
}

u8 MCU_getLcdVcomTop(void)
{
	return MCU_readReg(MCU_REG_LCD_VCOM_TOP);
}

bool MCU_setLcdVcomTop(u8 vcom)
{
	return MCU_writeReg(MCU_REG_LCD_VCOM_TOP, vcom);
}

u8 MCU_getLcdVcomBot(void)
{
	return MCU_readReg(MCU_REG_LCD_VCOM_BOT);
}

bool MCU_setLcdVcomBot(u8 vcom)
{
	return MCU_writeReg(MCU_REG_LCD_VCOM_BOT, vcom);
}

u8 MCU_get3dSliderPosition(void)
{
	return MCU_readReg(MCU_REG_3D_SLIDER);
}

u8 MCU_getVolumeSliderPosition(void)
{
	return MCU_readReg(MCU_REG_VOL_SLIDER);
}

s8 MCU_getBatteryTemperature(void)
{
	return (s8)MCU_readReg(MCU_REG_BATT_TEMP);
}

u8 MCU_getBatteryLevel(void)
{
	// The fractional part of the percentage is borderline useless.
	// It has varying accuracy and is stuck at 0 near 1%.
	return MCU_readReg(MCU_REG_BATT_LEVEL);
}

float MCU_getBatteryVoltage(void)
{
	return 0.02f * MCU_readReg(MCU_REG_BATT_VOLT);
}

u16 MCU_getExternalHardwareStatus(void)
{
	u16 status;

	// Read both status regs at once.
	if(!MCU_readRegBuf(MCU_REG_EX_HW_STAT2, (u8*)&status, sizeof(status))) status = 0;

	return __builtin_bswap16(status);
}

u32 MCU_getIrqMask(void)
{
	u32 mask;

	if(!MCU_readRegBuf(MCU_REG_IRQ_MASK, (u8*)&mask, sizeof(mask))) mask = 0;

	return mask;
}

bool MCU_setIrqMask(u32 mask)
{
	return MCU_writeRegBuf(MCU_REG_IRQ_MASK, (const u8*)&mask, sizeof(mask));
}

// TODO: MCU_setSystemPower()?

void MCU_powerOffSys(void)
{
	I2C_writeRegIntSafe(I2C_DEV_CTR_MCU, MCU_REG_SYS_POW, 1u);
}

void MCU_rebootSys(void)
{
	I2C_writeRegIntSafe(I2C_DEV_CTR_MCU, MCU_REG_SYS_POW, 1u<<2);
}

bool MCU_setTwlIrq(u8 bits)
{
	return MCU_writeReg(MCU_REG_TWL_IRQ, bits);
}

void MCU_setLcdPower(u8 bits)
{
	MCU_writeReg(MCU_REG_LCD_POW, bits);
}

u8 MCU_getPoweroffDelay(void)
{
	return MCU_readReg(MCU_REG_PWROFF_DELAY);
}

bool MCU_setPoweroffDelay(u8 delay)
{
	return MCU_writeReg(MCU_REG_PWROFF_DELAY, delay);
}

u8 MCU_getRegister0x25(void)
{
	return MCU_readReg(MCU_REG_UNK25);
}

bool MCU_setRegister0x25(u8 data)
{
	return MCU_writeReg(MCU_REG_UNK25, data);
}

u8 MCU_getRegister0x26(void)
{
	return MCU_readReg(MCU_REG_UNK26);
}

bool MCU_setRegister0x26(u8 data)
{
	return MCU_writeReg(MCU_REG_UNK26, data);
}

u8 MCU_getVolumeSliderPositionRaw(void)
{
	return MCU_readReg(MCU_REG_VOL_SLIDER_RAW);
}

bool MCU_setVolumeSliderPositionRaw(u8 data)
{
	return MCU_writeReg(MCU_REG_VOL_SLIDER_RAW, data);
}

u8 MCU_getLedMasterBrightness(void)
{
	return MCU_readReg(MCU_REG_LED_BRIGHTNESS);
}

bool MCU_setLedMasterBrightness(u8 brightness)
{
	return MCU_writeReg(MCU_REG_LED_BRIGHTNESS, brightness);
}

bool MCU_getPowerLedPattern(u8 pattern[5])
{
	return MCU_readRegBuf(MCU_REG_POWER_LED, pattern, 5);
}

bool MCU_setPowerLedPattern(const u8 pattern[5])
{
	return MCU_writeRegBuf(MCU_REG_POWER_LED, pattern, 5);
}

u8 MCU_getWifiLedState(void)
{
	return MCU_readReg(MCU_REG_WIFI_LED);
}

bool MCU_setWifiLedState(u8 state)
{
	return MCU_writeReg(MCU_REG_WIFI_LED, state);
}

u8 MCU_getCameraLedState(void)
{
	return MCU_readReg(MCU_REG_CAM_LED);
}

bool MCU_setCameraLedState(u8 state)
{
	return MCU_writeReg(MCU_REG_CAM_LED, state);
}

u8 MCU_get3dLedState(void)
{
	return MCU_readReg(MCU_REG_3D_LED);
}

bool MCU_set3dLedState(u8 state)
{
	return MCU_writeReg(MCU_REG_3D_LED, state);
}

bool MCU_setInfoLedPattern(const u8 pattern[100])
{
	return MCU_writeRegBuf(MCU_REG_INFO_LED, pattern, 100);
}

u8 MCU_getInfoLedStatus(void)
{
	return MCU_readReg(MCU_REG_INFO_LED_STAT);
}

bool MCU_getRtcTimeDate(RtcTimeDate *timeDate)
{
	// Read time and date at once.
	return MCU_readRegBuf(MCU_REG_RTC_S, (u8*)timeDate, sizeof(RtcTimeDate));
}

bool MCU_setRtcTimeDate(const RtcTimeDate *timeDate)
{
	// Write time and date at once.
	return MCU_writeRegBuf(MCU_REG_RTC_S, (const u8*)timeDate, sizeof(RtcTimeDate));
}

u8 MCU_getRtcErrorCorrection(void)
{
	return MCU_readReg(MCU_REG_RTC_ERR_CORR);
}

bool MCU_setRtcErrorCorrection(u8 correction)
{
	return MCU_writeReg(MCU_REG_RTC_ERR_CORR, correction);
}

bool MCU_getAlarmTimeDate(AlarmTimeDate *timeDate)
{
	// Read time and date at once.
	return MCU_readRegBuf(MCU_REG_ALARM_MIN, (u8*)timeDate, sizeof(AlarmTimeDate));
}

bool MCU_setAlarmTimeDate(const AlarmTimeDate *timeDate)
{
	// Write time and date at once.
	return MCU_writeRegBuf(MCU_REG_ALARM_MIN, (const u8*)timeDate, sizeof(AlarmTimeDate));
}

u16 MCU_getRtcTick(void)
{
	u16 tick;

	// Read both tick bytes at once.
	if(!MCU_readRegBuf(MCU_REG_RTC_TICK_LO, (u8*)&tick, sizeof(tick))) tick = 0;

	return tick;
}

bool MCU_setRegister0x3F(u8 data)
{
	return MCU_writeReg(MCU_REG_UNK3F, data);
}

AccCfg MCU_getAccelerometerConfig(void)
{
	return MCU_readReg(MCU_REG_ACC_CFG);
}

bool MCU_setAccelerometerConfig(AccCfg cfg)
{
	return MCU_writeReg(MCU_REG_ACC_CFG, cfg);
}

u8 MCU_readAccelerometerRegister(u8 reg)
{
	if(!MCU_writeReg(MCU_REG_ACC_READ_OFF, reg)) return 0;
	MCU_waitIrqs(MCU_IRQ_ACC_RW_DONE);

	return MCU_readReg(MCU_REG_ACC_DATA);
}

bool MCU_writeAccelerometerRegister(u8 reg, u8 data)
{
	const u16 regData = (u16)data<<8 | reg;

	// Write register and data at once.
	if(!MCU_writeRegBuf(MCU_REG_ACC_WRITE_OFF, (const u8*)&regData, sizeof(regData))) return false;
	MCU_waitIrqs(MCU_IRQ_ACC_RW_DONE); // TODO: Is this needed? mcu module doesn't wait for write.

	return true;
}

bool MCU_getAccelerometerSamples(AccData *samples)
{
	// Read all X/Y/Z sample bytes at once.
	const bool res = MCU_readRegBuf(MCU_REG_ACC_X_LO, (u8*)samples, sizeof(AccData));

	// Sample data is in the upper 12 bits.
	samples->x >>= 4;
	samples->y >>= 4;
	samples->z >>= 4;

	return res;
}

u32 MCU_getPedometerStepCount(void)
{
	u32 steps;

	// Read all step count bytes at once.
	if(!MCU_readRegBuf(MCU_REG_PM_COUNT_LO, (u8*)&steps, 3)) steps = 0;

	return steps & ~0xFF000000u; // Make sure byte 4 is 0.
}

bool MCU_setPedometerStepCount(u32 steps)
{
	// Write all step count bytes at once.
	return MCU_writeRegBuf(MCU_REG_PM_COUNT_LO, (u8*)&steps, 3);
}

// TODO: Reg 0x4E.

bool MCU_getPedometerHistory(u8 history[6 + 336])
{
	// Read all history bytes at once.
	const bool res = MCU_readRegBuf(MCU_REG_PM_HIST, history, 6 + 336);

	// TODO: BCD to decimal for the timestamps.

	return res;
}

u8 MCU_getRegister0x50(void)
{
	return MCU_readReg(MCU_REG_UNK50);
}

bool MCU_setRegister0x50(u8 data)
{
	return MCU_writeReg(MCU_REG_UNK50, data);
}

u8 MCU_getRegister0x51(void)
{
	return MCU_readReg(MCU_REG_UNK51);
}

bool MCU_setRegister0x51(u8 data)
{
	return MCU_writeReg(MCU_REG_UNK51, data);
}

bool MCU_getVolumeSliderCalibrationPoints(u8 minMax[2])
{
	// Read min and max at once.
	return MCU_readRegBuf(MCU_REG_VOL_SLIDER_MIN, minMax, 2);
}

bool MCU_setVolumeSliderCalibrationPoints(const u8 minMax[2])
{
	// Write min and max at once.
	return MCU_writeRegBuf(MCU_REG_VOL_SLIDER_MIN, minMax, 2);
}

bool MCU_getFreeRamData(u8 off, u8 *out, u8 size)
{
	if(!MCU_writeReg(MCU_REG_FREE_RAM_OFF, off)) return false;

	return MCU_readRegBuf(MCU_REG_FREE_RAM_DATA, out, size);
}

bool MCU_setFreeRamData(u8 off, const u8 *in, u8 size)
{
	if(!MCU_writeReg(MCU_REG_FREE_RAM_OFF, off)) return false;

	return MCU_writeRegBuf(MCU_REG_FREE_RAM_DATA, in, size);
}

u8 MCU_getConsoleType(void)
{
	return g_mcuRegCache.conType;
}

u8 MCU_getSystemModel(void)
{
	return g_mcuRegCache.systemModel;
}

u8 MCU_getEarlyButtonsHeld(void)
{
	return g_mcuRegCache.earlyButtonsHeld;
}


u8 MCU_readReg(McuReg reg)
{
	return I2C_readReg(I2C_DEV_CTR_MCU, reg);
}

bool MCU_writeReg(McuReg reg, u8 data)
{
	return I2C_writeReg(I2C_DEV_CTR_MCU, reg, data);
}

bool MCU_readRegBuf(McuReg reg, u8 *out, u32 size)
{
	return I2C_readRegBuf(I2C_DEV_CTR_MCU, reg, out, size);
}

bool MCU_writeRegBuf(McuReg reg, const u8 *const in, u32 size)
{
	return I2C_writeRegBuf(I2C_DEV_CTR_MCU, reg, in, size);
}
