#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
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
#include "mem_map.h"
#include "arm.h"


#define CPU_II_REGS_BASE     (MPCORE_PRIV_REG_BASE + 0x100)
#define REG_CPU_II_CNT       *((vu32*)(CPU_II_REGS_BASE + 0x00))
#define REG_CPU_II_MASK      *((vu32*)(CPU_II_REGS_BASE + 0x04))
#define REG_CPU_II_BIN_POI   *((vu32*)(CPU_II_REGS_BASE + 0x08))
#define REG_CPU_II_AKN       *((vu32*)(CPU_II_REGS_BASE + 0x0C))
#define REG_CPU_II_EOI       *((vu32*)(CPU_II_REGS_BASE + 0x10))
#define REG_CPU_II_RUN_PRIO  *((vu32*)(CPU_II_REGS_BASE + 0x14))
#define REG_CPU_II_HIGH_PEN  *((vu32*)(CPU_II_REGS_BASE + 0x18))

#define GID_REGS_BASE        (MPCORE_PRIV_REG_BASE + 0x1000)
#define REG_GID_CNT          *((vu32*)(GID_REGS_BASE + 0x000))
#define REG_GID_CONTR_TYPE   *((vu32*)(GID_REGS_BASE + 0x004))
#define REGs_GID_ENA_SET      ((vu32*)(GID_REGS_BASE + 0x100))
#define REGs_GID_ENA_CLR      ((vu32*)(GID_REGS_BASE + 0x180))
#define REGs_GID_PEN_SET      ((vu32*)(GID_REGS_BASE + 0x200))
#define REGs_GID_PEN_CLR      ((vu32*)(GID_REGS_BASE + 0x280))
#define REGs_GID_ACTIVE_BIT   ((vu32*)(GID_REGS_BASE + 0x300))
#define REGs_GID_IPRIO        ((vu32*)(GID_REGS_BASE + 0x400))
#define REGs_GID_ITARG        ((vu32*)(GID_REGS_BASE + 0x800))
#define REGs_GID_ICONF        ((vu32*)(GID_REGS_BASE + 0xC00))
#define REGs_GID_LINE_LEV     ((vu32*)(GID_REGS_BASE + 0xD00))
#define REG_GID_SW_INT       *((vu32*)(GID_REGS_BASE + 0xF00))
#define REG_GID_PERI_INFO0   *((vu32*)(GID_REGS_BASE + 0xFE0))
#define REG_GID_PERI_INFO1   *((vu32*)(GID_REGS_BASE + 0xFE4))
#define REG_GID_PERI_INFO2   *((vu32*)(GID_REGS_BASE + 0xFE8))
#define REG_GID_PERI_INFO3   *((vu32*)(GID_REGS_BASE + 0xFEC))
#define REG_GID_PRIME_CELL0  *((vu32*)(GID_REGS_BASE + 0xFF0))
#define REG_GID_PRIME_CELL1  *((vu32*)(GID_REGS_BASE + 0xFF4))
#define REG_GID_PRIME_CELL2  *((vu32*)(GID_REGS_BASE + 0xFF8))
#define REG_GID_PRIME_CELL3  *((vu32*)(GID_REGS_BASE + 0xFFC))


typedef enum
{
	IRQ_MPCORE_SW0    =   0u,
	IRQ_MPCORE_SW1    =   1u,
	IRQ_MPCORE_SW2    =   2u,
	IRQ_MPCORE_SW3    =   3u,
	IRQ_MPCORE_SW4    =   4u,
	IRQ_MPCORE_SW5    =   5u,
	IRQ_MPCORE_SW6    =   6u,
	IRQ_MPCORE_SW7    =   7u,
	IRQ_MPCORE_SW8    =   8u,
	IRQ_MPCORE_SW9    =   9u,
	IRQ_MPCORE_SW10   =  10u,
	IRQ_MPCORE_SW11   =  11u,
	IRQ_MPCORE_SW12   =  12u,
	IRQ_MPCORE_SW13   =  13u,
	IRQ_MPCORE_SW14   =  14u,
	IRQ_MPCORE_SW15   =  15u,
	IRQ_TIMER         =  29u, // MPCore timer
	IRQ_WATCHDOG      =  30u, // MPCore watchdog
	IRQ_SPI2          =  36u, // SPI bus 2 interrupt status update
	IRQ_PSC0          =  40u,
	IRQ_PSC1          =  41u,
	IRQ_PDC0          =  42u, // aka VBlank0
	IRQ_PDC1          =  43u, // aka VBlank1
	IRQ_PPF           =  44u,
	IRQ_P3D           =  45u,
	IRQ_CDMA_EVENT0   =  48u, // Old 3DS CDMA
	IRQ_CDMA_EVENT1   =  49u, // Old 3DS CDMA
	IRQ_CDMA_EVENT2   =  50u, // Old 3DS CDMA
	IRQ_CDMA_EVENT3   =  51u, // Old 3DS CDMA
	IRQ_CDMA_EVENT4   =  52u, // Old 3DS CDMA
	IRQ_CDMA_EVENT5   =  53u, // Old 3DS CDMA
	IRQ_CDMA_EVENT6   =  54u, // Old 3DS CDMA
	IRQ_CDMA_EVENT7   =  55u, // Old 3DS CDMA
	IRQ_CDMA_EVENT8   =  56u, // Old 3DS CDMA
	IRQ_CDMA_FAULT    =  57u, // Old 3DS CDMA
	IRQ_CDMA2_EVENT   =  58u, // New 3DS CDMA
	IRQ_CDMA2_FAULT   =  59u, // New 3DS CDMA
	IRQ_SDIO          =  64u, // SDIO controller (WiFi)
	IRQ_SDIO_IRQ      =  65u, // SDIO IRQ pin (WiFi)
	IRQ_CAM0          =  72u, // Camera 0 (DSi)
	IRQ_CAM1          =  73u, // Camera 1 (left eye)
	IRQ_LGYFB_BOT     =  76u, // Legacy framebuffer bottom screen
	IRQ_LGYFB_TOP     =  77u, // Legacy framebuffer top screen
	IRQ_PXI_SYNC      =  80u,
	IRQ_PXI_SYNC2     =  81u,
	IRQ_PXI_NOT_FULL  =  82u,
	IRQ_PXI_NOT_EMPTY =  83u,
	IRQ_I2C1          =  84u,
	IRQ_I2C2          =  85u,
	IRQ_SPI3          =  86u, // SPI bus 3 interrupt status update
	IRQ_SPI1          =  87u, // SPI bus 1 interrupt status update
	IRQ_PDN           =  88u,
	IRQ_LGY_SLEEP     =  89u, // Triggers if legacy mode enters sleep.
	IRQ_HID_PADCNT    =  91u,
	IRQ_I2C3          =  92u,
	IRQ_GPIO_1_2      =  96u,
	IRQ_SHELL_CLOSED  =  98u, // GPIO_1_0?
	IRQ_GPIO_1_1      =  99u,
	IRQ_GPIO_2_0      = 100u,
	IRQ_GPIO_2_1      = 102u,
	IRQ_GPIO_4_0      = 104u,
	IRQ_GPIO_4_1      = 105u,
	IRQ_GPIO_4_2      = 106u,
	IRQ_GPIO_4_3      = 107u,
	IRQ_GPIO_4_4      = 108u,
	IRQ_GPIO_4_5      = 109u,
	IRQ_GPIO_4_6      = 110u,
	IRQ_GPIO_4_7      = 111u,
	IRQ_GPIO_4_8      = 112u,
	IRQ_GPIO_4_9      = 113u,
	IRQ_GPIO_4_10     = 114u,
	IRQ_GPIO_4_11     = 115u,
	IRQ_GAMECARD      = 117u, // Gamecard inserted
	IRQ_PERF_MONITOR0 = 120u, // Core 0 performance monitor. Triggers on any counter overflow
	IRQ_PERF_MONITOR1 = 121u, // Core 1 performance monitor. Triggers on any counter overflow
	IRQ_PERF_MONITOR2 = 122u, // Unconfirmed. Core 2 performance monitor. Triggers on any counter overflow
	IRQ_PERF_MONITOR3 = 123u, // Unconfirmed. Core 3 performance monitor. Triggers on any counter overflow

	// Aliases
	IRQ_SHELL_OPENED  = IRQ_GPIO_1_2,
	IRQ_TOUCHSCREEN   = IRQ_GPIO_1_1, // Triggers on touchscreen pen down.
	IRQ_HEADPH_JACK   = IRQ_GPIO_2_0, // Headphone jack. Triggers on both plugging in and out?
	IRQ_CTR_MCU       = IRQ_GPIO_4_9  // Various MCU events trigger this. See MCU interrupt mask.
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
 * @param[in]  id             The interrupt ID. Must be <128.
 * @param[in]  prio           The priority. 0 = highest, 14 = lowest, 15 = disabled
 * @param[in]  cpuMask        The CPU mask. Each of the 4 bits stands for 1 core. 0 means current CPU.
 * @param[in]  edgeTriggered  Set to true to make the interrupt edge triggered. false is level triggered.
 * @param[in]  isr            The interrupt service routine to call.
 */
void IRQ_registerIsr(Interrupt id, u8 prio, u8 cpuMask, bool edgeTriggered, IrqIsr isr);

/**
 * @brief      Unregisters the interrupt service routine and disables the specified interrupt.
 *
 * @param[in]  id    The interrupt ID. Must be <128.
 */
void IRQ_unregisterIsr(Interrupt id);

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
 * @brief      Sets the priority of an interrupt.
 *
 * @param[in]  id    The interrupt ID. Must be <128.
 * @param[in]  prio  The priority. 0 = highest, 14 = lowest, 15 = disabled
 */
void IRQ_setPriority(Interrupt id, u8 prio);

/**
 * @brief      Triggers a software interrupt for the specified CPUs.
 *
 * @param[in]  id       The interrupt ID. Must be <16.
 * @param[in]  cpuMask  The CPU mask. Each of the 4 bits stands for 1 core.
 */
void IRQ_softwareInterrupt(Interrupt id, u8 cpuMask);


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
