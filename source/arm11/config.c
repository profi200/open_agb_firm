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

#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "arm11/config.h"
#include "inih/ini.h"
#include "util.h"
#include "fsutil.h"


#define INI_BUF_SIZE    (1024u)
#define DEFAULT_CONFIG  "[general]\n"             \
                        "backlight=64\n"          \
                        "backlightSteps=5\n"      \
                        "directBoot=false\n"      \
                        "useGbaDb=true\n\n"       \
                        "[video]\n"               \
                        "scaler=2\n"              \
                        "gbaGamma=2.2\n"          \
                        "lcdGamma=1.54\n"         \
                        "contrast=1.0\n"          \
                        "brightness=0.0\n\n"      \
                        "[audio]\n"               \
                        "audioOut=0\n"            \
                        "volume=127\n\n"          \
                        "[advanced]\n"            \
                        "saveOverride=false\n"    \
                        "defaultSave=14"



// Default config.
OafConfig g_oafConfig =
{
	// [general]
	64,    // backlight
	5,     // backlightSteps
	false, // directBoot
	true,  // useGbaDb

	// [video]
	2,     // scaler
	2.2f,  // gbaGamma
	1.54f, // lcdGamma
	1.f,   // contrast
	0.f,   // brightness

	// [audio]
	0,     // Automatic audio output.
	127,   // Control via volume slider.

	// [input]
	{      // buttonMaps
		0, // A
		0, // B
		0, // Select
		0, // Start
		0, // Right
		0, // Left
		0, // Up
		0, // Down
		0, // R
		0  // L
	},

	// [game]
	0,     // saveSlot
	0xFF,  // saveType

	// [advanced]
	false, // saveOverride
	14     // defaultSave
};



static u32 parseButtons(const char *str)
{
	if(str == NULL || *str == '\0') return 0;

	char buf[32]; // Should be enough for all useful mappings.
	buf[31] = '\0';
	strncpy(buf, str, 31);

	char *bufPtr = buf;
	static const char *const buttonStrLut[32] =
	{
		"A", "B", "SELECT", "START", "RIGHT", "LEFT", "UP", "DOWN",
		"R", "L", "X", "Y", "", "", "ZL", "ZR",
		"", "", "", "", "TOUCH", "", "", "",
		"CS_RIGHT", "CS_LEFT", "CS_UP", "CS_DOWN", "CP_RIGHT", "CP_LEFT", "CP_UP", "CP_DOWN"
	};
	u32 map = 0;
	while(1)
	{
		char *const nextDelimiter = strchr(bufPtr, ',');
		if(nextDelimiter != NULL) *nextDelimiter = '\0';

		unsigned i = 0;
		while(i < 32 && strcmp(buttonStrLut[i], bufPtr) != 0) ++i;
		if(i == 32) break;
		map |= 1u<<i;

		if(nextDelimiter == NULL) break;

		bufPtr = nextDelimiter + 1; // Skip delimiter.
	}

	// Empty strings will match the entry for bit 12.
	return map & ~(1u<<12);
}

static int cfgIniCallback(void* user, const char* section, const char* name, const char* value)
{
	OafConfig *const config = (OafConfig*)user;

	if(strcmp(section, "general") == 0)
	{
		if(strcmp(name, "backlight") == 0)
			config->backlight = (u8)strtoul(value, NULL, 10);
		else if(strcmp(name, "backlightSteps") == 0)
			config->backlightSteps = (u8)strtoul(value, NULL, 10);
		else if(strcmp(name, "directBoot") == 0)
			config->directBoot = (strcmp(value, "false") == 0 ? false : true);
		else if(strcmp(name, "useGbaDb") == 0)
			config->useGbaDb = (strcmp(value, "true") == 0 ? true : false);
	}
	else if(strcmp(section, "video") == 0)
	{
		if(strcmp(name, "scaler") == 0)
			config->scaler = (u8)strtoul(value, NULL, 10);
		else if(strcmp(name, "gbaGamma") == 0)
			config->gbaGamma = str2float(value);
		else if(strcmp(name, "lcdGamma") == 0)
			config->lcdGamma = str2float(value);
		else if(strcmp(name, "contrast") == 0)
			config->contrast = str2float(value);
		else if(strcmp(name, "brightness") == 0)
			config->brightness = str2float(value);
	}
	else if(strcmp(section, "audio") == 0)
	{
		if(strcmp(name, "audioOut") == 0)
			config->audioOut = (u8)strtoul(value, NULL, 10);
		else if(strcmp(name, "volume") == 0)
			config->volume = (s8)strtol(value, NULL, 10);
	}
	else if(strcmp(section, "input") == 0)
	{
		const u32 button = parseButtons(name) & 0x3FFu; // Only allow GBA buttons.
		if(button != 0)
		{
			// If the config option happens to abuse parseButtons() we will only use the highest bit.
			const u32 shift = 31u - __builtin_clzl(button);
			const u32 map   = parseButtons(value);
			config->buttonMaps[shift] = map;
		}
	}
	else if(strcmp(section, "game") == 0)
	{
		if(strcmp(name, "saveSlot") == 0)
			config->saveSlot = (u8)strtoul(value, NULL, 10);
		if(strcmp(name, "saveType") == 0)
			config->saveType = (u8)strtoul(value, NULL, 10);
	}
	else if(strcmp(section, "advanced") == 0)
	{
		if(strcmp(name, "saveOverride") == 0)
			config->saveOverride = (strcmp(value, "false") == 0 ? false : true);
		if(strcmp(name, "defaultSave") == 0)
			config->defaultSave = (u16)strtoul(value, NULL, 10);
	}
	else return 0; // Error.

	return 1; // 1 is no error? Really?
}

// TODO: Instead of writing a hardcoded string turn default config into a string.
Result parseOafConfig(const char *const path, OafConfig *cfg, const bool newCfgOnError)
{
	char *iniBuf = (char*)calloc(INI_BUF_SIZE, 1);
	if(iniBuf == NULL) return RES_OUT_OF_MEM;

	cfg = (cfg != NULL ? cfg : &g_oafConfig);
	Result res = fsQuickRead(path, iniBuf, INI_BUF_SIZE - 1);
	if(res == RES_OK) ini_parse_string(iniBuf, cfgIniCallback, cfg);
	else if(newCfgOnError)
	{
		const char *const defaultConfig = DEFAULT_CONFIG;
		res = fsQuickWrite(path, defaultConfig, strlen(defaultConfig));
	}

	free(iniBuf);

	return res;
}