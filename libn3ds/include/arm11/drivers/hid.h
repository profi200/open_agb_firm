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

/*
 *  Based on code from https://github.com/smealum/ctrulib
 */

#include "types.h"
#include "mem_map.h"


#define HID_REGS_BASE   (IO_MEM_ARM9_ARM11 + 0x46000)
#define REG_HID_PAD     (*((vu16*)(HID_REGS_BASE + 0x0)) ^ 0xFFFu)
#define REG_HID_PADCNT  *((vu16*)(HID_REGS_BASE + 0x2))


enum
{
	KEY_A            = 1u<<0,  // A
	KEY_B            = 1u<<1,  // B
	KEY_SELECT       = 1u<<2,  // Select
	KEY_START        = 1u<<3,  // Start
	KEY_DRIGHT       = 1u<<4,  // D-Pad Right
	KEY_DLEFT        = 1u<<5,  // D-Pad Left
	KEY_DUP          = 1u<<6,  // D-Pad Up
	KEY_DDOWN        = 1u<<7,  // D-Pad Down
	KEY_R            = 1u<<8,  // R
	KEY_L            = 1u<<9,  // L
	KEY_X            = 1u<<10, // X
	KEY_Y            = 1u<<11, // Y
	KEY_ZL           = 1u<<14, // ZL (New 3DS only)
	KEY_ZR           = 1u<<15, // ZR (New 3DS only)
	KEY_TOUCH        = 1u<<20, // Touch (Not actually provided by HID)
	KEY_CSTICK_RIGHT = 1u<<24, // C-Stick Right (New 3DS only)
	KEY_CSTICK_LEFT  = 1u<<25, // C-Stick Left (New 3DS only)
	KEY_CSTICK_UP    = 1u<<26, // C-Stick Up (New 3DS only)
	KEY_CSTICK_DOWN  = 1u<<27, // C-Stick Down (New 3DS only)
	KEY_CPAD_RIGHT   = 1u<<28, // Circle Pad Right
	KEY_CPAD_LEFT    = 1u<<29, // Circle Pad Left
	KEY_CPAD_UP      = 1u<<30, // Circle Pad Up
	KEY_CPAD_DOWN    = 1u<<31, // Circle Pad Down

	// Generic catch-all directions
	KEY_UP    = KEY_DUP    | KEY_CPAD_UP,    // D-Pad Up or Circle Pad Up
	KEY_DOWN  = KEY_DDOWN  | KEY_CPAD_DOWN,  // D-Pad Down or Circle Pad Down
	KEY_LEFT  = KEY_DLEFT  | KEY_CPAD_LEFT,  // D-Pad Left or Circle Pad Left
	KEY_RIGHT = KEY_DRIGHT | KEY_CPAD_RIGHT, // D-Pad Right or Circle Pad Right

	// Masks
	KEY_DPAD_MASK   = KEY_DDOWN       | KEY_DUP       | KEY_DLEFT       | KEY_DRIGHT,
	KEY_CSTICK_MASK = KEY_CSTICK_DOWN | KEY_CSTICK_UP | KEY_CSTICK_LEFT | KEY_CSTICK_RIGHT,
	KEY_CPAD_MASK   = KEY_CPAD_DOWN   | KEY_CPAD_UP   | KEY_CPAD_LEFT   | KEY_CPAD_RIGHT
};

// Extra keys use with hidGetExtraKeys()
enum
{
	KEY_POWER        = 1u<<0,
	KEY_POWER_HELD   = 1u<<1,
	KEY_HOME         = 1u<<2, // Auto clears on release
	KEY_WIFI         = 1u<<3,
	KEY_SHELL        = 1u<<4, // Auto clears on open
	KEY_BAT_CHARGING = 1u<<5, // Auto clears when charging stops
	KEY_VOL_SLIDER   = 1u<<6
};

typedef struct
{
	u16 x;
	u16 y;
} TouchPos;

typedef struct
{
	s16 x;
	s16 y;
} CpadPos;



void hidInit(void);
void hidScanInput(void);
u32 hidKeysHeld(void);
u32 hidKeysDown(void);
u32 hidKeysUp(void);
const TouchPos* hidGetTouchPosPtr(void);
const CpadPos* hidGetCpadPosPtr(void);
u32 hidGetExtraKeys(u32 clearMask);
