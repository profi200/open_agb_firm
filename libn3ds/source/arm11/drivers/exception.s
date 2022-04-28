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
.cpu mpcore
.fpu vfpv2

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
	sub sp, #84
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
	mrc p15, 0, r3, c5, c0, 0
	str r3, [sp, #68]              @ DFSR
	mrc p15, 0, r3, c5, c0, 1
	str r3, [sp, #72]              @ IFSR
	mrc p15, 0, r3, c6, c0, 0
	str r3, [sp, #76]              @ FAR
	mrc p15, 0, r3, c6, c0, 1
	str r3, [sp, #80]              @ WFAR
	mov r4, r0
	mov r5, sp
	bl deinitCpu
	mov r0, r4
	mov sp, r5
	mov r1, r5
	b guruMeditation               @ r0 = exception type, r1 = reg dump ptr {r0-r14, pc (unmodified), CPSR, DFSR, IFSR, FAR, WFAR}
END_ASM_FUNC


BEGIN_ASM_FUNC irqHandler
	sub lr, lr, #4
	srsfd sp!, #PSR_SYS_MODE     @ Store lr and spsr on system mode stack
	cps #PSR_SYS_MODE
	stmfd sp!, {r0-r3, r12, lr}
	ldr r12, =MPCORE_PRIV_REG_BASE
	ldr r2, =g_irqIsrTable
	ldr r0, [r12, #0x10C]        @ REG_GIC_CPU_INTACK
	and r1, r0, #0x7F
	cmp r1, #32
	mrclo p15, 0, r3, c0, c0, 5  @ Get CPU ID
	andlo r3, r3, #3
	addlo r1, r1, r3, lsl #5
	addhs r1, r1, #96
	ldr r3, [r2, r1, lsl #2]
	cmp r3, #0
	beq irqHandler_skip_processing
	cpsie i
	str r0, [sp, #-4]!           @ A single ldr/str can't be interrupted
	blx r3
	ldr r0, [sp], #4
	ldr r12, =MPCORE_PRIV_REG_BASE
	cpsid i
irqHandler_skip_processing:
	str r0, [r12, #0x110]        @ REG_GIC_CPU_EOI
	ldmfd sp!, {r0-r3, r12, lr}
	rfefd sp!                    @ Restore lr (pc) and spsr (cpsr)
END_ASM_FUNC
