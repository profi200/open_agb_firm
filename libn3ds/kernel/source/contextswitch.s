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

.syntax unified
.arm
.cpu mpcore
.fpu vfpv2



@ void switchContextNoScratchRegs(u32 *curRegs, const u32 *newRegs)
/*ASM_FUNC switchContextNoScratchRegs
	stmia r0!, {r4-r11, sp, lr}
	add r0, r0, #20                        @ Skip r0-r3, r12
	adr r2, switchContextNoScratchRegs_end
	mrs r3, cpsr
	stmia r0, {r2, r3}
	ldmia r1!, {r4-r11, sp, lr}
	add r1, r1, #20                        @ Skip r0-r3, r12
	rfeia r1
switchContextNoScratchRegs_end:
	cpsie i
	bx lr

@ void switchContextAllRegs(u32 *curRegs, const u32 *newRegs)
ASM_FUNC switchContextAllRegs
	stmia r0!, {r4-r11, sp, lr}
	add r0, r0, #20                  @ Skip r0-r3, r12
	adr r2, switchContextAllRegs_end
	mrs r3, cpsr
	stmia r0, {r2, r3}
	ldmia r1!, {r4-r11, sp, lr}
	ldr r3, [r1, #24]                @ cpsr
	cps #19                          @ SVC mode
	msr spsr_fsxc, r3
	ldmia r1, {r0-r3, r12, pc}^
switchContextAllRegs_end:
	cpsie i
	bx lr*/

@ KRes switchContext(KRes res, uintptr_t *oldSp, uintptr_t newSp);
BEGIN_ASM_FUNC switchContext
	stmfd sp!, {r4-r11, lr}
	str sp, [r1]
	mov sp, r2
	ldmfd sp!, {r4-r11, lr}
	bx lr
END_ASM_FUNC


.pool
