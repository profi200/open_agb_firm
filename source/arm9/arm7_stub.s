.arm
.cpu arm7tdmi
.fpu softvfp

.global _arm7_stub_start
.global _arm7_stub_end

.type _arm7_stub_start %function
.type gbaReset %function
.type waitCycles %function


.align 2
_arm7_stub_start:
	mov r0, #0xD3
	msr CPSR_cxsf, r0
	ldr sp, =0x03008000
	mov r0, #0xD2
	msr CPSR_cxsf, r0
	ldr sp, =0x03007FA0
	mov r0, #0xDF
	msr CPSR_cxsf, r0
	ldr sp, =0x03007F80
	mov r3, #0x04700000
	mov r0, #1
	str r0, [r3]        @ Disable BIOS overlay.
	mov r0, #0x100
	bl  waitCycles
	ldr r0, =0x1999E8
	bl  waitCycles

	ldr r3, =0x04000006 @ REG_VCOUNT
wait_vcount_160_lp:
	ldrb r0, [r3]
	cmp  r0, #160       @ Wait for REG_VCOUNT = 160.
	bne  wait_vcount_160_lp

	mov  r0, #0xFF
	bl   gbaReset
	mov  r2, #0
	mov  r4, #0x04000000
	mov  lr, #0xBC
	bx   lr
.pool


gbaReset:
	swi 0x10000   @ RegisterRamReset
	@swi 0x260000  @ HardReset
	bx lr


waitCycles:
	subs r0, r0, #4
	bcs waitCycles
	bx lr

_arm7_stub_end:
