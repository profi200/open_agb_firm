#pragma once

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