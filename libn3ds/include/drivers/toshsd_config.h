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

#ifdef _3DS
#ifdef ARM9
#include "arm9/drivers/interrupt.h"
#include "arm9/drivers/cfg9.h"
#elif ARM11
#include "arm11/drivers/interrupt.h"
#endif // #ifdef ARM9
#elif TWL
#include <nds.h>
#endif // #ifdef _3DS


// Note on port numbers:
// To make things easier 2 ports are assigned to each controller.
// There are a maximum of 2 controllers mapped at the same time
// and 3 (on DSi 2) controllers in total.
// Also see toshsd.h.
//
// Examples:
// Port 0 is port 0 on controller 1, port 3 is port 1 on controller 2.
#ifdef _3DS
// This define determines whenever the SD slot is accessible on
// ARM9 or ARM11 when TOSHSD_CARD_PORT for ARM9 is set to 2.
#define TOSHSD_C2_MAP     (0u) // Controller 2 (physical 3) memory mapping. 0=ARM9 0x10007000 or 1=ARM11 0x10100000.

#ifdef ARM9
#define TOSHSD_CARD_PORT  (2u) // Can be on port 0 or 2. 0 always on ARM9.
#define TOSHSD_eMMC_PORT  (1u) // Port 1 only. Do not change.
#elif ARM11
#define TOSHSD_CARD_PORT  (2u) // Port 2 only. Do not change.
#define TOSHSD_eMMC_PORT  (3u) // Placeholder. Do not change. Not connected/accessible.
#endif // #ifdef ARM9

#elif TWL

#define TOSHSD_CARD_PORT  (0u)
#define TOSHSD_eMMC_PORT  (1u)
#endif // #ifdef _3DS



// Don't modify anything below!
#ifdef _3DS
#ifdef ARM9
#define TOSHSD_MAP_CONTROLLERS() \
{ \
	getCfg9Regs()->sdmmcctl = (TOSHSD_CARD_PORT == 2u ? SDMMCCTL_CARD_TOSHSD3_SEL : SDMMCCTL_CARD_TOSHSD1_SEL) | \
	                          (TOSHSD_C2_MAP == 1u ? SDMMCCTL_TOSHSD3_MAP11 : SDMMCCTL_TOSHSD3_MAP9) | \
	                          SDMMCCTL_UNK_BIT6 | SDMMCCTL_UNK_PWR_OFF; \
}

#define TOSHSD_UNMAP_CONTROLLERS()  {getCfg9Regs()->sdmmcctl = SDMMCCTL_UNK_BIT6 | SDMMCCTL_UNK_PWR_OFF | SDMMCCTL_CARD_PWR_OFF;}
#define TOSHSD_NUM_CONTROLLERS      (TOSHSD_C2_MAP == 0u ? 2u : 1u)
#define TOSHSD_IRQ_ID_CONTROLLER1   (IRQ_TOSHSD1)
#define TOSHSD_REGISTER_ISR(isr) \
{ \
	IRQ_registerIsr(IRQ_TOSHSD1, (isr)); \
	if(TOSHSD_NUM_CONTROLLERS == 2u) \
		IRQ_registerIsr(IRQ_TOSHSD3, (isr)); \
}
#elif ARM11
#define TOSHSD_MAP_CONTROLLERS()
#define TOSHSD_UNMAP_CONTROLLERS()
#define TOSHSD_NUM_CONTROLLERS      (TOSHSD_C2_MAP == 1u ? 2u : 1u)
#define TOSHSD_IRQ_ID_CONTROLLER1   (IRQ_TOSHSD2)
#define TOSHSD_REGISTER_ISR(isr) \
{ \
	IRQ_registerIsr(IRQ_TOSHSD2, 14, 0, (isr)); \
	if(TOSHSD_NUM_CONTROLLERS == 2u) \
		IRQ_registerIsr(IRQ_TOSHSD3, 14, 0, (isr)); \
}
#endif // #ifdef ARM9

#define TOSHSD_UNREGISTER_ISR() \
{ \
	IRQ_unregisterIsr(TOSHSD_IRQ_ID_CONTROLLER1); \
	if(TOSHSD_NUM_CONTROLLERS == 2u) \
		IRQ_unregisterIsr(IRQ_TOSHSD3); \
}

#elif TWL

#define TOSHSD_MAP_CONTROLLERS()
#define TOSHSD_UNMAP_CONTROLLERS()
#define TOSHSD_NUM_CONTROLLERS      (2u)
#define TOSHSD_IRQ_ID_CONTROLLER1   (IRQ_SDMMC)
#define TOSHSD_REGISTER_ISR(isr) \
{ \
	irqSetAUX(TOSHSD_IRQ_ID_CONTROLLER1, (isr)); \
	/*irqSetAUX(..., (isr)); // Missing IRQ for controller 2.*/ \
	irqEnableAUX(TOSHSD_IRQ_ID_CONTROLLER1); \
	/*irqEnableAUX(...); // Missing IRQ for controller 2.*/ \
}
#define TOSHSD_UNREGISTER_ISR() \
{ \
	irqClearAUX(TOSHSD_IRQ_ID_CONTROLLER1); \
	/*irqClearAUX(...); // Missing IRQ for controller 2.*/ \
}
#endif // #ifdef _3DS
