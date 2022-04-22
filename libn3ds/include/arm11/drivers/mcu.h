#pragma once

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

#include "types.h"
#include "arm11/drivers/mcu_regmap.h"


#define DEFAULT_MCU_IRQ_MASK  (~(MCU_IRQ_TOP_BL_ON | MCU_IRQ_TOP_BL_OFF | MCU_IRQ_BOT_BL_ON | \
                                 MCU_IRQ_BOT_BL_OFF | MCU_IRQ_LCD_POWER_ON | MCU_IRQ_LCD_POWER_OFF | \
                                 MCU_IRQ_VOL_SLIDER_CHANGE | MCU_IRQ_BATT_CHARGE_START | \
                                 MCU_IRQ_BATT_CHARGE_STOP | MCU_IRQ_ACC_DATA_READY | MCU_IRQ_ACC_RW_DONE | \
                                 MCU_IRQ_WATCHDOG | MCU_IRQ_SHELL_OPEN | MCU_IRQ_SHELL_CLOSE | \
                                 MCU_IRQ_WIFI_PRESS | MCU_IRQ_HOME_RELEASE | MCU_IRQ_HOME_PRESS | \
                                 MCU_IRQ_POWER_HELD | MCU_IRQ_POWER_PRESS))


/*typedef struct
{
	// TODO
} PowerLedPattern;*/

/*typedef struct
{
	// TODO
} InfoLedPattern;*/

typedef struct
{
	u8 s;   // Second.
	u8 min; // Minute.
	u8 h;   // Hour.
	u8 dow; // Unused day of week.
	u8 d;   // Day.
	u8 mon; // Month.
	u8 y;   // Year.
} RtcTimeDate;

typedef struct
{
	u8 min; // Minute.
	u8 h;   // Hour.
	u8 d;   // Day.
	u8 mon; // Month.
	u8 y;   // Year.
} AlarmTimeDate;

typedef struct
{
	s16 x;
	s16 y;
	s16 z;
} AccData;

/*typedef struct
{
	// 6 bytes timestamps.
	// 336 bytes step counts.
} PedometerHistory;*/



// TODO: For most functions check if the return value on error makes sense.

/**
 * @brief      Initializes the MCU driver.
 */
void MCU_init(void);

/**
 * @brief      Reboots the MCU.
 *
 * @return     Returns true on success and false on failure.
 */
//bool MCU_reboot(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  mask  The mask
 *
 * @return     { description_of_the_return_value }
 */
u32 MCU_getIrqs(u32 mask);

/**
 * @brief      { function_description }
 *
 * @param[in]  mask  The mask
 *
 * @return     { description_of_the_return_value }
 */
u32 MCU_waitIrqs(u32 mask);


/**
 * @brief      Reads the MCU firmware version.
 *
 * @return     Returns the MCU firmware version.
 */
u16 MCU_getFirmwareVersion(void);

/**
 * @brief      Reads MCU status bits.
 *
 * @return     Returns the MCU status bits.
 */
u8 MCU_getStatus(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  status  The status
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setStatus(u8 status);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getLcdVcomTop(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  vcom  The vcom
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setLcdVcomTop(u8 vcom);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getLcdVcomBot(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  vcom  The vcom
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setLcdVcomBot(u8 vcom);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_get3dSliderPosition(void);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getVolumeSliderPosition(void);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
s8 MCU_getBatteryTemperature(void);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getBatteryLevel(void);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
float MCU_getBatteryVoltage(void);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u16 MCU_getExternalHardwareStatus(void);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u32 MCU_getIrqMask(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  mask  The mask
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setIrqMask(u32 mask);

// TODO: MCU_setSystemPower()?

void MCU_powerOffSys(void);

void MCU_rebootSys(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  bits  The bits
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setTwlIrq(u8 bits);

/**
 * @brief      { function_description }
 *
 * @param[in]  bits  The bits
 */
void MCU_setLcdPower(u8 bits);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getPoweroffDelay(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  delay  The delay
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setPoweroffDelay(u8 delay);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getRegister0x25(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  data  The data
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setRegister0x25(u8 data);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getRegister0x26(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  data  The data
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setRegister0x26(u8 data);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getVolumeSliderPositionRaw(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  data  The data
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setVolumeSliderPositionRaw(u8 data);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getLedMasterBrightness(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  brightness  The brightness
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setLedMasterBrightness(u8 brightness);

/**
 * @brief      { function_description }
 *
 * @param      pattern  The pattern
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_getPowerLedPattern(u8 pattern[5]); // TODO: Struct.

/**
 * @brief      { function_description }
 *
 * @param[in]  pattern  The pattern
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setPowerLedPattern(const u8 pattern[5]); // TODO: Struct.

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getWifiLedState(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  state  The state
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setWifiLedState(u8 state);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getCameraLedState(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  state  The state
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setCameraLedState(u8 state);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_get3dLedState(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  state  The state
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_set3dLedState(u8 state);

/**
 * @brief      { function_description }
 *
 * @param[in]  pattern  The pattern
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setInfoLedPattern(const u8 pattern[100]); // TODO: Struct.

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getInfoLedStatus(void);

/**
 * @brief      { function_description }
 *
 * @param      timeDate  The time date
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_getRtcTimeDate(RtcTimeDate *timeDate);

/**
 * @brief      { function_description }
 *
 * @param[in]  timeDate  The time date
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setRtcTimeDate(const RtcTimeDate *timeDate);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getRtcErrorCorrection(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  correction  The correction
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setRtcErrorCorrection(u8 correction);

/**
 * @brief      { function_description }
 *
 * @param      timeDate  The time date
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_getAlarmTimeDate(AlarmTimeDate *timeDate);

/**
 * @brief      { function_description }
 *
 * @param[in]  timeDate  The time date
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setAlarmTimeDate(const AlarmTimeDate *timeDate);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u16 MCU_getRtcTick(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  data  The data
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setRegister0x3F(u8 data);

/**
 * @brief      { function_description }
 *
 * @return     The acc configuration.
 */
AccCfg MCU_getAccelerometerConfig(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  cfg   The configuration
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setAccelerometerConfig(AccCfg cfg);

/**
 * @brief      { function_description }
 *
 * @param[in]  reg   The register
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_readAccelerometerRegister(u8 reg);

/**
 * @brief      { function_description }
 *
 * @param[in]  reg   The register
 * @param[in]  data  The data
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_writeAccelerometerRegister(u8 reg, u8 data);

/**
 * @brief      { function_description }
 *
 * @param      samples  The samples
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_getAccelerometerSamples(AccData *samples);

/**
 * @brief      { function_description }
 *
 * @param[in]  steps  The steps
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setPedometerStepCount(u32 steps);

// TODO: Reg 0x4E.

/**
 * @brief      { function_description }
 *
 * @param      history  The history
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_getPedometerHistory(u8 history[6 + 336]); // TODO: Struct.

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getRegister0x50(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  data  The data
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setRegister0x50(u8 data);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getRegister0x51(void);

/**
 * @brief      { function_description }
 *
 * @param[in]  data  The data
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setRegister0x51(u8 data);

/**
 * @brief      { function_description }
 *
 * @param      minMax  The minimum maximum
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_getVolumeSliderCalibrationPoints(u8 minMax[2]);

/**
 * @brief      { function_description }
 *
 * @param[in]  minMax  The minimum maximum
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setVolumeSliderCalibrationPoints(const u8 minMax[2]);

/**
 * @brief      { function_description }
 *
 * @param[in]  off   Off
 * @param      out   The out
 * @param[in]  size  The size
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_getFreeRamData(u8 off, u8 *out, u8 size);

/**
 * @brief      { function_description }
 *
 * @param[in]  off   Off
 * @param[in]  in    { parameter_description }
 * @param[in]  size  The size
 *
 * @return     { description_of_the_return_value }
 */
bool MCU_setFreeRamData(u8 off, const u8 *in, u8 size);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getConsoleType(void);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getSystemModel(void);

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
u8 MCU_getEarlyButtonsHeld(void);


/**
 * @brief      Reads a single MCU register.
 *
 * @param[in]  reg   The MCU register to read.
 *
 * @return     The MCU register data.
 */
u8 MCU_readReg(McuReg reg);

/**
 * @brief      Writes a single MCU register.
 *
 * @param[in]  reg   The MCU register to write.
 * @param[in]  data  The data to write.
 *
 * @return     Returns true on success and false on failure.
 */
bool MCU_writeReg(McuReg reg, u8 data);

/**
 * @brief      Reads multiple MCU registers or buffers behind registers.
 *
 * @param[in]  reg   The MCU register(s) to read.
 * @param      out   The output data pointer.
 * @param[in]  size  The output buffer size.
 *
 * @return     Returns true on success and false on failure.
 */
bool MCU_readRegBuf(McuReg reg, u8 *out, u32 size);

/**
 * @brief      Writes multiple MCU registers or buffers behind registers.
 *
 * @param[in]  reg   The MCU register(s) to write.
 * @param[in]  in    The input data pointer.
 * @param[in]  size  The input data size.
 *
 * @return     Returns true on success and false on failure.
 */
bool MCU_writeRegBuf(McuReg reg, const u8 *const in, u32 size);
