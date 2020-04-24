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
#include "arm11/console.h"
#include "arm11/fmt.h"
#include "hardware/pxi.h"
#include "ipc_handler.h"
#include "hardware/gfx.h"
#include "arm11/hardware/interrupt.h"
#include "arm.h"
#include "arm11/hardware/mcu.h"
#include "arm11/hardware/hid.h"



noreturn void panic()
{
	enterCriticalSection();

	consoleInit(SCREEN_BOT, NULL, false);
	ee_printf("\x1b[41m\x1b[0J\x1b[15C****PANIC!!!****\n");
	GX_textureCopy((u32*)RENDERBUF_TOP, 0, (u32*)GFX_getFramebuffer(SCREEN_TOP),
	               0, SCREEN_SIZE_TOP + SCREEN_SIZE_BOT);
	GFX_swapFramebufs();

	//PXI_sendPanicCmd(IPC_CMD9_PANIC);

	// Wait for A/B/X or Y
	do
	{
		hidScanInput();
	} while(!(hidKeysDown() & (KEY_A | KEY_B | KEY_X | KEY_Y)));

	MCU_powerOffSys();
	while(1) __wfi();
}

noreturn void panicMsg(const char *msg)
{
	enterCriticalSection();

	consoleInit(SCREEN_BOT, NULL, false);
	ee_printf("\x1b[41m\x1b[0J\x1b[15C****PANIC!!!****\n\n");
	ee_printf("\nERROR MESSAGE:\n%s\n", msg);
	GX_textureCopy((u32*)RENDERBUF_TOP, 0, (u32*)GFX_getFramebuffer(SCREEN_TOP),
				   0, SCREEN_SIZE_TOP + SCREEN_SIZE_BOT);
	GFX_swapFramebufs();

	//PXI_sendPanicCmd(IPC_CMD9_PANIC);

	// Wait for A/B/X or Y
	do
	{
		hidScanInput();
	} while(!(hidKeysDown() & (KEY_A | KEY_B | KEY_X | KEY_Y)));

	MCU_powerOffSys();
	while(1) __wfi();
}

// Expects the registers in the exception stack to be in the following order:
// r0-r14, pc (unmodified), cpsr
noreturn void guruMeditation(u8 type, const u32 *excStack)
{
	const char *const typeStr[3] = {"Undefined instruction", "Prefetch abort", "Data abort"};
	u32 realPc, instSize = 4;
	//bool codeChanged = false;


	// verify text and rodata
	/*u32 prevHash = debugHash;
	debugHashCodeRoData();
	if(prevHash != debugHash)
		codeChanged = true;*/

	consoleInit(SCREEN_BOT, NULL, false);

	if(excStack[16] & 0x20) instSize = 2;                 // Processor was in Thumb mode?
	if(type == 2) realPc = excStack[15] - (instSize * 2); // Data abort
	else realPc = excStack[15] - instSize;                // Other

	ee_printf("\x1b[41m\x1b[0J\x1b[15CGuru Meditation Error!\n\n%s:\n", typeStr[type]);
	ee_printf("CPSR: 0x%08" PRIX32 "\n"
	       "r0 = 0x%08" PRIX32 " r6  = 0x%08" PRIX32 " r12 = 0x%08" PRIX32 "\n"
	       "r1 = 0x%08" PRIX32 " r7  = 0x%08" PRIX32 " sp  = 0x%08" PRIX32 "\n"
	       "r2 = 0x%08" PRIX32 " r8  = 0x%08" PRIX32 " lr  = 0x%08" PRIX32 "\n"
	       "r3 = 0x%08" PRIX32 " r9  = 0x%08" PRIX32 " pc  = 0x%08" PRIX32 "\n"
	       "r4 = 0x%08" PRIX32 " r10 = 0x%08" PRIX32 "\n"
	       "r5 = 0x%08" PRIX32 " r11 = 0x%08" PRIX32 "\n\n",
	       excStack[16],
	       excStack[0], excStack[6],  excStack[12],
	       excStack[1], excStack[7],  excStack[13],
	       excStack[2], excStack[8],  excStack[14],
	       excStack[3], excStack[9],  realPc,
	       excStack[4], excStack[10],
	       excStack[5], excStack[11]);

	ee_puts("Stack dump:");
	u32 sp = excStack[13];
	if(sp >= AXIWRAM_BASE && sp < AXIWRAM_BASE + AXIWRAM_SIZE && !(sp & 3u))
	{
		u32 stackWords = ((AXIWRAM_BASE + AXIWRAM_SIZE - sp) / 4 > 48 ? 48 : (AXIWRAM_BASE + AXIWRAM_SIZE - sp) / 4);

		u32 newlineCounter = 0;
		for(u32 i = 0; i < stackWords; i++)
		{
			if(newlineCounter == 4) {ee_printf("\n"); newlineCounter = 0;}
			ee_printf("0x%08" PRIX32 " ", ((u32*)sp)[i]);
			newlineCounter++;
		}
	}

	//if(codeChanged) ee_printf("Attention: RO section data changed!!");
	GX_textureCopy((u32*)RENDERBUF_TOP, 0, (u32*)GFX_getFramebuffer(SCREEN_TOP),
				   0, SCREEN_SIZE_TOP + SCREEN_SIZE_BOT);
	GFX_swapFramebufs();

	//PXI_sendPanicCmd(IPC_CMD9_EXCEPTION);

	// Wait for A/B/X or Y
	do
	{
		hidScanInput();
	} while(!(hidKeysDown() & (KEY_A | KEY_B | KEY_X | KEY_Y)));

	MCU_powerOffSys();
	while(1) __wfi();
}

/*void debugMemdump(const char *filepath, void *mem, size_t size)
{
	s32 file;

	if((file = fOpen(filepath, FS_CREATE_ALWAYS | FS_OPEN_WRITE)) < 0)
	{
		return;
	}
		
	fWrite(file, mem, size);
	
	fSync(file);

	fClose(file);
}*/
