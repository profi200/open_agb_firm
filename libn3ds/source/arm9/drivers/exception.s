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

.syntax unified
.cpu arm946e-s
.fpu softvfp

.macro EXCEPTION_ENTRY name, type
BEGIN_ASM_FUNC \name
	msr cpsr_f, #\type             @ Abuse conditional flags in cpsr for temporary exception type storage
	b exceptionHandler
END_ASM_FUNC
.endm



EXCEPTION_ENTRY undefInstrHandler, 0<<29
EXCEPTION_ENTRY prefetchAbortHandler, 1<<29
EXCEPTION_ENTRY dataAbortHandler, 2<<29
BEGIN_ASM_FUNC exceptionHandler
	sub sp, #68
	stmia sp, {r0-r14}^            @ Save all user/system mode regs except pc
	mrs r2, spsr                   @ Get saved cpsr
	mrs r3, cpsr
	lsr r0, r3, #29                @ Get back the exception type from cpsr
	and r1, r2, #PSR_MODE_MASK
	cmp r1, #PSR_USER_MODE
	beq exceptionHandler_skip_other_mode
	add r4, sp, #32
	msr cpsr_c, r2
	stmia r4!, {r8-r14}            @ Some regs are written twice but we don't care
	msr cpsr_c, r3
exceptionHandler_skip_other_mode:
	str lr, [sp, #60]              @ Save lr (pc) on exception stack
	str r2, [sp, #64]              @ Save spsr (cpsr) on exception stack
	mov r4, r0
	mov r5, sp
	bl deinitCpu
	mov r0, r4
	mov sp, r5
	mov r1, r5
	b guruMeditation               @ r0 = exception type, r1 = reg dump ptr {r0-r14, pc (unmodified), cpsr}
END_ASM_FUNC


BEGIN_ASM_FUNC irqHandler
	sub lr, lr, #4
	stmfd sp!, {r0-r3, r12, lr}
	ldr r12, =IO_MEM_ARM9_ONLY + 0x1000 @ REG_IRQ_IE
	ldrd r0, r1, [r12]
	mov r3, #0x80000000
	and r1, r0, r1
	irqHandler_find_first_lp:
		clz r2, r1
		bics r1, r1, r3, lsr r2
		bne irqHandler_find_first_lp
	mov r1, r3, lsr r2
	ldr r3, =g_irqIsrTable
	rsb r0, r2, #31                  @ r0 = 31 - r2
	str r1, [r12, #4]                @ REG_IRQ_IF
	ldr r2, [r3, r0, lsl #2]
	cmp r2, #0
	beq irqHandler_skip_processing
	mrs r3, spsr
	str r3, [sp, #-4]!
	msr cpsr_c, #PSR_F | PSR_SYS_MODE
	str lr, [sp, #-4]!               @ A single ldr/str can't be interrupted
	blx r2
	ldr lr, [sp], #4
	msr cpsr_c, #PSR_INT_OFF | PSR_IRQ_MODE
	ldr r0, [sp], #4
	msr spsr_cxsf, r0
irqHandler_skip_processing:
	ldmfd sp!, {r0-r3, r12, pc}^
END_ASM_FUNC
