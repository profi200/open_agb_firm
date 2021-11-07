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

#include "types.h"



noreturn void panic(void);
noreturn void panicMsg(const char *msg);

// Exception tests
/*static inline regTest(void)
{
	__asm__ volatile("mov r0, #1\n\tmov r1, #2\n\tmov r2, #3\n\tmov r3, #4\n\tmov r4, #5\n\t"
	                 "mov r5, #6\n\tmov r6, #7\n\tmov r7, #8\n\tmov r8, #9\n\tmov r9, #10\n\t"
	                 "mov r10, #11\n\tmov r11, #12\n\tmov r12, #13\n\tmov r13, #14\n\t"
	                 "mov r14, #15\n\tmov r15, #16\n\t" : :);
}

static inline breakpointTest(void)
{
	__asm__ volatile("bkpt #0xCAFE" : :);
}

static inline dataAbortTest(void)
{
	__asm__ volatile("mov r0, #4\n\tmov r1, #0xEF\n\tstr r1, [r0]" : :);
}

static inline undefInstrTest(void)
{
	__asm__ volatile("udf #0xDEAD" : :);
}*/
