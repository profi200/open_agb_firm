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

#include <stdio.h>
#include <string.h>
#include "types.h"
#include "mem_map.h"
#include "hardware/pxi.h"
#include "ipc_handler.h"
//#include "fatfs/ff.h"
//#include "fs.h"
#include "arm9/hardware/interrupt.h"
#include "hardware/gfx.h"
#include "arm9/hardware/ndma.h"



noreturn void panic()
{
	enterCriticalSection();
	//fsDeinit();
	PXI_sendPanicCmd(IPC_CMD11_PANIC);

	while(1)
	{
		const u32 color = RGB8_to_565(0, 255, 0)<<16 | RGB8_to_565(0, 255, 0);
		NDMA_fill((u32*)FRAMEBUF_BOT_A_1, color, SCREEN_SIZE_BOT);
		NDMA_fill((u32*)FRAMEBUF_BOT_A_2, color, SCREEN_SIZE_BOT);
	}
}

noreturn void panicMsg(UNUSED const char *msg)
{
	enterCriticalSection();
	//fsDeinit();
	PXI_sendPanicCmd(IPC_CMD11_PANIC);

	while(1)
	{
		const u32 color = RGB8_to_565(0, 255, 0)<<16 | RGB8_to_565(0, 255, 0);
		NDMA_fill((u32*)FRAMEBUF_BOT_A_1, color, SCREEN_SIZE_BOT);
		NDMA_fill((u32*)FRAMEBUF_BOT_A_2, color, SCREEN_SIZE_BOT);
	}
}

// Expects the registers in the exception stack to be in the following order:
// r0-r14, pc (unmodified), cpsr
noreturn void guruMeditation(UNUSED u8 type, UNUSED const u32 *excStack)
{
	// avoid fs corruptions
	//fsDeinit();
	PXI_sendPanicCmd(IPC_CMD11_EXCEPTION);

	while(1)
	{
		const u32 color = RGB8_to_565(255, 0, 0)<<16 | RGB8_to_565(255, 0, 0);
		NDMA_fill((u32*)FRAMEBUF_BOT_A_1, color, SCREEN_SIZE_BOT);
		NDMA_fill((u32*)FRAMEBUF_BOT_A_2, color, SCREEN_SIZE_BOT);
	}
}

/*void dumpMem(u8 *mem, u32 size, char *filepath)
{
	FIL file;
	UINT bytesWritten;

	if(f_open(&file, filepath, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
		return;

	f_write(&file, mem, size, &bytesWritten);
	f_sync(&file);
	f_close(&file);
}*/
