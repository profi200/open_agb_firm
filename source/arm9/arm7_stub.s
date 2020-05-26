#include "asm_macros.h"

.cpu arm7tdmi
.fpu softvfp



@ Must be located at 0x3007E00.
BEGIN_ASM_FUNC _arm7_stub_start
	mov r0, #0xD3
	adr r1, _arm7_stub_start + 0x200  @ 0x3008000
	msr CPSR_cxsf, r0
	mov r0, #0xD2
	mov sp, r1
	msr CPSR_cxsf, r0
	mov r0, #0xDF
	sub sp, r1, #0x60  @ 0x3007FA0
	msr CPSR_cxsf, r0
	mov r3, #0x4700000
	adr r2, _arm7_stub_16 + 1
	sub sp, r1, #0x80  @ 0x3007F80
	bx  r2

.thumb
_arm7_stub_16:
	mov r0, #1
	str r0, [r3]  @ Disable BIOS overlay.
	@ The original ARM7 stub waits 256 cycles here (for the BIOS overlay disable?).
	@ The original ARM7 stub waits 1677800 cycles (100 ms) here for LCD/LgyFb sync.

	lsl  r3, r0, #26   @ 0x4000000
wait_vcount_160_lp:
	ldrb r0, [r3, #6]  @ REG_VCOUNT
	cmp  r0, #160      @ Wait for REG_VCOUNT == 160.
	bne  wait_vcount_160_lp

	mov  r4, r3     @ Needed for function call 0xBC below.
	mov  r0, #0xFF

.global _arm7_stub_swi
_arm7_stub_swi:
	swi  0x10       @ RegisterRamReset
	@swi  0x26       @ HardReset (BIOS animation)
	mov  r0, #0xBC
	mov  r2, #0
	bx   r0

.pool
.align 2
.global _arm7_stub_end
_arm7_stub_end:
END_ASM_FUNC
