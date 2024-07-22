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
#include "mem_map.h"

.syntax unified
.cpu mpcore
.fpu vfpv2



@ Whole frame converter.
/*BEGIN_ASM_FUNC convertFrameFast
	@ Load frame, output and lookup table pointers.
	@ Our frame is in a 512x512 texture. Same for the output.
	@ The table is a 15 to 32-bit 3D lookup table with color correction pre-applied.
	ldr  r0, =0x18200000                @ r0 = 0x18200000;
	ldr  r1, =0x18300000                @ r1 = 0x18300000;
	ldr  r2, =0x1FF00000                @ r2 = 0x1FF00000;

	@ Prefetch first cache line, save registers, load color mask and load 8 line counter.
	pld  [r0]                           @ Prefetch from r0.
	stmfd sp!, {r4-r11, lr}             @ Save registers.
	ldrh r12, =0x7FFF                   @ r12 = 0x7FFF;
	mov  r11, #30                       @ r11 = 30;

	@ Convert 8 lines each round until we have a whole frame.
	convertFrameFast_8l_lp:
		@ Load size of 8 lines in bytes.
		mov  r3, #0x1680                @ r3 = 0x1680;

		@ Convert 8 pixels each round until we have 8 lines.
		convertFrameFast_8p_lp:
			@ Load 8 pixels from frame.
			ldmia  r0!, {r8-r10, lr}    @ r8_to_r10_lr = *((_16BytesBlock*)r0); r0 += 16;

			@ Decrement size and extract first 2 pixels.
			subs r3,  r3, #16           @ r3 -= 16;              // Updates flags.
			and  r4, r12,  r8, lsr #1   @ r4 = 0x7FFF & (r8>>1); // r12 is 0x7FFF.
			lsr  r5,  r8, #17           @ r5 = r8>>17;

			@ Look up pixel 1 and extract pixel 3.
			ldr  r4, [r2,  r4, lsl #2]  @ r4 = r2[r4];           // u32.
			and  r6, r12,  r9, lsr #1   @ r6 = 0x7FFF & (r9>>1); // r12 is 0x7FFF.

			@ Look up pixel 2 and extract pixel 4.
			ldr  r5, [r2,  r5, lsl #2]  @ r5 = r2[r5]; // u32.
			lsr  r7,  r9, #17           @ r7 = r9>>17;

			@ Look up pixel 3 and extract pixel 5.
			ldr  r6, [r2,  r6, lsl #2]  @ r6 = r2[r6];            // u32.
			and  r8, r12, r10, lsr #1   @ r8 = 0x7FFF & (r10>>1); // r12 is 0x7FFF.

			@ Look up pixel 4 and extract pixel 6.
			ldr  r7, [r2,  r7, lsl #2]  @ r7 = r2[r7];  // u32.
			lsr  r9, r10, #17           @ r9 = r10>>17;

			@ Look up pixel 5 and extract pixel 7.
			ldr  r8, [r2,  r8, lsl #2]  @ r8 = r2[r8];            // u32.
			and r10, r12,  lr, lsr #1   @ r10 = 0x7FFF & (lr>>1); // r12 is 0x7FFF.

			@ Look up pixel 6 and extract pixel 8.
			ldr  r9, [r2,  r9, lsl #2]  @ r9 = r2[r9]; // u32.
			lsr  lr,  lr, #17           @ lr = lr>>17;

			@ Look up pixel 7 and 8.
			ldr r10, [r2, r10, lsl #2]  @ r10 = r2[r10]; // u32.
			ldr  lr, [r2,  lr, lsl #2]  @ lr = r2[lr];   // u32.

			@ Prefetch next cache line, write 8 pixels and jump back if we are not done yet.
			pld [r0, #32]               @ Prefetch from r0 + 32. // Offset 32 is a tiny bit better. Most of the time the result is the same as 64.
			stmia  r1!, {r4-r10, lr}    @ *((_32BytesBlock*)r1) = r4_to_r10_lr; r1 += 32;
			bne convertFrameFast_8p_lp  @ if(r3 != 0) goto convertFrameFast_8p_lp;

		@ Decrement 8 line counter, skip texture padding and jump back if we are not done yet.
		subs r11, r11, #1               @ r11--;        // Updates flags.
		add   r0,  r0, #0x980           @ r0 += 0x980;
		add   r1,  r1, #0x1300          @ r1 += 0x1300;
		bne convertFrameFast_8l_lp      @ if(r11 != 0) goto convertFrameFast_8l_lp;

	ldmfd sp!, {r4-r11, pc}             @ Restore registers and return.
END_ASM_FUNC*/

@ Converts a 160p frame while it's being DMAd to memory.
BEGIN_ASM_FUNC convert160pFrameFast
	@ Enable top LCD LgyCap IRQs.
	mov  r0, #77                                   @ r0 = 77; // id     IRQ_LGYCAP_TOP.
	mov  r1, #0                                    @ r1 = 0;  // prio   0 (highest).
	mov  r2, #0                                    @ r2 = 0;  // target 0 (this CPU).
	mov  r3, #0                                    @ r3 = 0;  // isr    NULL.
	blx IRQ_registerIsr                            @ IRQ_registerIsr(IRQ_LGYCAP_TOP, 0, 0, (IrqIsr)NULL);

	@ We will be using IRQs without our IRQ handler to minimize latency.
	cpsid i                                        @ __disableIrq();

	@ Load lookup table address and color mask.
	ldr   r2, =0x1FF00000                          @ r2 = 0x1FF00000;
	ldrh r12, =0x7FFF                              @ r12 = 0x7FFF;

	convert160pFrameFast_frame_lp:
		@ Load input and output addresses.
		ldr  r0, =0x18200000                       @ r0 = 0x18200000;    // u32.
		@ldr  r1, =0x18300000                       @ r1 = 0x18300000;    // u32.
		add  r1,  r0, #0x100000                    @ r1 = r0 + 0x100000; // Note: ldr would be faster here (result latency). Saves 4 bytes.

		@ Convert 8 lines each round until we have a whole frame.
		convert160pFrameFast_8l_lp:
			ldr  r4, =0x10111008                   @ r4 = &REG_LGYCAP1_STAT; // u32.
			ldr  r5, =MPCORE_PRIV_BASE             @ r5 = MPCORE_PRIV_BASE;  // u32.

			convert160pFrameFast_wait_irq:
				@ Wait for LgyCap IRQs.
				wfi                                @ __waitForInterrupt();

				@ Acknowledge IRQ and extract line number.
				ldr r11, [r4]                      @ r11 = REG_LGYCAP_STAT; // u32.
				ldr  r7, [r5, #0x10C]              @ r7 = REG_GICC_INTACK;  // u32.
				str r11, [r4]                      @ REG_LGYCAP_STAT = r11; // u32.
				lsrs r11, r11, #16                 @ r11 >>= 16;            // Updates flags.
				str  r7, [r5, #0x110]              @ REG_GICC_EOI = r7;     // u32.

				@ Ignore DREQ IRQ for line 0.
				beq convert160pFrameFast_wait_irq      @ if((r11>>16) == 0) goto convert160pFrameFast_wait_irq;

			convert160pFrameFast_skip_irq_wait:
			@ Load size of 8 lines in bytes.
			mov  r3, #0xF00                        @ r3 = 0xF00;

			@ Convert 8 pixels each round until we have 8 lines.
			convert160pFrameFast_8p_lp:
				@ Load 8 pixels from frame.
				ldmia  r0!, {r8-r10, lr}           @ r8_to_r10_lr = *((_16BytesBlock*)r0); r0 += 16;

				@ Decrement size and extract first 2 pixels.
				subs r3,  r3, #16                  @ r3 -= 16;              // Updates flags.
				and  r4, r12,  r8, lsr #1          @ r4 = 0x7FFF & (r8>>1); // r12 is 0x7FFF.
				lsr  r5,  r8, #17                  @ r5 = r8>>17;

				@ Look up pixel 1 and extract pixel 3.
				ldr  r4, [r2,  r4, lsl #2]         @ r4 = r2[r4];           // u32.
				and  r6, r12,  r9, lsr #1          @ r6 = 0x7FFF & (r9>>1); // r12 is 0x7FFF.

				@ Look up pixel 2 and extract pixel 4.
				ldr  r5, [r2,  r5, lsl #2]         @ r5 = r2[r5]; // u32.
				lsr  r7,  r9, #17                  @ r7 = r9>>17;

				@ Look up pixel 3 and extract pixel 5.
				ldr  r6, [r2,  r6, lsl #2]         @ r6 = r2[r6];            // u32.
				and  r8, r12, r10, lsr #1          @ r8 = 0x7FFF & (r10>>1); // r12 is 0x7FFF.

				@ Look up pixel 4 and extract pixel 6.
				ldr  r7, [r2,  r7, lsl #2]         @ r7 = r2[r7];  // u32.
				lsr  r9, r10, #17                  @ r9 = r10>>17;

				@ Look up pixel 5 and extract pixel 7.
				ldr  r8, [r2,  r8, lsl #2]         @ r8 = r2[r8];            // u32.
				and r10, r12,  lr, lsr #1          @ r10 = 0x7FFF & (lr>>1); // r12 is 0x7FFF.

				@ Look up pixel 6 and extract pixel 8.
				ldr  r9, [r2,  r9, lsl #2]         @ r9 = r2[r9]; // u32.
				lsr  lr,  lr, #17                  @ lr = lr>>17;

				@ Look up pixel 7 and 8.
				ldr r10, [r2, r10, lsl #2]         @ r10 = r2[r10]; // u32.
				ldr  lr, [r2,  lr, lsl #2]         @ lr = r2[lr];   // u32.

				@ Prefetch next cache line, write 8 pixels and jump back if we are not done yet.
				pld [r0, #32]                      @ Prefetch from r0 + 32. // Offset 32 is a tiny bit better. Most of the time the result is the same as 64.
				stmia  r1!, {r4-r10, lr}           @ *((_32BytesBlock*)r1) = r4_to_r10_lr; r1 += 32;
				bne convert160pFrameFast_8p_lp     @ if(r3 != 0) goto convert160pFrameFast_8p_lp;

			@ Test if 8 line counter is 152, skip texture padding and jump back if we are not done yet.
			cmp r11, #152                          @ r11 - 152; // Updates flags.
			add  r0,  r0, #0x1100                  @ r0 += 0x1100;
			add  r1,  r1, #0x2200                  @ r1 += 0x2200;
			moveq r11, #160                        @ if(r11 == 152) r11 = 160;
			beq convert160pFrameFast_skip_irq_wait @ if(r11 == 152) goto convert160pFrameFast_skip_irq_wait;
			bls convert160pFrameFast_8l_lp         @ if(r11 <= 152) goto convert160pFrameFast_8l_lp;

		@ Flush the D-Cache, wait for flush completion, notify core 0 and jump back.
		@ Note: r3 has been decremented down to 0 previously and so it's safe to use.
		mcr p15, 0, r3, c7, c14, 0                 @ Clean and Invalidate Entire Data Cache.
		ldr  r4, =MPCORE_PRIV_BASE                 @ r4 = MPCORE_PRIV_BASE;  // u32.
		mov  r5, #0x10000                          @ r5 = 0x10000;
		orr  r5,  r5, #0xF                         @ r5 |= 0xF;
		add  r4,  r4, #0x1F00                      @ r4 += 0x1F00; // REG_GICD_SOFTINT.
		mcr p15, 0, r3, c7, c10, 4                 @ Data Synchronization Barrier.
		str  r5, [r4]                              @ *r4 = r5; // u32.
		b convert160pFrameFast_frame_lp            @ goto convert160pFrameFast_frame_lp;
END_ASM_FUNC

@ Converts the frame while it's being DMAd to memory.
BEGIN_ASM_FUNC convert240pFrameFast
	@ Enable top LCD LgyCap IRQs.
	mov  r0, #77                                   @ r0 = 77; // id     IRQ_LGYCAP_TOP.
	mov  r1, #0                                    @ r1 = 0;  // prio   0 (highest).
	mov  r2, #0                                    @ r2 = 0;  // target 0 (this CPU).
	mov  r3, #0                                    @ r3 = 0;  // isr    NULL.
	blx IRQ_registerIsr                            @ IRQ_registerIsr(IRQ_LGYCAP_TOP, 0, 0, (IrqIsr)NULL);

	@ We will be using IRQs without our IRQ handler to minimize latency.
	cpsid i                                        @ __disableIrq();

	@ Load lookup table address and color mask.
	ldr   r2, =0x1FF00000                          @ r2 = 0x1FF00000;
	ldrh r12, =0x7FFF                              @ r12 = 0x7FFF;

	convert240pFrameFast_frame_lp:
		@ Load input and output addresses.
		ldr  r0, =0x18200000                       @ r0 = 0x18200000;    // u32.
		@ldr  r1, =0x18300000                       @ r1 = 0x18300000;    // u32.
		add  r1,  r0, #0x100000                    @ r1 = r0 + 0x100000; // Note: ldr would be faster here (result latency). Saves 4 bytes.

		@ Convert 8 lines each round until we have a whole frame.
		convert240pFrameFast_8l_lp:
			ldr  r4, =0x10111008                   @ r4 = &REG_LGYCAP1_STAT; // u32.
			ldr  r5, =MPCORE_PRIV_BASE             @ r5 = MPCORE_PRIV_BASE;  // u32.

			convert240pFrameFast_wait_irq:
				@ Wait for LgyCap IRQs.
				wfi                                @ __waitForInterrupt();

				@ Acknowledge IRQ and extract line number.
				ldr r11, [r4]                      @ r11 = REG_LGYCAP_STAT; // u32.
				ldr  r7, [r5, #0x10C]              @ r7 = REG_GICC_INTACK;  // u32.
				str r11, [r4]                      @ REG_LGYCAP_STAT = r11; // u32.
				lsrs r11, r11, #16                 @ r11 >>= 16;            // Updates flags.
				str  r7, [r5, #0x110]              @ REG_GICC_EOI = r7;     // u32.

				@ Ignore DREQ IRQ for line 0.
				beq convert240pFrameFast_wait_irq      @ if((r11>>16) == 0) goto convert240pFrameFast_wait_irq;

			convert240pFrameFast_skip_irq_wait:
			@ Load size of 8 lines in bytes.
			mov  r3, #0x1680                       @ r3 = 0x1680;

			@ Convert 8 pixels each round until we have 8 lines.
			convert240pFrameFast_8p_lp:
				@ Load 8 pixels from frame.
				ldmia  r0!, {r8-r10, lr}           @ r8_to_r10_lr = *((_16BytesBlock*)r0); r0 += 16;

				@ Decrement size and extract first 2 pixels.
				subs r3,  r3, #16                  @ r3 -= 16;              // Updates flags.
				and  r4, r12,  r8, lsr #1          @ r4 = 0x7FFF & (r8>>1); // r12 is 0x7FFF.
				lsr  r5,  r8, #17                  @ r5 = r8>>17;

				@ Look up pixel 1 and extract pixel 3.
				ldr  r4, [r2,  r4, lsl #2]         @ r4 = r2[r4];           // u32.
				and  r6, r12,  r9, lsr #1          @ r6 = 0x7FFF & (r9>>1); // r12 is 0x7FFF.

				@ Look up pixel 2 and extract pixel 4.
				ldr  r5, [r2,  r5, lsl #2]         @ r5 = r2[r5]; // u32.
				lsr  r7,  r9, #17                  @ r7 = r9>>17;

				@ Look up pixel 3 and extract pixel 5.
				ldr  r6, [r2,  r6, lsl #2]         @ r6 = r2[r6];            // u32.
				and  r8, r12, r10, lsr #1          @ r8 = 0x7FFF & (r10>>1); // r12 is 0x7FFF.

				@ Look up pixel 4 and extract pixel 6.
				ldr  r7, [r2,  r7, lsl #2]         @ r7 = r2[r7];  // u32.
				lsr  r9, r10, #17                  @ r9 = r10>>17;

				@ Look up pixel 5 and extract pixel 7.
				ldr  r8, [r2,  r8, lsl #2]         @ r8 = r2[r8];            // u32.
				and r10, r12,  lr, lsr #1          @ r10 = 0x7FFF & (lr>>1); // r12 is 0x7FFF.

				@ Look up pixel 6 and extract pixel 8.
				ldr  r9, [r2,  r9, lsl #2]         @ r9 = r2[r9]; // u32.
				lsr  lr,  lr, #17                  @ lr = lr>>17;

				@ Look up pixel 7 and 8.
				ldr r10, [r2, r10, lsl #2]         @ r10 = r2[r10]; // u32.
				ldr  lr, [r2,  lr, lsl #2]         @ lr = r2[lr];   // u32.

				@ Prefetch next cache line, write 8 pixels and jump back if we are not done yet.
				pld [r0, #32]                      @ Prefetch from r0 + 32. // Offset 32 is a tiny bit better. Most of the time the result is the same as 64.
				stmia  r1!, {r4-r10, lr}           @ *((_32BytesBlock*)r1) = r4_to_r10_lr; r1 += 32;
				bne convert240pFrameFast_8p_lp     @ if(r3 != 0) goto convert240pFrameFast_8p_lp;

			@ Test if 8 line counter is 232, skip texture padding and jump back if we are not done yet.
			cmp r11, #232                          @ r11 - 232; // Updates flags.
			add  r0,  r0, #0x980                   @ r0 += 0x980;
			add  r1,  r1, #0x1300                  @ r1 += 0x1300;
			moveq r11, #240                        @ if(r11 == 232) r11 = 240;
			beq convert240pFrameFast_skip_irq_wait @ if(r11 == 232) goto convert240pFrameFast_skip_irq_wait;
			bls convert240pFrameFast_8l_lp         @ if(r11 <= 232) goto convert240pFrameFast_8l_lp;

		@ Flush the D-Cache, wait for flush completion, notify core 0 and jump back.
		@ Note: r3 has been decremented down to 0 previously and so it's safe to use.
		mcr p15, 0, r3, c7, c14, 0                 @ Clean and Invalidate Entire Data Cache.
		ldr  r4, =MPCORE_PRIV_BASE                 @ r4 = MPCORE_PRIV_BASE;  // u32.
		mov  r5, #0x10000                          @ r5 = 0x10000;
		orr  r5,  r5, #0xF                         @ r5 |= 0xF;
		add  r4,  r4, #0x1F00                      @ r4 += 0x1F00; // REG_GICD_SOFTINT.
		mcr p15, 0, r3, c7, c10, 4                 @ Data Synchronization Barrier.
		str  r5, [r4]                              @ *r4 = r5; // u32.
		b convert240pFrameFast_frame_lp            @ goto convert240pFrameFast_frame_lp;
END_ASM_FUNC