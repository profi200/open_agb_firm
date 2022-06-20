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
#include "arm11/drivers/gic.h"
#include "arm11/drivers/interrupt.h"
#include "mmio.h"
#include "arm.h"
#include "arm11/drivers/cfg11.h"


// Level high active keeps firing until acknowledged (on the periphal side).
// Rising edge sensitive only fires on rising edges.
#define ICONF_RSVD   (0u) // Unused/reserved.
#define ICONF_L_NN   (0u) // Level high active, N-N software model.
#define ICONF_L_1N   (1u) // Level high active, 1-N software model.
#define ICONF_E_NN   (2u) // Rising edge sinsitive, N-N software model.
#define ICONF_E_1N   (3u) // Rising edge sinsitive, 1-N software model.
#define MAKE_ICONF(c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) \
                  ((c15)<<30 | (c14)<<28 | (c13)<<26 | (c12)<<24 | (c11)<<22 | \
                   (c10)<<20 | (c9)<<18 | (c8)<<16 | (c7)<<14 | (c6)<<12 | \
                   (c5)<<10 | (c4)<<8 | (c3)<<6 | (c2)<<4 | (c1)<<2 | (c0))


// First 32 interrupts are private to each core (4 * 32).
// 96 external interrupts (total 128).
IrqIsr g_irqIsrTable[224] = {0};



// Per core interrupts.
static void configPrivateInterrupts(GicDist *const gicDist)
{
	// Disable first 32 interrupts.
	// Interrupts 0-15 cant be disabled.
	gicDist->enable_clear[0] = 0xFFFFFFFFu;

	// Set first 32 interrupts to inactive state.
	// Interrupt 0-15 can't be set to inactive.
	gicDist->pending_clear[0] = 0xFFFFFFFFu;

	// Set first 32 interrupts to lowest priority.
	iomemset(gicDist->pri, 0xF0F0F0F0u, 8 * 4);

	// Interrupt target 0-31 can't be changed.

	// Kernel11 config.
	// Interrupts 0-15.
	gicDist->config[0] = MAKE_ICONF(ICONF_E_NN, ICONF_E_NN, ICONF_E_NN, ICONF_E_NN,  // 0-3
	                                ICONF_E_NN, ICONF_E_NN, ICONF_E_NN, ICONF_E_NN,  // 4-7
	                                ICONF_E_NN, ICONF_E_NN, ICONF_E_NN, ICONF_E_NN,  // 8-11
	                                ICONF_E_NN, ICONF_E_NN, ICONF_E_NN, ICONF_E_NN); // 12-15
	// Interrupts 16-31.
	gicDist->config[1] = MAKE_ICONF(ICONF_RSVD, ICONF_RSVD, ICONF_RSVD, ICONF_RSVD,  // 16-19
	                                ICONF_RSVD, ICONF_RSVD, ICONF_RSVD, ICONF_RSVD,  // 20-23
	                                ICONF_RSVD, ICONF_RSVD, ICONF_RSVD, ICONF_RSVD,  // 24-27
	                                ICONF_RSVD, ICONF_E_NN, ICONF_E_NN, ICONF_RSVD); // 28-31
}

static void configExternalInterrupts(GicDist *const gicDist)
{
	// Kernel11 config with slight modifications.
	static const u32 configTable[6] =
	{
		// Interrupts 32-47.
		MAKE_ICONF(ICONF_L_1N, ICONF_L_1N, ICONF_L_1N, ICONF_L_1N,  // 32-35
		           ICONF_E_1N, ICONF_L_1N, ICONF_RSVD, ICONF_RSVD,  // 36-39
		           /*ICONF_L_1N, ICONF_L_1N, ICONF_L_1N, ICONF_L_1N,  // 40-43
		           ICONF_L_1N, ICONF_L_1N, ICONF_RSVD, ICONF_RSVD),*/ // 44-47
		           ICONF_E_1N, ICONF_E_1N, ICONF_E_1N, ICONF_E_1N,  // 40-43 Modified to prevent IRQ storms.
		           ICONF_E_1N, ICONF_E_1N, ICONF_RSVD, ICONF_RSVD), // 44-47 Modified to prevent IRQ storms.
		// Interrupts 48-63.
		MAKE_ICONF(ICONF_L_1N, ICONF_L_1N, ICONF_L_1N, ICONF_L_1N,  // 48-51
		           ICONF_L_1N, ICONF_L_1N, ICONF_L_1N, ICONF_L_1N,  // 52-55
		           ICONF_L_1N, ICONF_L_1N, ICONF_L_1N, ICONF_L_1N,  // 56-59
		           ICONF_RSVD, ICONF_RSVD, ICONF_RSVD, ICONF_RSVD), // 60-63
		// Interrupts 64-79.
		MAKE_ICONF(ICONF_E_1N, ICONF_E_1N, ICONF_E_1N, ICONF_E_1N,  // 64-67
		           ICONF_E_1N, ICONF_E_1N, ICONF_E_1N, ICONF_RSVD,  // 68-71
		           ICONF_E_1N, ICONF_E_1N, ICONF_E_1N, ICONF_E_1N,  // 72-75
		           ICONF_E_1N, ICONF_E_1N, ICONF_E_1N, ICONF_L_1N), // 76-79
		// Interrupts 80-95.
		MAKE_ICONF(ICONF_E_1N, ICONF_E_1N, ICONF_E_1N, ICONF_E_1N,  // 80-83
		           ICONF_E_1N, ICONF_E_1N, ICONF_E_1N, ICONF_E_1N,  // 84-87
		           ICONF_L_1N, ICONF_E_1N, ICONF_E_1N, ICONF_E_1N,  // 88-91
		           ICONF_RSVD, ICONF_RSVD, ICONF_RSVD, ICONF_E_1N), // 92-95
		// Interrupts 96-111.
		MAKE_ICONF(ICONF_E_1N, ICONF_E_1N, ICONF_RSVD, ICONF_RSVD,  // 96-99
		           ICONF_E_1N, ICONF_E_1N, ICONF_E_1N, ICONF_RSVD,  // 100-103
		           ICONF_E_1N, ICONF_E_1N, ICONF_E_1N, ICONF_E_1N,  // 104-107
		           ICONF_E_1N, ICONF_E_1N, ICONF_E_1N, ICONF_E_1N), // 108-111
		// Interrupts 112-127.
		MAKE_ICONF(ICONF_E_1N, ICONF_E_1N, ICONF_E_1N, ICONF_E_1N,  // 112-115
		           ICONF_E_1N, ICONF_E_1N, ICONF_L_1N, ICONF_L_1N,  // 116-119
		           ICONF_E_1N, ICONF_L_1N, ICONF_L_1N, ICONF_L_1N,  // 120-123
		           ICONF_L_1N, ICONF_L_1N, ICONF_RSVD, ICONF_RSVD)  // 124-127
	};

	iomemcpy(&gicDist->config[2], configTable, 6 * 4);
}

// Note: Core 0 must execute this last.
void IRQ_init(void)
{
	GicDist *const gicDist = getGicDistRegs();
	gicDist->ctrl = 0; // Disable the global interrupt distributor.

	configPrivateInterrupts(gicDist);

	if(__getCpuId() == 0)
	{
		// Disable the remaining 96 interrupts.
		// Set the remaining 96 pending interrupts to inactive state.
		for(u32 i = 1; i < 4; i++)
		{
			gicDist->enable_clear[i] = 0xFFFFFFFFu;
			gicDist->pending_clear[i] = 0xFFFFFFFFu;
		}

		// Set the remaining 96 interrupts to lowest priority.
		// Set the remaining 96 interrupts to target no CPU.
		iomemset(&gicDist->pri[8], 0xF0F0F0F0u, (32 - 8) * 4);
		iomemset(&gicDist->target[8], 0, (32 - 8) * 4);

		configExternalInterrupts(gicDist);

		// TODO: This is a potential bug. If any other core executes this
		// function the distributor will be disabled but not re-enabled.
		gicDist->ctrl = 1; // Enable the global interrupt distributor.
	}


	GicCpu *const gicCpu = getGicCpuRegs();
	gicCpu->primask  = 0xF0; // Mask no interrupt.
	gicCpu->binpoint = 3;    // All priority bits are compared for pre-emption.
	gicCpu->ctrl     = 1;    // Enable the interrupt interface for this CPU.

	getCfg11Regs()->fiq_mask = FIQ_MASK_CPU3 | FIQ_MASK_CPU2 | FIQ_MASK_CPU1 | FIQ_MASK_CPU0; // Disable FIQs.
}

void IRQ_registerIsr(Interrupt id, u8 prio, u8 cpuMask, IrqIsr isr)
{
	const u32 cpuId = __getCpuId();
	if(!cpuMask) cpuMask = 1u<<cpuId;

	const u32 oldState = enterCriticalSection();

	g_irqIsrTable[(id < 32 ? 32 * cpuId + id : 96u + id)] = isr;

	// Priority
	GicDist *const gicDist = getGicDistRegs();
	u32 shift = (id % 4 * 8) + 4;
	u32 tmp = gicDist->pri[id / 4] & ~(0xFu<<shift);
	gicDist->pri[id / 4] = tmp | (u32)prio<<shift;

	// Target
	shift = id % 4 * 8;
	tmp = gicDist->target[id / 4] & ~(0xFu<<shift);
	gicDist->target[id / 4] = tmp | (u32)cpuMask<<shift;

	// Enable it.
	gicDist->enable_set[id / 32] = 1u<<(id % 32);

	leaveCriticalSection(oldState);
}

void IRQ_enable(Interrupt id)
{
	getGicDistRegs()->enable_set[id / 32] = 1u<<(id % 32);
}

void IRQ_disable(Interrupt id)
{
	getGicDistRegs()->enable_clear[id / 32] = 1u<<(id % 32);
}

void IRQ_softInterrupt(Interrupt id, u8 cpuMask)
{
	getGicDistRegs()->softint = (u32)cpuMask<<16 | id;
}

void IRQ_setPriority(Interrupt id, u8 prio)
{
	const u32 oldState = enterCriticalSection();

	GicDist *const gicDist = getGicDistRegs();
	const u32 shift = (id % 4 * 8) + 4;
	u32 tmp = gicDist->pri[id / 4] & ~(0xFu<<shift);
	gicDist->pri[id / 4] = tmp | (u32)prio<<shift;

	leaveCriticalSection(oldState);
}

// TODO: The reg write is atomic.
//       The ISR table write could be moved somewhere else by the compiler. Signal fence is enough?
//       Without critical section the ISR for this exact IRQ could re-enable it between the 2 writes.
//       Could be fixed by doing an acquire fence + doing the table write first.
void IRQ_unregisterIsr(Interrupt id)
{
	const u32 oldState = enterCriticalSection();

	getGicDistRegs()->enable_clear[id / 32] = 1u<<(id % 32);

	g_irqIsrTable[(id < 32 ? 32 * __getCpuId() + id : 96u + id)] = (IrqIsr)NULL;

	leaveCriticalSection(oldState);
}
