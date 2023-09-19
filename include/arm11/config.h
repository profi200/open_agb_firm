#pragma once

#include "types.h"
#include "oaf_error_codes.h"


typedef struct
{
	// [general]
	u8 backlight;       // Both LCDs.
	u8 backlightSteps;
	bool directBoot;
	bool useGbaDb;

	// [video]
	u8 scaler;          // 0 = 1:1, 1 = bilinear (GPU) x1.5, 2 = matrix (hardware) x1.5.
	float gbaGamma;
	float lcdGamma;
	float contrast;
	float brightness;

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
	u16 defaultSave;
} OafConfig;
//static_assert(sizeof(OafConfig) == 76, "nope");



Result parseOafConfig(const char *const path, OafConfig *const cfg, const bool newCfgOnError);
