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



noreturn void _a7_overlay_stub(void);
extern const u32 _a7_overlay_stub_size[];

noreturn void _a7_overlay_stub_capture(void);
extern const u32 _a7_overlay_stub_capture_size[];

noreturn void _a7_stub_start(void);
extern u16 _a7_stub9_swi[]; // Final ARM9 mem location.
extern const u32 _a7_stub_size[];
