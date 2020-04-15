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


#define CFG11_REGS_BASE                (IO_MEM_ARM9_ARM11 + 0x40000)
#define REG_CFG11_FIQ_CNT              *((vu8* )(CFG11_REGS_BASE + 0x0104))
#define REG_CFG11_SPI_CNT              *((vu16*)(CFG11_REGS_BASE + 0x01C0))
#define REG_UNK_10140400               *((vu8* )(CFG11_REGS_BASE + 0x0400))
#define REG_UNK_10140410               *((vu32*)(CFG11_REGS_BASE + 0x0410))
#define REG_CFG11_BOOTROM_OVERLAY_CNT  *((vu8* )(CFG11_REGS_BASE + 0x0420))
#define REG_CFG11_BOOTROM_OVERLAY_VAL  *((vu32*)(CFG11_REGS_BASE + 0x0424))
#define REG_CFG11_SOCINFO              *((vu16*)(CFG11_REGS_BASE + 0x0FFC))
#define REG_CFG11_MPCORE_CLKCNT        *((vu16*)(CFG11_REGS_BASE + 0x1300))
#define REG_CFG11_MPCORE_CNT           *((vu16*)(CFG11_REGS_BASE + 0x1304))
#define REGs_CFG11_MPCORE_BOOTCNT       ((vu8* )(CFG11_REGS_BASE + 0x1310))
