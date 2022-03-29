#pragma once

/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2022 derrek, profi200
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



typedef enum
{
	MCU_REG_VERS_MAJOR      = 0x00u, // (ro) MCU firmware major version.
	MCU_REG_VERS_MINOR      = 0x01u, // (ro) MCU firmware minor version.
	MCU_REG_STAT            = 0x02u, // (rw) Reset status and TWL MCU emulation stuff.
	MCU_REG_LCD_VCOM_TOP    = 0x03u, // (rw) Top LCD VCOM ("flicker").
	MCU_REG_LCD_VCOM_BOT    = 0x04u, // (rw) Bottom LCD VCOM ("flicker").
	MCU_REG_FW_UPDATE       = 0x05u, // (rw) 0x05-0x07 Firmware update magic "jhl" is written here. If we stop the transfer after the magic the MCU will just reset.
	MCU_REG_3D_SLIDER       = 0x08u, // (ro) Raw 3D slider position (0-0x3F?).
	MCU_REG_VOL_SLIDER      = 0x09u, // (ro) Volume slider position (0-0x3F).
	MCU_REG_BATT_TEMP       = 0x0Au, // (ro) Battery temperature in celsius.
	MCU_REG_BATT_LEVEL      = 0x0Bu, // (ro) Battery percentage (0-100).
	MCU_REG_BATT_LEVEL_FRAC = 0x0Cu, // (ro) Battery percentage fractional part (percent/256).
	MCU_REG_BATT_VOLT       = 0x0Du, // (ro) Battery voltage in 20mV units.
	MCU_REG_EX_HW_STAT2     = 0x0Eu, // (ro) More hardware status bits.
	MCU_REG_EX_HW_STAT      = 0x0Fu, // (ro) Hardware status bits.
	MCU_REG_IRQ             = 0x10u, // (ro) 0x10-0x13 Interrupt status (clear on read).
	// 0x14 ro, 0x15-0x17 rw. All unused.
	MCU_REG_IRQ_MASK        = 0x18u, // (rw) 0x18-0x1B Interrupt mask (each bit 0=enabled, 1=disabled).
	// 0x1C-0x1F rw and unused.
	MCU_REG_SYS_POW         = 0x20u, // (wo) System power/reset control.
	MCU_REG_TWL_IRQ         = 0x21u, // (wo) Various TWL MCU status change signals.
	MCU_REG_LCD_POW         = 0x22u, // (wo) LCD power control.
	MCU_REG_RESTART         = 0x23u, // (wo) Stubbed (on retail?) MCU restart register.
	MCU_REG_PWROFF_DELAY    = 0x24u, // (rw) Force power off delay.
	MCU_REG_UNK25           = 0x25u, // (rw) Volume related? Volume value for override?
	MCU_REG_UNK26           = 0x26u, // (rw) Volume related? Bit 0: Enable/disable MCU reporting slider state to CODEC? Bit 1: Force? Bit 2: Mode? Bit 4: Trigger volume update from slider?
	MCU_REG_VOL_SLIDER_RAW  = 0x27u, // (rw) Volume slider raw ADC data (0-0x3F?).
	MCU_REG_LED_BRIGHTNESS  = 0x28u, // (rw) Master brightness of power/Wifi/3D (and camera?) LEDs.
	MCU_REG_POWER_LED       = 0x29u, // (rw) 5 bytes power LED state + pattern.
	MCU_REG_WIFI_LED        = 0x2Au, // (rw) WiFi LED state.
	MCU_REG_CAM_LED         = 0x2Bu, // (rw) Camera LED state.
	MCU_REG_3D_LED          = 0x2Cu, // (rw) 3D LED state.
	MCU_REG_INFO_LED        = 0x2Du, // (wo) 100 bytes notification/info LED pattern.
	MCU_REG_INFO_LED_STAT   = 0x2Eu, // (ro) Info LED status.
	// 0x2F wo with stubbed write handler.
	MCU_REG_RTC_S           = 0x30u, // (rw) RTC second.
	MCU_REG_RTC_MIN         = 0x31u, // (rw) RTC minute.
	MCU_REG_RTC_H           = 0x32u, // (rw) RTC hour.
	MCU_REG_RTC_DOW         = 0x33u, // (rw) RTC day of week (unused)?
	MCU_REG_RTC_D           = 0x34u, // (rw) RTC day.
	MCU_REG_RTC_MON         = 0x35u, // (rw) RTC month.
	MCU_REG_RTC_Y           = 0x36u, // (rw) RTC year.
	MCU_REG_RTC_ERR_CORR    = 0x37u, // (rw) RTC Watch error correction.
	MCU_REG_ALARM_MIN       = 0x38u, // (rw) Alarm minute.
	MCU_REG_ALARM_H         = 0x39u, // (rw) Alarm hour.
	MCU_REG_ALARM_D         = 0x3Au, // (rw) Alarm day.
	MCU_REG_ALARM_MON       = 0x3Bu, // (rw) Alarm month.
	MCU_REG_ALARM_Y         = 0x3Cu, // (rw) Alarm year.
	MCU_REG_RTC_TICK_LO     = 0x3Du, // (ro) RTC tick counter LSB in 32768 Hz units.
	MCU_REG_RTC_TICK_HI     = 0x3Eu, // (ro) RTC tick counter MSB.
	MCU_REG_UNK3F           = 0x3Fu, // (wo) Unknown state/control reg.
	MCU_REG_ACC_CFG         = 0x40u, // (rw) Accelerometer configuration register.
	MCU_REG_ACC_READ_OFF    = 0x41u, // (rw) Accelerometer I2C register offset for read (via MCU reg 0x44).
	// 0x42 rw unused.
	MCU_REG_ACC_WRITE_OFF   = 0x43u, // (rw) Accelerometer I2C register offset for write (via MCU reg 0x44).
	MCU_REG_ACC_DATA        = 0x44u, // (rw) Accelerometer I2C register data.
	MCU_REG_ACC_X_LO        = 0x45u, // (ro) Accelerometer X sample data LSB.
	MCU_REG_ACC_X_HI        = 0x46u, // (ro) Accelerometer X sample data MSB.
	MCU_REG_ACC_Y_LO        = 0x47u, // (ro) Accelerometer Y sample data LSB.
	MCU_REG_ACC_Y_HI        = 0x48u, // (ro) Accelerometer Y sample data MSB.
	MCU_REG_ACC_Z_LO        = 0x49u, // (ro) Accelerometer Z sample data LSB.
	MCU_REG_ACC_Z_HI        = 0x4Au, // (ro) Accelerometer Z sample data MSB.
	MCU_REG_PM_COUNT_LO     = 0x4Bu, // (rw) Pedometer step count LSB.
	MCU_REG_PM_COUNT_MI     = 0x4Cu, // (rw) Pedometer step count middle byte.
	MCU_REG_PM_COUNT_HI     = 0x4Du, // (rw) Pedometer step count MSB.
	MCU_REG_PM_HIST_STAT    = 0x4Eu, // (rw) Pedometer history state. TODO: Better name.
	MCU_REG_PM_HIST         = 0x4Fu, // (ro) 6 + 336 bytes pedometer history data.
	MCU_REG_UNK50           = 0x50u, // (rw)
	MCU_REG_UNK51           = 0x51u, // (rw)
	// 0x52-0x57 rw unknown/unused.
	MCU_REG_VOL_SLIDER_MIN  = 0x58u, // (rw) Volume slider minimum calibration point.
	MCU_REG_VOL_SLIDER_MAX  = 0x59u, // (rw) Volume slider maximum calibration point.
	// 0x5A rw/ro depending on firmware version. Unused.
	// 0x5B-0x5F unused.
	MCU_REG_FREE_RAM_OFF    = 0x60u, // (rw) Free RAM offset for MCU register 0x61.
	MCU_REG_FREE_RAM_DATA   = 0x61u, // (rw) Free RAM data register.
	// 0x62-0x7E unused.
	MCU_REG_RAW_STATE       = 0x7Fu  // (ro) 19 bytes of various raw state.
	// 0x80-0xFF unused.
} McuReg;


// MCU_REG_IRQ and MCU_REG_IRQ_MASK.
#define MCU_IRQ_POWER_PRESS        (1u)     // Power button pressed for 200 ms.
#define MCU_IRQ_POWER_HELD         (1u<<1)  // Power button held for 3 seconds.
#define MCU_IRQ_HOME_PRESS         (1u<<2)  // HOME button pressed for 40 ms.
#define MCU_IRQ_HOME_RELEASE       (1u<<3)  // HOME button released.
#define MCU_IRQ_WIFI_PRESS         (1u<<4)  // WiFi button pressed for 40 ms.
#define MCU_IRQ_SHELL_CLOSE        (1u<<5)  // Shell has been closed.
#define MCU_IRQ_SHELL_OPEN         (1u<<6)  // Shell has been opened.
#define MCU_IRQ_WATCHDOG           (1u<<7)  // MCU has been reset by the watchdog.
#define MCU_IRQ_CHARGER_UNPLUG     (1u<<8)  // Charger has been unplugged.
#define MCU_IRQ_CHARGER_PLUG       (1u<<9)  // Charger has been plugged in.
#define MCU_IRQ_RTC_ALARM          (1u<<10) // RTC alarm.
#define MCU_IRQ_ACC_RW_DONE        (1u<<11) // Accelerometer I2C read/write done.
#define MCU_IRQ_ACC_DATA_READY     (1u<<12) // Accelerometer X/Y/Z sample data ready.
#define MCU_IRQ_LOW_BATT           (1u<<13) // Low battery warning IRQ triggered at 10, 5 and 0%. TODO: gbatek says 11, 6 and 1%.
#define MCU_IRQ_BATT_CHARGE_STOP   (1u<<14) // Battery charging stopped.
#define MCU_IRQ_BATT_CHARGE_START  (1u<<15) // Battery charging started.
#define MCU_IRQ_TWL_RESET          (1u<<16) // DS powerman register 0x10 bit 0 = 1 or TWL MCU register 0x11 = 1 (reset).
#define MCU_IRQ_TWL_PWROFF         (1u<<17) // DS powerman register 0x10 bit 6 = 1. Poweroff request?
#define MCU_IRQ_TWL_BOT_BL_OFF     (1u<<18) // DS powerman register 0x10 bit 2 = 0. Bottom LCD backlight off request?
#define MCU_IRQ_TWL_BOT_BL_ON      (1u<<19) // DS powerman register 0x10 bit 2 = 1. Bottom LCD backlight on request?
#define MCU_IRQ_TWL_TOP_BL_OFF     (1u<<20) // DS powerman register 0x10 bit 3 = 0. Top LCD backlight off request?
#define MCU_IRQ_TWL_TOP_BL_ON      (1u<<21) // DS powerman register 0x10 bit 3 = 1. Top LCD backlight on request?
#define MCU_IRQ_VOL_SLIDER_CHANGE  (1u<<22) // Volume slider position changed.
#define MCU_IRQ_TWL_MCU_VER_READ   (1u<<23) // TWL MCU version register (0x00) read.
#define MCU_IRQ_LCD_POWER_OFF      (1u<<24) // LCDs have been powered off.
#define MCU_IRQ_LCD_POWER_ON       (1u<<25) // LCDs have been powered on.
#define MCU_IRQ_BOT_BL_OFF         (1u<<26) // Bottom LCD backlight has been powered off.
#define MCU_IRQ_BOT_BL_ON          (1u<<27) // Bottom LCD backlight has been powered on.
#define MCU_IRQ_TOP_BL_OFF         (1u<<28) // Top LCD backlight has been powered off.
#define MCU_IRQ_TOP_BL_ON          (1u<<29) // Top LCD backlight has been powered on.

// MCU_REG_ACC_CFG
typedef enum
{
	ACC_CFG_ALL_OFF       = 0u, // Accelerometer and pedometer off.
	ACC_CFG_ACC_ON_PM_OFF = 1u, // Accelerometer on and pedometer off.
	ACC_CFG_ACC_OFF_PM_ON = 2u, // Accelerometer off and pedometer on.
	ACC_CFG_ACC_ON_PM_ON  = 3u  // Accelerometer on and pedometer on.
} AccCfg;
