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


#define IPC_MAX_PARAMS               (15)
#define IPC_CMD_RESP_FLAG            (1u<<15)
#define IPC_CMD_ID_MASK(cmd)         ((cmd)>>8)      // Max 127
#define IPC_CMD_SEND_BUFS_MASK(cmd)  ((cmd)>>6 & 3u) // Max 3
#define IPC_CMD_RECV_BUFS_MASK(cmd)  ((cmd)>>4 & 3u) // Max 3
#define IPC_CMD_PARAMS_MASK(cmd)     ((cmd) & 15u)   // Max 15


// https://stackoverflow.com/a/52770279
// Note: __COUNTER__ is non standard.
#define MAKE_CMD9(sendBufs, recvBufs, params) ((__COUNTER__ - _CMD9_C_BASE)<<8 | (sendBufs)<<6 | (recvBufs)<<4 | params)
#define MAKE_CMD11(sendBufs, recvBufs, params) ((__COUNTER__ - _CMD11_C_BASE)<<8 | (sendBufs)<<6 | (recvBufs)<<4 | params)

enum {_CMD9_C_BASE = __COUNTER__ + 1}; // Start at 0.
typedef enum
{
	// Filesystem API.
	IPC_CMD9_FMOUNT            = MAKE_CMD9(0, 0, 1),
	IPC_CMD9_FUNMOUNT          = MAKE_CMD9(0, 0, 1),
	IPC_CMD9_FGETFREE          = MAKE_CMD9(0, 1, 1),
	IPC_CMD9_FOPEN             = MAKE_CMD9(1, 1, 1),
	IPC_CMD9_FREAD             = MAKE_CMD9(0, 2, 1),
	IPC_CMD9_FWRITE            = MAKE_CMD9(1, 1, 1),
	IPC_CMD9_FSYNC             = MAKE_CMD9(0, 0, 1),
	IPC_CMD9_FLSEEK            = MAKE_CMD9(0, 0, 2),
	IPC_CMD9_FTELL             = MAKE_CMD9(0, 0, 1),
	IPC_CMD9_FSIZE             = MAKE_CMD9(0, 0, 1),
	IPC_CMD9_FCLOSE            = MAKE_CMD9(0, 0, 1),
	IPC_CMD9_FSTAT             = MAKE_CMD9(1, 1, 0),
	IPC_CMD9_FCHDIR            = MAKE_CMD9(1, 0, 0),
	IPC_CMD9_FOPEN_DIR         = MAKE_CMD9(1, 1, 0),
	IPC_CMD9_FREAD_DIR         = MAKE_CMD9(0, 2, 2),
	IPC_CMD9_FCLOSE_DIR        = MAKE_CMD9(0, 0, 1),
	IPC_CMD9_FMKDIR            = MAKE_CMD9(1, 0, 0),
	IPC_CMD9_FRENAME           = MAKE_CMD9(2, 0, 0),
	IPC_CMD9_FUNLINK           = MAKE_CMD9(1, 0, 0),

	// open_agb_firm specific API.
	IPC_CMD9_PREPARE_GBA       = MAKE_CMD9(1, 0, 2),
	IPC_CMD9_SET_GBA_RTC       = MAKE_CMD9(0, 0, 2),
	IPC_CMD9_GET_GBA_RTC       = MAKE_CMD9(0, 1, 0),
	IPC_CMD9_BACKUP_GBA_SAVE   = MAKE_CMD9(0, 0, 0),
	//IPC_CMD9_TEST              = MAKE_CMD9(0, 0, 0),

	// Miscellaneous API.
	IPC_CMD9_PREPARE_POWER     = MAKE_CMD9(0, 0, 0)  // Also used for panic() and guruMeditation().
} IpcCmd9;

enum {_CMD11_C_BASE = __COUNTER__ + 1}; // Start at 0.
typedef enum
{
	// Miscellaneous API.
	IPC_CMD11_PRINT_MSG        = MAKE_CMD11(0, 0, 0), // Invalid on purpose. Will be decided later.
	IPC_CMD11_PANIC            = MAKE_CMD11(0, 0, 0),
	IPC_CMD11_EXCEPTION        = MAKE_CMD11(0, 0, 0)
} IpcCmd11;

#undef MAKE_CMD9
#undef MAKE_CMD11


typedef struct
{
	void *ptr;
	u32 size;
} IpcBuffer;



u32 IPC_handleCmd(u8 cmdId, u32 sendBufs, u32 recvBufs, const u32 *const buf);
