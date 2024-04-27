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

#include "error_codes.h"



#ifdef __cplusplus
extern "C"
{
#endif

Result oafParseConfigEarly(void);
void changeBacklight(s16 amount);
Result oafInitAndRun(void);
void oafUpdate(void);
void oafFinish(void);
void oafSleep(void);
void oafWakeup(void);

#ifdef __cplusplus
} // extern "C"
#endif