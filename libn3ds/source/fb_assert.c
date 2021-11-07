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
#include "ipc_handler.h"
#include "drivers/pxi.h"
#ifdef ARM9
	#include "arm9/drivers/interrupt.h"
	#include "arm9/drivers/ndma.h"
#elif ARM11
	#include "arm11/fmt.h"
	#include "arm11/drivers/interrupt.h"
#endif
#include "drivers/gfx.h"
#include "arm.h"



noreturn void __fb_assert(const char *const str, u32 line)
{
	enterCriticalSection();

#ifdef ARM9
	// Get rid of the warnings.
	(void)str;
	(void)line;
	PXI_sendCmd(IPC_CMD11_PANIC, NULL, 0);
#elif ARM11
	ee_printf("Assertion failed: %s:%" PRIu32, str, line);
	//PXI_sendCmd(IPC_CMD9_PANIC, NULL, 0);
#endif

	while(1)
	{
#ifdef ARM9
		const u32 color = RGB8_to_565(0, 0, 255)<<16 | RGB8_to_565(0, 0, 255);
		NDMA_fill((u32*)FRAMEBUF_BOT_A_1, color, SCREEN_SIZE_BOT);
		NDMA_fill((u32*)FRAMEBUF_BOT_A_2, color, SCREEN_SIZE_BOT);
#elif ARM11
		__wfi();
#endif
	}
}
