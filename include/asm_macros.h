#pragma once

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

#if !__ASSEMBLER__
	#error Only include this in assembly files!
#endif


.macro BEGIN_ASM_FUNC name, type=arm, linkage=global, section=text
	.section        .\section\().\name, "ax", %progbits
.if \type == thumb
	.align          1
	.thumb
.else
	.align          2
	.arm
.endif
	.\linkage       \name
	.type           \name, %function
	.func           \name
	.cfi_sections   .debug_frame
	.cfi_startproc
\name:
.endm

.macro END_ASM_FUNC
	.cfi_endproc
	.endfunc
.endm
