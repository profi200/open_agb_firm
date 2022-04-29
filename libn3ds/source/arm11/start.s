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

#include "asm_macros.h"
#include "arm.h"
#include "mem_map.h"
#include "arm11/drivers/performance_monitor.h"

.syntax unified
.cpu mpcore
.fpu vfpv2

.section .crt0, "ax", %progbits



BEGIN_ASM_FUNC vectors no_section
	ldr pc, resetHandlerPtr         @ Reset vector
	ldr pc, undefInstrHandlerPtr    @ Undefined instruction vector
	udf #3                          @ Software interrupt (SVC) vector
	ldr pc, prefetchAbortHandlerPtr @ Prefetch abort vector
	ldr pc, dataAbortHandlerPtr     @ Data abort vector
	udf #6                          @ Reserved (unused) vector
	ldr pc, irqHandlerPtr           @ Interrupt (IRQ) vector
	udf #8                          @ Fast interrupt (FIQ) vector
	resetHandlerPtr:         .4byte _start
	undefInstrHandlerPtr:    .4byte undefInstrHandler
	@svcHandlerPtr:           .4byte (vectors + 0x08)
	prefetchAbortHandlerPtr: .4byte prefetchAbortHandler
	dataAbortHandlerPtr:     .4byte dataAbortHandler
	irqHandlerPtr:           .4byte irqHandler
	@fiqHandlerPtr:           .4byte (vectors + 0x1C)
END_ASM_FUNC


BEGIN_ASM_FUNC _start no_section
	cpsid aif, #PSR_SVC_MODE

	@ Control register:
	@ [29] Force AP functionality             : disabled
	@ [28] TEX remap                          : disabled
	@ [27] NMFI bit                           : normal FIQ behavior
	@ [25] CPSR E bit on taking an exception  : 0
	@ [23] Extended page table configuration  : subpage AP bits enabled
	@ [22] Unaligned data access              : disabled
	@ [15] Disable loading TBIT               : disabled
	@ [13] Vector select                      : 0x00000000
	@ [12] Level one instruction cache        : enabled
	@ [11] Program flow prediction            : disabled
	@ [7]  Endianess                          : little
	@ [2]  Level one data cache               : disabled
	@ [1]  Strict data address alignment fault: disabled
	@ [0]  MMU                                : disabled
	ldr r3, =0x55078
	mov r4, #0
	mcr p15, 0, r3, c1, c0, 0   @ Write control register
	mcr p15, 0, r4, c1, c0, 1   @ Write Auxiliary Control Register
	mcr p15, 0, r4, c7, c7, 0   @ Invalidate Both Caches. Also flushes the branch target cache
	mcr p15, 0, r4, c7, c10, 4  @ Data Synchronization Barrier
	mcr p15, 0, r4, c7, c5, 4   @ Flush Prefetch Buffer
	clrex

	mrc p15, 0, r4, c0, c0, 5   @ Get CPU ID
	ands r4, r4, #3
	bleq stubExceptionVectors   @ Stub the vectors in AXIWRAM bootrom vectors jump to

	mov sp, #0                  @ unused SVC mode sp
	cps #PSR_FIQ_MODE
	mov sp, #0                  @ Unused
	cps #PSR_IRQ_MODE
	mov sp, #0                  @ not needed
	ldr r0, =A11_EXC_STACK_END
	cps #PSR_ABORT_MODE
	mov sp, r0
	cps #PSR_UNDEF_MODE
	mov sp, r0
	cps #PSR_SYS_MODE
	adr r2, _sysmode_stacks
	ldr sp, [r2, r4, lsl #2]

	cmp r4, #0
	bne _start_skip_bss_init_array

	@ Clear bss section
	ldr r0, =__bss_start__
	ldr r1, =__bss_end__
	sub r2, r1, r0
	mov r1, #0
	bl iomemset
	@ Setup newlib heap
	ldr r0, =A11_HEAP_END
	ldr r1, =fake_heap_end
	str r0, [r1]
	blx __libc_init_array       @ Initialize ctors and dtors
	blx PDN_core123Init
_start_skip_bss_init_array:
	@ Disable + reset all performance monitor counters. Acknowledge IRQs.
	ldrh r2, =PM_CCNT_IRQ | PM_PMN1_IRQ | PM_PMN0_IRQ | PM_CCNT_RST | PM_PMN01_RST
	mcr p15, 0, r2, c15, c12, 0 @ Write Performance Monitor Control Register
	blx setupMmu
	bl setupVfp
	cpsie a
	blx __systemInit

	mov r0, #0                  @ argc
	adr r1, _dummyArgv          @ argv
	blx main
	blx __systemDeinit
_start_lp:
	wfi
	b _start_lp

.pool
_sysmode_stacks:
	.4byte A11_C0_STACK_END      @ Stack for core 0
	.4byte A11_C1_STACK_END      @ Stack for core 1
	.4byte A11_C2_STACK_END      @ Stack for core 2
	.4byte A11_C3_STACK_END      @ Stack for core 3
_dummyArgv:
	.4byte 0
END_ASM_FUNC


#define MAKE_BRANCH(src, dst) (0xEA000000 | (((((dst) - (src)) >> 2) - 2) & 0xFFFFFF))

BEGIN_ASM_FUNC stubExceptionVectors no_section
	ldr r0, =A11_VECTORS_START
	ldr r2, =MAKE_BRANCH(0, 0)  @ Endless loop
	mov r1, #6
	stubExceptionVectors_lp:
		str r2, [r0], #8
		subs r1, r1, #1
		bne stubExceptionVectors_lp
	bx lr

.pool
END_ASM_FUNC


BEGIN_ASM_FUNC setupVfp no_section
	mov r0, #0xF00000           @ Give full access to cp10/11 in user and privileged mode
	mov r1, #0
	mcr p15, 0, r0, c1, c0, 2   @ Write Coprocessor Access Control Register
	mcr p15, 0, r1, c7, c5, 4   @ Flush Prefetch Buffer
	mov r0, #0x40000000         @ Clear exception bits and enable VFP11
	mov r1, #0x3000000          @ Round to nearest (RN) mode, flush-to-zero mode, default NaN mode
	fmxr fpexc, r0              @ Write Floating-point exception register
	fmxr fpscr, r1              @ Write Floating-Point Status and Control Register
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC _init no_section thumb
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC deinitCpu no_section
	mov r3, lr

	cpsid aif, #PSR_SYS_MODE
	bl stubExceptionVectors
	bl cleanDCache

	ldr r1, =0xC03805           @ Disable MMU, D-Cache, Program flow prediction, I-Cache,
	                            @ high exception vectors, Unaligned data access,
	                            @ subpage AP bits disabled
	mrc p15, 0, r0, c1, c0, 0   @ Read control register
	bic r0, r0, r1
	mcr p15, 0, r0, c1, c0, 0   @ Write control register
	mrc p15, 0, r0, c1, c0, 1   @ Read Auxiliary Control Register
	bic r0, r0, #0x6F           @ Return stack, Dynamic branch prediction, Static branch prediction,
	                            @ Instruction folding, SMP mode: the CPU is taking part in coherency
	                            @ and L1 parity checking
	mcr p15, 0, r0, c1, c0, 1   @ Write Auxiliary Control Register
	mcr p15, 0, r2, c7, c7, 0   @ Invalidate Both Caches. Also flushes the branch target cache
	mcr p15, 0, r2, c7, c10, 4  @ Data Synchronization Barrier
	mcr p15, 0, r2, c7, c5, 4   @ Flush Prefetch Buffer
	bx r3

.pool
END_ASM_FUNC
