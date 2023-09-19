#pragma once

/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2023 profi200
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

#include <assert.h>
#include "types.h"
#include "arm11/config.h"


typedef struct
{
	u8 sha1[20];
	char serial[4];
	u32 attr;
} GbaDbEntry;
static_assert(sizeof(GbaDbEntry) == 28, "Error: GBA DB entry struct is not packed!");



u16 detectSaveType(const u32 romSize, const u16 defaultSave);
u16 getSaveType(const OafConfig *const cfg, const u32 romSize, const char *const savePath);