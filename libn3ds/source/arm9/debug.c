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

#include <stdio.h>
#include <string.h>
#include "types.h"
#include "mem_map.h"
#include "drivers/pxi.h"
#include "ipc_handler.h"
#include "arm9/drivers/interrupt.h"
#include "drivers/gfx.h"
#include "arm9/drivers/ndma.h"



NOINLINE noreturn void panic(void)
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

NOINLINE noreturn void panicMsg(UNUSED const char *msg)
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
NOINLINE noreturn void guruMeditation(UNUSED u8 type, UNUSED const u32 *excStack)
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

#ifndef NDEBUG
// Needs to be marked as used to work with LTO.
// The used attribute also overrides the newlib symbol.
// This is for debugging purposes only. For security this value needs to be random!
__attribute__((used)) uintptr_t __stack_chk_guard = 0xC724B66D;

// Needs to be marked as noinline and used to work with LTO.
// The used attribute also overrides the newlib symbol.
// Combine -fstack-protector-all with -fno-inline to get the most effective detection.
__attribute__((noinline, used)) noreturn void __stack_chk_fail(void)
{
	panicMsg("Stack smash!");
}


// Add "-Wl,-wrap=malloc,-wrap=calloc,-wrap=free" to LDFLAGS to enable the heap check.
static const u32 __heap_chk_guard[4] = {0x9240A724, 0x6A6594A0, 0x976F0392, 0xB3A669AB};

void* __real_malloc(size_t size);
void __real_free(void *ptr);

void* __wrap_malloc(size_t size)
 {
	void *const buf = __real_malloc(size + 32);
	if(buf == NULL) return NULL;

	memcpy(buf, &size, sizeof(size_t));
	memcpy(buf + sizeof(size_t), (u8*)__heap_chk_guard + sizeof(size_t), 16 - sizeof(size_t));
	memcpy(buf + 16 + size, __heap_chk_guard, 16);

	return buf + 16;
}

void* __wrap_calloc(size_t num, size_t size)
{
	void *const buf = __wrap_malloc(num * size);
	if(buf == NULL) return NULL;

	memset(buf, 0, num * size);

	return buf;
}

void __wrap_free(void *ptr)
{
	if(ptr == NULL) return;

	if(memcmp(ptr - (16 - sizeof(size_t)), (u8*)__heap_chk_guard + sizeof(size_t), 16 - sizeof(size_t)) != 0)
		panicMsg("Heap underflow!");
	size_t size;
	memcpy(&size, ptr - 16, sizeof(size_t));

	// Important! Adjust the size check if needed.
	// 1024u * 1024 is roughly ok for ARM9 mem.
	if(size > (1024u * 1024) || memcmp(ptr + size, __heap_chk_guard, 16) != 0)
		panicMsg("Heap overflow!");

	__real_free(ptr - 16);
}
#endif
