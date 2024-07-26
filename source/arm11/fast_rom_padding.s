@ This file is part of open_agb_firm
@ Copyright (C) 2024 profi200
@
@ This program is free software: you can redistribute it and/or modify
@ it under the terms of the GNU General Public License as published by
@ the Free Software Foundation, either version 3 of the License, or
@ (at your option) any later version.
@
@ This program is distributed in the hope that it will be useful,
@ but WITHOUT ANY WARRANTY; without even the implied warranty of
@ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@ GNU General Public License for more details.
@
@ You should have received a copy of the GNU General Public License
@ along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "asm_macros.h"

.syntax unified
.cpu mpcore
.fpu vfpv2



@ void makeOpenBusPaddingFast(u32 *romEnd);
BEGIN_ASM_FUNC makeOpenBusPaddingFast
	@ Save registers and calculate size from start and highest ROM address.
	stmfd sp!, {r4, lr}                   @ Save registers.
	rsb  r1,  r0, #0x22000000             @ r1 = 0x22000000 - r0;

	@ Generate pattern halves from address.
	lsr  r2,  r0, #1                      @ r2 = r0>>1;
	add  r3,  r2, #1                      @ r3 = r2 + 1;

	@ Generate constant for incrementing the pattern halves.
	mov r12, #2                           @ r12 = 2;
	add r12, r12, #0x20000                @ r12 += 0x20000;

	@ Join pattern halves and precalculate the next 3 patterns.
	pkhbt  r2,  r2,  r3, lsl #16          @ r2 = (r2 & 0xFFFF) | r3<<16;
	uadd16  r3,  r2, r12                  @ r3 = ((r2 + 0x20000) & 0xFFFF0000) | ((r2 + 2) & 0xFFFF); // r12 is 0x20002.
	uadd16  r4,  r3, r12                  @ r4 = ((r3 + 0x20000) & 0xFFFF0000) | ((r3 + 2) & 0xFFFF); // r12 is 0x20002.
	uadd16  lr,  r4, r12                  @ lr = ((r4 + 0x20000) & 0xFFFF0000) | ((r4 + 2) & 0xFFFF); // r12 is 0x20002.

	@ Adjust constant for unrolled loop. 0x20002 --> 0x80008.
	lsl r12, r12, #2                      @ r12 <<= 2;
	makeOpenBusPaddingFast_blk_lp:
		@ Store 16 pattern bytes at a time and decrement size.
		stmia r0!, {r2-r4, lr}            @ *((_16BytesBlock*)r0) = r2_to_r4_lr; r0 += 16;
		subs  r1,  r1, #16                @ r1 -= 16; // Updates flags.

		@ Increment patterns and jump back if we are not done yet.
		uadd16  r2,  r2, r12              @ r2 = ((r2 + 0x80000) & 0xFFFF0000) | ((r2 + 8) & 0xFFFF); // r12 is 0x80008.
		uadd16  r3,  r3, r12              @ r3 = ((r3 + 0x80000) & 0xFFFF0000) | ((r3 + 8) & 0xFFFF); // r12 is 0x80008.
		uadd16  r4,  r4, r12              @ r3 = ((r4 + 0x80000) & 0xFFFF0000) | ((r4 + 8) & 0xFFFF); // r12 is 0x80008.
		uadd16  lr,  lr, r12              @ lr = ((lr + 0x80000) & 0xFFFF0000) | ((lr + 8) & 0xFFFF); // r12 is 0x80008.
		bne makeOpenBusPaddingFast_blk_lp @ if(r1 != 0) goto makeOpenBusPaddingFast_blk_lp;

	ldmfd sp!, {r4, pc}                   @ Restore registers and return.
END_ASM_FUNC