#pragma once

/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2024 profi200
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
#include "oaf_error_codes.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define OAF_WORK_DIR        "sdmc:/3ds/open_agb_firm"
#define OAF_SAVE_DIR        "saves"       // Relative to work dir.
#define OAF_SCREENSHOT_DIR  "screenshots" // Relative to work dir.


typedef struct
{
	// [general]
	u8 backlight;       // Both LCDs.
	u8 backlightSteps;
	bool directBoot;
	bool useGbaDb;
	bool useSavesFolder;

	// [video]
	u8 scaler;          // 0 = 1:1/none, 1 = bilinear (GPU) x1.5, 2 = matrix (hardware) x1.5.
	u8 colorProfile;    // 0 = none, 1 = GBA, 2 = GB micro, 3 = GBA SP (AGS-101), 4 = DS phat, 5 = DS lite, 6 = Nintendo Switch Online, 7 = Visual Boy Advance/No$GBA, 8 = identity.
	float contrast;     // Range 0.0-1.0.
	float brightness;   // Range 0.0-1.0.
	float saturation;   // Range 0.0-1.0.

	// [audio]
	u8 audioOut;        // 0 = auto, 1 = speakers, 2 = headphones.
	s8 volume;          // -128 = muted, -127 - 48 = -63.5 - +24 dB.
	                    // Higher than 48 = volume control via slider.

	// [input]
	u32 buttonMaps[10]; // A, B, Select, Start, Right, Left, Up, Down, R, L.

	// [game]
	u8 saveSlot;
	u8 saveType;

	// [advanced]
	bool saveOverride;
	u16 defaultSave; // TODO: Should be u8. Investigate if u8 has any downsides.
} OafConfig;

extern OafConfig g_oafConfig; // Global config in config.c.



Result parseOafConfig(const char *const path, OafConfig *cfg, const bool newCfgOnError);

#ifdef __cplusplus
} // extern "C"
#endif