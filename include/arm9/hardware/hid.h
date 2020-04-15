#pragma once

/*
 * This code is part of ctrulib (https://github.com/smealum/ctrulib)
 */

#include "types.h"


#define HID_KEY_MASK_ALL          ((KEY_Y << 1) - 1)
#define HID_VERBOSE_MODE_BUTTONS  (KEY_SELECT | KEY_START)

enum
{
	KEY_A       = 1u,
	KEY_B       = 1u<<1,
	KEY_SELECT  = 1u<<2,
	KEY_START   = 1u<<3,
	KEY_DRIGHT  = 1u<<4,
	KEY_DLEFT   = 1u<<5,
	KEY_DUP     = 1u<<6,
	KEY_DDOWN   = 1u<<7,
	KEY_R       = 1u<<8,
	KEY_L       = 1u<<9,
	KEY_X       = 1u<<10,
	KEY_Y       = 1u<<11
};



void hidScanInput(void);
u32 hidKeysHeld(void);
u32 hidKeysDown(void);
u32 hidKeysUp(void);
