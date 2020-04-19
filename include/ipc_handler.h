#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
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


#define IPC_MAX_PARAMS              (15)
#define IPC_CMD_RESP_FLAG           (1u<<15)
#define IPC_CMD_ID_MASK(cmd)        ((cmd)>>8)      // Max 127
#define IPC_CMD_IN_BUFS_MASK(cmd)   ((cmd)>>6 & 3u) // Max 3
#define IPC_CMD_OUT_BUFS_MASK(cmd)  ((cmd)>>4 & 3u) // Max 3
#define IPC_CMD_PARAMS_MASK(cmd)    ((cmd) & 15u)   // Max 15


#define MAKE_CMD(id, inBufs, outBufs, params)  ((id)<<8 | (inBufs)<<6 | (outBufs)<<4 | params)

typedef enum
{
	IPC_CMD9_PREPARE_AGB       = MAKE_CMD(0, 0, 0, 1),
	IPC_CMD9_PREPARE_POWER     = MAKE_CMD(1, 0, 0, 0)
} IpcCmd9;

typedef enum
{
	IPC_CMD11_PRINT_MSG        = MAKE_CMD(0, 0, 0, 0), // Invalid on purpose. Will be decided later.
	IPC_CMD11_PANIC            = MAKE_CMD(1, 0, 0, 0),
	IPC_CMD11_EXCEPTION        = MAKE_CMD(2, 0, 0, 0)
} IpcCmd11;

#undef MAKE_CMD


typedef struct
{
	void *ptr;
	u32 size;
} IpcBuffer;



u32 IPC_handleCmd(u8 cmdId, u32 inBufs, u32 outBufs, const u32 *const buf);
