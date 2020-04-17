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
#include "util.h"
#include "ipc_handler.h"
#include "arm11/hardware/mcu.h"
#include "arm11/hardware/timer.h"
#include "hardware/cache.h"
#include "hardware/pxi.h"
#include "arm.h"



static void power_safe_halt(void)
{
	MCU_powerOffLCDs();
	PXI_sendCmd(IPC_CMD9_PREPARE_POWER, NULL, 0);

	// give the screens a bit of time to turn off
	TIMER_sleepMs(400);

	flushDCache();
}

noreturn void power_off(void)
{
	power_safe_halt();

	MCU_powerOffSys();

	__cpsid(aif);
	while(1) __wfi();
}

noreturn void power_reboot(void)
{
	power_safe_halt();

	MCU_rebootSys();

	__cpsid(aif);
	while(1) __wfi();
}
