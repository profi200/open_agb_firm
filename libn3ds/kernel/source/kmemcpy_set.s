/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2019 Aurora Wright, TuxSH, derrek, profi200
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

 @ Based on https://github.com/AuroraWright/Luma3DS/blob/master/arm9/source/alignedseqmemcpy.s

.syntax unified
.arm
.cpu arm946e-s
.fpu softvfp



@ void kmemcpy(u32 *restrict dst, const u32 *restrict src, u32 size);
@ void iokmemcpy(vu32 *restrict dst, const vu32 *restrict src, u32 size);
.section .text.kmemcpy, "ax", %progbits
.global kmemcpy
.global iokmemcpy
.type kmemcpy %function
.type iokmemcpy %function
.align 2
kmemcpy:
iokmemcpy:
	bics    r12, r2, #31
	beq     kmemcpy_test_words
	stmfd   sp!, {r4-r10}
	kmemcpy_blocks_lp:
		ldmia  r1!, {r3-r10}
		subs   r12, #32
		stmia  r0!, {r3-r10}
		bne    kmemcpy_blocks_lp
	ldmfd   sp!, {r4-r10}
kmemcpy_test_words:
	ands    r12, r2, #28
	beq     kmemcpy_halfword_byte
	kmemcpy_words_lp:
		ldr    r3, [r1], #4
		subs   r12, #4
		str    r3, [r0], #4
		bne    kmemcpy_words_lp
kmemcpy_halfword_byte:
	tst     r2, #2
	ldrhne  r3, [r1], #2
	strhne  r3, [r0], #2
	tst     r2, #1
	ldrbne  r3, [r1]
	strbne  r3, [r0]
	bx      lr


@ void kmemset(u32 *ptr, u32 value, u32 size);
@ void iokmemset(vu32 *ptr, u32 value, u32 size);
.section .text.kmemset, "ax", %progbits
.global kmemset
.global iokmemset
.type kmemset %function
.type iokmemset %function
.align 2
kmemset:
iokmemset:
	bics    r12, r2, #31
	beq     kmemset_test_words
	stmfd   sp!, {r4-r9}
	mov     r3, r1
	mov     r4, r1
	mov     r5, r1
	mov     r6, r1
	mov     r7, r1
	mov     r8, r1
	mov     r9, r1
	kmemset_blocks_lp:
		stmia  r0!, {r1, r3-r9}
		subs   r12, #32
		bne    kmemset_blocks_lp
	ldmfd   sp!, {r4-r9}
kmemset_test_words:
	ands    r12, r2, #28
	beq     kmemset_halfword_byte
	kmemset_words_lp:
		str    r1, [r0], #4
		subs   r12, #4
		bne    kmemset_words_lp
kmemset_halfword_byte:
	tst     r2, #2
	strhne  r1, [r0], #2
	tst     r2, #1
	strbne  r1, [r0]
	bx      lr
