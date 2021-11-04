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
#include "util.h"
#include "ipc_handler.h"
#include "arm11/drivers/mcu.h"
#include "arm11/drivers/timer.h"
#include "drivers/cache.h"
#include "drivers/pxi.h"
#include "arm.h"



static void power_safe_halt(void)
{
	PXI_sendCmd(IPC_CMD9_PREPARE_POWER, NULL, 0);

	cleanDCache();
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
