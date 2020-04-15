#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2018 derrek, profi200
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

#include "mem_map.h"


#define CFG9_REGS_BASE      (IO_MEM_ARM9_ONLY)
#define REG_CFG9_SYSPROT9   *((vu8* )(CFG9_REGS_BASE + 0x00000))
#define REG_CFG9_SYSPROT11  *((vu8* )(CFG9_REGS_BASE + 0x00001))
#define REG_CFG9_SOCINFO    *((vu16*)(CFG9_REGS_BASE + 0x00FFC))
#define REG_CFG9_BOOTENV    *((vu32*)(CFG9_REGS_BASE + 0x10000))
#define REG_CFG9_UNITINFO   *((vu8* )(CFG9_REGS_BASE + 0x10010))
