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


#ifdef __cplusplus
extern "C"
{
#endif

#define GPU_RENDER_BUF_ADDR  (0x18180000)
#define GBA_INIT_LIST_SIZE   (1136)
#define GBA_LIST2_SIZE       (448)


extern u8 gbaGpuInitList[GBA_INIT_LIST_SIZE];
extern u8 gbaGpuList2[GBA_LIST2_SIZE];



void patchGbaGpuCmdList(u8 scaleType);

#ifdef __cplusplus
} // extern "C"
#endif