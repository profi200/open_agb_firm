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

.section .crt0, "ax", %progbits



BEGIN_ASM_FUNC _start no_section
	msr cpsr_cxsf, #PSR_INT_OFF | PSR_SVC_MODE

	@ Control register:
	@ [19] ITCM load mode         : disabled
	@ [18] ITCM                   : disabled
	@ [17] DTCM load mode         : disabled
	@ [16] DTCM                   : disabled
	@ [15] Disable loading TBIT   : disabled
	@ [14] Round-robin replacement: disabled
	@ [13] Vector select          : 0xFFFF0000
	@ [12] I-Cache                : disabled
	@ [7]  Endianess              : little
	@ [2]  D-Cache                : disabled
	@ [0]  MPU                    : disabled
	ldrh r3, =0x2078
	mov r4, #0
	mcr p15, 0, r3, c1, c0, 0   @ Write control register
	mcr p15, 0, r4, c7, c5, 0   @ Invalidate I-Cache
	mcr p15, 0, r4, c7, c6, 0   @ Invalidate D-Cache
	mcr p15, 0, r4, c7, c10, 4  @ Drain write buffer

	bl setupExceptionVectors    @ Setup the vectors in ARM9 mem bootrom vectors jump to
	bl setupTcms                @ Setup and enable DTCM and ITCM

	mov sp, #0                  @ unused SVC mode sp
	msr cpsr_cxsf, #PSR_INT_OFF | PSR_IRQ_MODE
	ldr sp, =A9_IRQ_STACK_END
	ldr r0, =A9_EXC_STACK_END
	msr cpsr_cxsf, #PSR_INT_OFF | PSR_ABORT_MODE
	mov sp, r0
	msr cpsr_cxsf, #PSR_INT_OFF | PSR_UNDEF_MODE
	mov sp, r0
	msr cpsr_cxsf, #PSR_INT_OFF | PSR_SYS_MODE
	ldr sp, =A9_STACK_END

	bl setupMpu

	@ Clear bss section
	ldr r0, =__bss_start__
	ldr r1, =__bss_end__
	sub r2, r1, r0
	mov r1, #0
	bl iomemset
	@ Setup newlib heap
	ldr r0, =IO_MEM_ARM9_ONLY   @ CFG9 regs
	ldr r1, [r0, #0xFFC]        @ REG_CFG9_SOCINFO
	tst r1, #2                  @ Test for LGR1 bit (New 3DS prototype).
	movne r2, #1
	strne r2, [r0, #0x200]      @ REG_CFG9_EXTMEMCNT9
	ldr r0, =A9_HEAP_END
	addne r0, #A9_RAM_N3DS_EXT_SIZE
	ldr r1, =fake_heap_end
	str r0, [r1]
	blx __libc_init_array       @ Initialize ctors and dtors
	blx __systemInit

	mov r0, #0                  @ argc
	adr r1, _dummyArgv          @ argv
	blx main
	blx __systemDeinit
	_start_lp:
		mcr p15, 0, r0, c7, c0, 4 @ Wait for interrupt
		b _start_lp

.pool
_dummyArgv:
	.4byte 0
END_ASM_FUNC


#define MAKE_BRANCH(src, dst) (0xEA000000 | (((((dst) - (src)) >> 2) - 2) & 0xFFFFFF))

BEGIN_ASM_FUNC setupExceptionVectors no_section
	adr r0, _vectorStubs
	ldr r1, =A9_VECTORS_START
	ldmia r0!, {r2-r9}
	stmia r1!, {r2-r9}
	ldm r0, {r2-r5}
	stm r1, {r2-r5}
	bx lr

.pool
_vectorStubs:
	ldr pc, irqHandlerPtr
	irqHandlerPtr:                  .4byte irqHandler
	udf #2
	fiqHandlerPtr:                  .4byte (A9_VECTORS_START + 0x08)
	udf #3
	svcHandlerPtr:                  .4byte (A9_VECTORS_START + 0x10)
	ldr pc, undefInstrHandlerPtr
	undefInstrHandlerPtr:           .4byte undefInstrHandler
	ldr pc, prefetchAbortHandlerPtr
	prefetchAbortHandlerPtr:        .4byte prefetchAbortHandler
	ldr pc, dataAbortHandlerPtr
	dataAbortHandlerPtr:            .4byte dataAbortHandler
END_ASM_FUNC


BEGIN_ASM_FUNC setupTcms no_section
	ldr r1, =(ITCM_BASE | 0x24) @ Base = 0x00000000, size = 128 MiB (32 KiB mirrored)
	ldr r0, =(DTCM_BASE | 0x0A) @ Base = 0xFFF00000, size = 16 KiB
	mcr p15, 0, r0, c9, c1, 0   @ Write DTCM region reg
	mcr p15, 0, r1, c9, c1, 1   @ Write ITCM region reg
	mrc p15, 0, r0, c1, c0, 0   @ Read control register
	orr r0, r0, #0x50000        @ Enable DTCM and ITCM
	mcr p15, 0, r0, c1, c0, 0   @ Write control register
	bx lr

.pool
END_ASM_FUNC


#define REGION_4KiB   (0b01011)
#define REGION_8KiB   (0b01100)
#define REGION_16KiB  (0b01101)
#define REGION_32KiB  (0b01110)
#define REGION_64KiB  (0b01111)
#define REGION_128KiB (0b10000)
#define REGION_256KiB (0b10001)
#define REGION_512KiB (0b10010)
#define REGION_1MiB   (0b10011)
#define REGION_2MiB   (0b10100)
#define REGION_4MiB   (0b10101)
#define REGION_8MiB   (0b10110)
#define REGION_16MiB  (0b10111)
#define REGION_32MiB  (0b11000)
#define REGION_64MiB  (0b11001)
#define REGION_128MiB (0b11010)
#define REGION_256MiB (0b11011)
#define REGION_512MiB (0b11100)
#define REGION_1GiB   (0b11101)
#define REGION_2GiB   (0b11110)
#define REGION_4GiB   (0b11111)
#define MAKE_REGION(adr, size) ((adr) | ((size)<<1) | 1)

#define PER_NA             (0)
#define PER_PRIV_RW_USR_NA (0b0001)
#define PER_PRIV_RW_USR_RO (0b0010)
#define PER_PRIV_RW_USR_RW (0b0011)
#define PER_PRIV_RO_USR_NA (0b0101)
#define PER_PRIV_RO_USR_RO (0b0110)
#define MAKE_PERMISSIONS(r0, r1, r2, r3, r4, r5, r6, r7) \
        ((r0) | (r1<<4) | (r2<<8) | (r3<<12) | (r4<<16) | (r5<<20) | (r6<<24) | (r7<<28))

BEGIN_ASM_FUNC setupMpu no_section
	adr r0, _mpu_regions        @ Table at end of file
	ldm r0, {r1-r10}
	mcr p15, 0, r1, c6, c0, 0   @ Write MPU region reg 0-7
	mcr p15, 0, r2, c6, c1, 0
	mcr p15, 0, r3, c6, c2, 0
	mcr p15, 0, r4, c6, c3, 0
	mcr p15, 0, r5, c6, c4, 0
	mcr p15, 0, r6, c6, c5, 0
	mcr p15, 0, r7, c6, c6, 0
	mcr p15, 0, r8, c6, c7, 0

	mcr p15, 0, r9, c5, c0, 2   @ Write data access permissions
	mcr p15, 0, r10, c5, c0, 3  @ Write instruction access permissions

	@ Data cachable bits:
	@ Region 0 = no
	@ Region 1 = yes
	@ Region 2 = no  <-- Never cache IO regs
	@ Region 3 = yes
	@ Region 4 = yes
	@ Region 5 = no
	@ Region 6 = no
	@ Region 7 = yes
	mov r0, #0b10011010
	mcr p15, 0, r0, c2, c0, 0   @ Data cachable bits

	@ Instruction cachable bits:
	@ Region 0 = no
	@ Region 1 = yes
	@ Region 2 = no
	@ Region 3 = no
	@ Region 4 = no
	@ Region 5 = no
	@ Region 6 = no
	@ Region 7 = yes
	mov r1, #0b10000010
	mcr p15, 0, r1, c2, c0, 1   @ Instruction cachable bits

	@ Write bufferable bits:
	@ Region 0 = no
	@ Region 1 = yes
	@ Region 2 = no  <-- Never buffer IO regs
	@ Region 3 = yes
	@ Region 4 = yes
	@ Region 5 = no
	@ Region 6 = no
	@ Region 7 = yes
	@mov r2, #0b10011010        @ Same as data cachable bits
	mcr p15, 0, r0, c3, c0, 0   @ Write bufferable bits

	ldrh r1, =0x1005            @ MPU, D-Cache and I-Cache bitmask
	mrc p15, 0, r0, c1, c0, 0   @ Read control register
	orr r0, r0, r1              @ Enable MPU, D-Cache and I-Cache
	mcr p15, 0, r0, c1, c0, 0   @ Write control register
	bx lr

.pool
_mpu_regions:
	@ Region 0: ITCM kernel mirror 32 KiB
	@ Region 1: ARM9 internal mem + N3DS extension 2 MiB
	@ Region 2: IO region 2 MiB covers only ARM9 accessible regs
	@ Region 3: VRAM 8 MiB
	@ Region 4: DSP mem and AXIWRAM 1 MiB
	@ Region 5: FCRAM + N3DS extension 256 MiB
	@ Region 6: DTCM 16 KiB
	@ Region 7: Exception vectors + ARM9 bootrom 64 KiB
	.4byte MAKE_REGION(ITCM_KERNEL_MIRROR, REGION_32KiB)
	.4byte MAKE_REGION(A9_RAM_BASE,        REGION_2MiB)
	.4byte MAKE_REGION(IO_MEM_ARM9_ONLY,   REGION_2MiB)
	.4byte MAKE_REGION(VRAM_BASE,          REGION_8MiB)
	.4byte MAKE_REGION(DSP_MEM_BASE,       REGION_1MiB)
	.4byte MAKE_REGION(FCRAM_BASE,         REGION_256MiB)
	.4byte MAKE_REGION(DTCM_BASE,          REGION_16KiB)
	.4byte MAKE_REGION(BOOT9_BASE,         REGION_64KiB)
_mpu_permissions:
	@ Data access permissions:
	@ Region 0: User = --, Privileged = RW
	@ Region 1: User = --, Privileged = RW
	@ Region 2: User = --, Privileged = RW
	@ Region 3: User = --, Privileged = RW
	@ Region 4: User = --, Privileged = RW
	@ Region 5: User = --, Privileged = RW
	@ Region 6: User = --, Privileged = RW
	@ Region 7: User = --, Privileged = RO
	.4byte MAKE_PERMISSIONS(PER_PRIV_RW_USR_NA, PER_PRIV_RW_USR_NA,
	                       PER_PRIV_RW_USR_NA, PER_PRIV_RW_USR_NA,
	                       PER_PRIV_RW_USR_NA, PER_PRIV_RW_USR_NA,
	                       PER_PRIV_RW_USR_NA, PER_PRIV_RO_USR_NA)
	@ Instruction access permissions:
	@ Region 0: User = --, Privileged = RO
	@ Region 1: User = --, Privileged = RO
	@ Region 2: User = --, Privileged = --
	@ Region 3: User = --, Privileged = --
	@ Region 4: User = --, Privileged = --
	@ Region 5: User = --, Privileged = --
	@ Region 6: User = --, Privileged = --
	@ Region 7: User = --, Privileged = RO
	.4byte MAKE_PERMISSIONS(PER_PRIV_RO_USR_NA, PER_PRIV_RO_USR_NA,
	                       PER_NA,             PER_NA,
	                       PER_NA,             PER_NA,
	                       PER_NA, PER_PRIV_RO_USR_NA)
END_ASM_FUNC


@ Needed by libc
BEGIN_ASM_FUNC _init no_section thumb
	bx lr
END_ASM_FUNC


BEGIN_ASM_FUNC deinitCpu no_section
	mov r3, lr

	msr cpsr_cxsf, #PSR_INT_OFF | PSR_SYS_MODE
	@ Stub vectors to endless loops
	ldr r0, =A9_RAM_BASE
	ldr r2, =MAKE_BRANCH(0, 0)  @ Endless loop
	mov r1, #6
	deinitCpu_lp:
		str r2, [r0], #8
		subs r1, r1, #1
		bne deinitCpu_lp

	bl cleanDCache
	mov r2, #0
	ldrh r1, =0x1005            @ MPU, D-Cache and I-Cache bitmask
	mrc p15, 0, r0, c1, c0, 0   @ Read control register
	bic r0, r0, r1              @ Disable MPU, D-Cache and I-Cache
	mcr p15, 0, r0, c1, c0, 0   @ Write control register
	mcr p15, 0, r2, c7, c5, 0   @ Invalidate I-Cache
	mcr p15, 0, r2, c7, c6, 0   @ Invalidate D-Cache
	mcr p15, 0, r2, c7, c10, 4  @ Drain write buffer
	bx r3

.pool
END_ASM_FUNC
