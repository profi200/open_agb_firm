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
#include "mem_map.h"


#define SCU_REGS_BASE      (MPCORE_PRIV_REG_BASE)
#define REG_SCU_CNT        *((vu32*)(SCU_REGS_BASE + 0x00))
#define REG_SCU_CONFIG     *((vu32*)(SCU_REGS_BASE + 0x04))
#define REG_SCU_CPU_STAT   *((vu32*)(SCU_REGS_BASE + 0x08))
#define REG_SCU_INVAL_TAG  *((vu32*)(SCU_REGS_BASE + 0x0C))
