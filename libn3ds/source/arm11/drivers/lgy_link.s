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
#include "mem_map.h"

.syntax unified
.cpu mpcore
.fpu vfpv2



@ void lgyLinkRecv16(u16 *dst, u32 size)
BEGIN_ASM_FUNC lgyLinkRecv16
	add r1, r0, r1              @ r1 = r0 + r1;
	ldr r12, =0x10141100        @ r12 = &REG_LGY_MODE; // Base address.
	mov r2, #0                  @ All buttons pressed (start signal).
	mov r3, #0xFFFFFFFF         @ Override all buttons. Should be 0x3FF but his works too.
	strh r2, [r12, #0x12]       @ REG_LGY_PAD_VAL = r2;
	cpsid i                     @ Disable IRQs. Timing is critical.
	strh r3, [r12, #0x10]       @ REG_LGY_PAD_SEL = r3;

lgyLinkRecv16_byte1_lp:
	ldrh r2, [r12, #0x0A]       @ r2 = REG_LGY_PADCNT;
	tst r2, #1<<8
	beq lgyLinkRecv16_byte1_lp @ while(!(r2 & 1u<<8));
	uxtb r2, r2                 @ r2 = r2 & 0xFFu;

lgyLinkRecv16_byte2_lp:
	ldrh r3, [r12, #0x0A]       @ r3 = REG_LGY_PADCNT;
	tst r3, #1<<8
	bne lgyLinkRecv16_byte2_lp @ while(r3 & 1u<<8);

	orr	r2, r2, r3, lsl #8      @ r2 = r3<<8 | r2;
	strh r2, [r0], #2           @ *r0++ = r2;
	cmp r1, r0
	bhi lgyLinkRecv16_byte1_lp @ while(r0 < r1);
	cpsie i                     @ Enable IRQs.
	bx lr
END_ASM_FUNC


@ void lgyLinkRecv32(u32 *dst, u32 size)
BEGIN_ASM_FUNC lgyLinkRecv32
	add r1, r0, r1              @ r1 = r0 + r1;
	ldr r12, =0x10141100        @ r12 = &REG_LGY_MODE; // Base address.
	mov r2, #0                  @ All buttons pressed (start signal).
	mov r3, #0xFFFFFFFF         @ Override all buttons. Should be 0x3FF but his works too.
	strh r2, [r12, #0x12]       @ REG_LGY_PAD_VAL = r2;
	cpsid i                     @ Disable IRQs. Timing is critical.
	strh r3, [r12, #0x10]       @ REG_LGY_PAD_SEL = r3;

lgyLinkRecv32_byte1_lp:
	ldrh r2, [r12, #0x0A]       @ r2 = REG_LGY_PADCNT;
	tst r2, #1<<8
	beq lgyLinkRecv32_byte1_lp  @ while(!(r2 & 1u<<8));
	uxtb r2, r2                 @ r2 = r2 & 0xFFu;

lgyLinkRecv32_byte2_lp:
	ldrh r3, [r12, #0x0A]       @ r3 = REG_LGY_PADCNT;
	tst r3, #1<<8
	bne lgyLinkRecv32_byte2_lp  @ while(r3 & 1u<<8);
	orr	r2, r2, r3, lsl #8      @ r2 = r3<<8 | r2;

lgyLinkRecv32_byte3_lp:
	ldrh r3, [r12, #0x0A]       @ r3 = REG_LGY_PADCNT;
	tst r3, #1<<8
	beq lgyLinkRecv32_byte3_lp  @ while(!(r3 & 1u<<8));
	bic r3, r3, #1<<8           @ r3 &= ~(1u<<8);
	orr	r2, r2, r3, lsl #16     @ r2 = r3<<16 | r2;

lgyLinkRecv32_byte4_lp:
	ldrh r3, [r12, #0x0A]       @ r3 = REG_LGY_PADCNT;
	tst r3, #1<<8
	bne lgyLinkRecv32_byte4_lp  @ while(r3 & 1u<<8);
	orr	r2, r2, r3, lsl #24     @ r2 = r3<<24 | r2;

	str r2, [r0], #4            @ *r0++ = r2;
	cmp r1, r0
	bhi lgyLinkRecv32_byte1_lp @ while(r0 < r1);
	cpsie i                     @ Enable IRQs.
	bx lr
END_ASM_FUNC




@ void lgyLinkSend32(const u32 *src, u32 size)
BEGIN_ASM_FUNC lgyLinkSend32
	add r1, r0, r1              @ r1 = r0 + r1;
	ldr r12, =0x10141100        @ r12 = &REG_LGY_MODE; // Base address.
	mov r2, #0                  @ All buttons pressed (start signal).
	mov r3, #0xFFFFFFFF         @ Override all buttons. Should be 0x3FF but his works too.
	strh r2, [r12, #0x12]       @ REG_LGY_PAD_VAL = r2;
	cpsid i                     @ Disable IRQs. Timing is critical.
	strh r3, [r12, #0x10]       @ REG_LGY_PAD_SEL = r3;

lgyLinkSend32_word_lp:
	ldr r2, [r0], #4

	strh r2, [r12, #0x12]
	lsr r2, r2, #8
lgyLinkSend32_byte1_lp:
	ldrh r3, [r12, #0x0A]
	cmp r3, #0xFF
	bne lgyLinkSend32_byte1_lp

	strh r2, [r12, #0x12]
	lsr r2, r2, #8
lgyLinkSend32_byte2_lp:
	ldrh r3, [r12, #0x0A]
	cmp r3, #0xFF
	beq lgyLinkSend32_byte2_lp

	strh r2, [r12, #0x12]
	lsr r2, r2, #8
lgyLinkSend32_byte3_lp:
	ldrh r3, [r12, #0x0A]
	cmp r3, #0xFF
	bne lgyLinkSend32_byte3_lp

	strh r2, [r12, #0x12]
	lsr r2, r2, #8
lgyLinkSend32_byte4_lp:
	ldrh r3, [r12, #0x0A]
	cmp r3, #0xFF
	beq lgyLinkSend32_byte4_lp

	cmp r1, r0
	bhi lgyLinkSend32_word_lp  @ while(r0 < r1);

	bx lr
END_ASM_FUNC
