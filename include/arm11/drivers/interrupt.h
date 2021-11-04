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
#include "arm.h"


typedef enum
{
	IRQ_IPI0          =   0u,
	IRQ_IPI1          =   1u,
	IRQ_IPI2          =   2u,
	IRQ_IPI3          =   3u,
	IRQ_IPI4          =   4u,
	IRQ_IPI5          =   5u,
	IRQ_IPI6          =   6u,
	IRQ_IPI7          =   7u,
	IRQ_IPI8          =   8u,
	IRQ_IPI9          =   9u,
	IRQ_IPI10         =  10u,
	IRQ_IPI11         =  11u,
	IRQ_IPI12         =  12u,
	IRQ_IPI13         =  13u,
	IRQ_IPI14         =  14u,
	IRQ_IPI15         =  15u,
	IRQ_TIMER         =  29u, // MPCore timer.
	IRQ_WATCHDOG      =  30u, // MPCore watchdog.
	IRQ_SPI2          =  36u, // SPI bus 2 interrupt status update.
	IRQ_UART          =  37u, // New3DS-only UART?
	IRQ_PSC0          =  40u,
	IRQ_PSC1          =  41u,
	IRQ_PDC0          =  42u, // PDC0 topscreen H-/VBlank and errors.
	IRQ_PDC1          =  43u, // PDC1 bottom screen H-/VBlank and errors.
	IRQ_PPF           =  44u,
	IRQ_P3D           =  45u,
	IRQ_CDMA_EVENT0   =  48u, // Old3DS CDMA.
	IRQ_CDMA_EVENT1   =  49u, // Old3DS CDMA.
	IRQ_CDMA_EVENT2   =  50u, // Old3DS CDMA.
	IRQ_CDMA_EVENT3   =  51u, // Old3DS CDMA.
	IRQ_CDMA_EVENT4   =  52u, // Old3DS CDMA.
	IRQ_CDMA_EVENT5   =  53u, // Old3DS CDMA.
	IRQ_CDMA_EVENT6   =  54u, // Old3DS CDMA.
	IRQ_CDMA_EVENT7   =  55u, // Old3DS CDMA.
	IRQ_CDMA_EVENT8   =  56u, // Old3DS CDMA.
	IRQ_CDMA_FAULT    =  57u, // Old3DS CDMA.
	IRQ_CDMA2_EVENT   =  58u, // New3DS-only CDMA event 0-31.
	IRQ_CDMA2_FAULT   =  59u, // New3DS-only CDMA.
	IRQ_TOSHSD2       =  64u, // Toshsd 2 SDIO controller (WiFi).
	IRQ_TOSHSD2_IRQ   =  65u, // Toshsd 2 SDIO IRQ pin (WiFi).
	IRQ_TOSHSD3       =  66u, // Toshsd 3 SDIO controller.
	IRQ_TOSHSD3_IRQ   =  67u, // Toshsd 3 SDIO IRQ pin.
	IRQ_NTRCARD       =  68u, // NTRCARD controller.
	IRQ_L2B1          =  69u, // New3DS-only first L2B converter.
	IRQ_L2B2          =  70u, // New3DS-only second L2B converter.
	IRQ_CAM1          =  72u, // Camera 1 (DSi).
	IRQ_CAM2          =  73u, // Camera 2 (left eye).
	IRQ_DSP           =  74u,
	IRQ_Y2R1          =  75u,
	IRQ_LGYFB_BOT     =  76u, // Legacy framebuffer bottom screen.
	IRQ_LGYFB_TOP     =  77u, // Legacy framebuffer top screen.
	IRQ_Y2R2          =  78u, // New3DS-only.
	IRQ_G1            =  79u, // New3DS-only Hantro G1 decoder.
	IRQ_PXI_SYNC      =  80u,
	IRQ_PXI_SYNC2     =  81u,
	IRQ_PXI_NOT_FULL  =  82u,
	IRQ_PXI_NOT_EMPTY =  83u,
	IRQ_I2C1          =  84u,
	IRQ_I2C2          =  85u,
	IRQ_SPI3          =  86u, // SPI bus 3 interrupt status update.
	IRQ_SPI1          =  87u, // SPI bus 1 interrupt status update.
	IRQ_PDN           =  88u,
	IRQ_LGY_SLEEP     =  89u, // Triggers if legacy mode enters sleep.
	IRQ_MIC           =  90u,
	IRQ_HID_PADCNT    =  91u,
	IRQ_I2C3          =  92u,
	IRQ_DS_WIFI       =  95u,
	IRQ_GPIO_1_2_HIGH =  96u,
	IRQ_GPIO_1_2_LOW  =  98u,
	IRQ_GPIO_1_1      =  99u,
	IRQ_GPIO_2_0      = 100u,
	IRQ_GPIO_2_2      = 102u,
	IRQ_GPIO_3_0      = 104u,
	IRQ_GPIO_3_1      = 105u,
	IRQ_GPIO_3_2      = 106u,
	IRQ_GPIO_3_3      = 107u,
	IRQ_GPIO_3_4      = 108u,
	IRQ_GPIO_3_5      = 109u,
	IRQ_GPIO_3_6      = 110u,
	IRQ_GPIO_3_7      = 111u,
	IRQ_GPIO_3_8      = 112u,
	IRQ_GPIO_3_9      = 113u,
	IRQ_GPIO_3_10     = 114u,
	IRQ_GPIO_3_11     = 115u,
	IRQ_GAMECARD_OFF  = 116u, // Gamecard poweroff.
	IRQ_GAMECARD_INS  = 117u, // Gamecard inserted.
	IRQ_L2C           = 118u, // New3DS-only L2C-310 Level 2 Cache Controller.
	IRQ_UNK119        = 119u,
	IRQ_PERF_MONITOR0 = 120u, // Core 0 performance monitor. Triggers on any counter overflow.
	IRQ_PERF_MONITOR1 = 121u, // Core 1 performance monitor. Triggers on any counter overflow.
	IRQ_PERF_MONITOR2 = 122u, // Unconfirmed. Core 2 performance monitor. Triggers on any counter overflow.
	IRQ_PERF_MONITOR3 = 123u, // Unconfirmed. Core 3 performance monitor. Triggers on any counter overflow.

	// Aliases
	IRQ_SHELL_OPENED  = IRQ_GPIO_1_2_HIGH,
	IRQ_SHELL_CLOSED  = IRQ_GPIO_1_2_LOW,  // Triggers on GPIO_1_2 low.
	IRQ_TOUCHSCREEN   = IRQ_GPIO_1_1,      // Triggers on touchscreen pen down.
	IRQ_HEADPH_JACK   = IRQ_GPIO_2_0,      // Headphone jack. Triggers on both plugging in and out?
	IRQ_CTR_MCU       = IRQ_GPIO_3_9       // Various MCU events trigger this. See MCU interrupt mask.
} Interrupt;


// IRQ interrupt service routine type.
// intSource: bit 10-12 CPU source ID (0 except for interrupt ID 0-15),
// bit 0-9 interrupt ID
typedef void (*IrqIsr)(u32 intSource);



/**
 * @brief      Initializes the generic interrupt controller.
 */
void IRQ_init(void);

/**
 * @brief      Registers a interrupt service routine and enables the specified interrupt.
 *
 * @param[in]  id       The interrupt ID. Must be <128.
 * @param[in]  prio     The priority. 0 = highest, 14 = lowest, 15 = disabled.
 * @param[in]  cpuMask  The CPU mask. Each of the 4 bits stands for 1 core.
 *                      0 means current CPU.
 * @param[in]  isr      The interrupt service routine to call.
 */
void IRQ_registerIsr(Interrupt id, u8 prio, u8 cpuMask, IrqIsr isr);

/**
 * @brief      Reenables a previously disabled but registered interrupt.
 *
 * @param[in]  id    The interrupt ID. Must be <128.
 */
void IRQ_enable(Interrupt id);

/**
 * @brief      Disables a previously registered interrupt temporarily.
 *
 * @param[in]  id    The interrupt ID. Must be <128.
 */
void IRQ_disable(Interrupt id);

/**
 * @brief      Triggers a software interrupt for the specified CPUs.
 *
 * @param[in]  id       The interrupt ID. Must be <16.
 * @param[in]  cpuMask  The CPU mask. Each of the 4 bits stands for 1 core.
 */
void IRQ_softInterrupt(Interrupt id, u8 cpuMask);

/**
 * @brief      Sets the priority of an interrupt.
 *
 * @param[in]  id    The interrupt ID. Must be <128.
 * @param[in]  prio  The priority. 0 = highest, 14 = lowest, 15 = disabled
 */
void IRQ_setPriority(Interrupt id, u8 prio);

/**
 * @brief      Unregisters the interrupt service routine and disables the specified interrupt.
 *
 * @param[in]  id    The interrupt ID. Must be <128.
 */
void IRQ_unregisterIsr(Interrupt id);


#if !__thumb__
static inline u32 enterCriticalSection(void)
{
	const u32 tmp = __getCpsr();
	__cpsid(i);
	return tmp & PSR_I;
}

static inline void leaveCriticalSection(u32 oldState)
{
	__setCpsr_c((__getCpsr() & ~PSR_I) | oldState);
}
#endif
