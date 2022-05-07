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
#include "arm9/drivers/interrupt.h"
#include "arm9/drivers/timer.h"
#include "arm9/drivers/ndma.h"
#include "drivers/corelink_dma-330.h"
#include "drivers/toshsd.h"
#include "drivers/pxi.h"



void WEAK __systemInit(void)
{
	IRQ_init();
	leaveCriticalSection(0); // Enables interrupts.
	TIMER_init();
	NDMA_init();
	DMA330_init();
	TOSHSD_init();
	PXI_init();
}

void WEAK __systemDeinit(void)
{
	TOSHSD_deinit();
	DMA330_init();
	NDMA_init();
	IRQ_init();
}
