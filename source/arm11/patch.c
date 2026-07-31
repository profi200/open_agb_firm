/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2022 spitzeqc
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
#include "oaf_error_codes.h"
#include "util.h"
#include "arm11/open_agb_firm.h"
#include "arm11/drivers/hid.h"
#ifndef NDEBUG
#include "arm11/drivers/mcu.h"
#endif
#include "drivers/lgy_common.h"
#include "arm11/fmt.h"
#include "fs.h"
#include "arm11/patch.h"
#include "arm11/power.h"
#include "drivers/sha.h"


#define min(a, b)  ((size_t) (a) <= (size_t) (b) ? (size_t) (a) : (size_t) (b))

#define BUFFER_CAPACITY  4096
#define UPS_MAGIC_SIZE   4
#define UPS_CRC_SIZE     4 * 3

typedef struct
{
	u8 *buffer;
	u16 size;
	u16 offset;
	u16 capacity;
	u32 totalRead;
} Cache;

typedef struct
{
	FHandle handle;
	u32 size;
	u32 baseRomSize;
	u32 patchedRomSize;
} UPSPatch;



#ifndef NDEBUG
static u8 elapsedSecs(const RtcTimeDate *before, const RtcTimeDate *after)
{
	// RtcTimeDate seconds are represented as hex, i.e. the 59th second is 0x59.
	// Convert those to decimal so we can do decent math on them.
	u8 beforeSecs = (before->sec / 16 * 10) + (before->sec % 16);
	u8 afterSecs = (after->sec / 16 * 10) + (after->sec % 16);

	// NOTE: only accounts for the first minute boundary.
	if(afterSecs < beforeSecs) afterSecs += 60;

	return afterSecs - beforeSecs;
}
#endif

static u16 loadCache(const UPSPatch *patch, Cache *cache, Result *res)
{
	cache->size = min(cache->capacity, patch->size - cache->totalRead);
	*res = fRead(patch->handle, cache->buffer, cache->size, NULL);
	cache->totalRead += cache->size;  // We already computed how much will be read.
	cache->offset = 0;

	return cache->size;
}

static u8 readCache(const UPSPatch *patch, Cache *cache, Result *res)
{
	u8 byte = cache->buffer[cache->offset++];

	// Load new block of data from patch if needed.
	if(cache->offset >= cache->capacity) loadCache(patch, cache, res);

	return byte;
}

static bool hasDataLeft(const Cache *cache)
{
	return cache->offset < cache->size;
}

static Result patchIPS(const FHandle patchHandle) {
	ee_puts("IPS patch found! Patching...");

	const u16 bufferSize = BUFFER_CAPACITY;
	char *buffer = (char*)calloc(bufferSize, 1);
	if(buffer == NULL) return RES_OUT_OF_MEM;

	// Verify patch is IPS (magic number "PATCH").
	Result res = fRead(patchHandle, buffer, 5, NULL);
	if(res != RES_OK || memcmp("PATCH", buffer, 5) != 0)
	{
		free(buffer);
		return RES_INVALID_PATCH;
	}

	u32 offset = 0;
	u16 length = 0;
	while(res == RES_OK)
	{
		// Read offset.
		res = fRead(patchHandle, buffer, 3, NULL);
		if(res != RES_OK || memcmp("EOF", buffer, 3) == 0) break;
		offset = (buffer[0] << 16) + (buffer[1] << 8) + buffer[2];

		// Read length.
		res = fRead(patchHandle, buffer, 2, NULL);
		if(res != RES_OK) break;
		length = (buffer[0] << 8) + buffer[1];

		// RLE hunk.
		if(length == 0)
		{
			res = fRead(patchHandle, buffer, 3, NULL);
			if(res != RES_OK) break;

			length = (buffer[0] << 8) + buffer[1];
			memset((void*)(LGY_ROM_LOC + offset), buffer[2], length * sizeof(char));

			continue;
		}

		// Regular hunks.
		u16 fullCount = length / bufferSize;
		for(u16 i = 0; i < fullCount; ++i)
		{
			res = fRead(patchHandle, buffer, bufferSize, NULL);
			if(res != RES_OK) break;

			for(u16 j = 0; j < bufferSize; ++j)
			{
				*(char*)(LGY_ROM_LOC + offset + (bufferSize * i) + j) = buffer[j];
			}
		}

		u16 remaining = length % bufferSize;
		if(remaining == 0) continue;

		res = fRead(patchHandle, buffer, remaining, NULL);
		if(res != RES_OK) break;

		for(u16 j = 0; j < remaining; ++j)
		{
			*(char*)(LGY_ROM_LOC + offset + (bufferSize * fullCount) + j) = buffer[j];
		}
	}

	free(buffer);

	return res;
}

//based on code from http://fileformats.archiveteam.org/wiki/UPS_(binary_patch_format) (CC0, No copyright)
static u32 read_vuint(const UPSPatch *patch, Result *res, Cache *cache)
{
	u32 result = 0, shift = 0;

	u8 octet = 0;
	for (;;) {
		octet = readCache(patch, cache, res);
		if(*res != RES_OK) break;
		if(octet & 0x80) {
			result += (octet & 0x7f) << shift;
			break;
		}
		result += (octet | 0x80) << shift;
		shift += 7;
	}

	return result;
}

// This function expects that the first chunk of the UPS patch is cached.
static Result loadUPSMetadata(UPSPatch *patch, Cache *cache)
{
	// Check magic.
	if(memcmp("UPS1", cache->buffer, UPS_MAGIC_SIZE) != 0) return RES_INVALID_PATCH;
	cache->offset += UPS_MAGIC_SIZE;

	// Decode base and patched ROM sizes.
	Result res = RES_OK;
	patch->baseRomSize = read_vuint(patch, &res, cache);
	if(res != RES_OK) return res;

	patch->patchedRomSize = read_vuint(patch, &res, cache);
	if(res != RES_OK) return res;

	debug_printf("Base size:    0x%lx\nPatched size: 0x%lx\n", patch->baseRomSize, patch->patchedRomSize);

	// Patches that would result in a ROM bigger than 32MiB are invalid.
	if(patch->patchedRomSize > LGY_MAX_ROM_SIZE)
	{
		ee_puts("Patched ROM exceeds 32MiB! Skipping patching...");
		return RES_INVALID_PATCH;
	}

	return res;
}

static Result patchUPS(const FHandle patchHandle, u32 *romSize) {
	ee_puts("UPS patch found! Patching...");

	// Reject patches shorter than header + CRC hashes.
	// Compute length minus hashes when done.
	u32 patchSize = fSize(patchHandle);
	if(patchSize < UPS_MAGIC_SIZE + UPS_CRC_SIZE) return RES_INVALID_PATCH;
	patchSize -= UPS_CRC_SIZE;

	UPSPatch patch = {
		patchHandle,
		patchSize,   // Patch file size minus CRC-32 hashes.
		0,           // Base ROM size.
		0            // Patched ROM size.
	};

	// Try to perform initial caching.
	Cache cache = {
		(u8*)calloc(BUFFER_CAPACITY, 1),  // Buffer.
		min(BUFFER_CAPACITY, patch.size), // Size.
		0,                                // Offset.
		BUFFER_CAPACITY,                  // Capacity.
		0,                                // Total bytes read.
	};
	if(cache.buffer == NULL) return RES_OUT_OF_MEM;

	Result res;
	loadCache(&patch, &cache, &res);
	if(res != RES_OK)
	{
		free(cache.buffer);
		return res;
	}

	// Validate the patch and load the metadata.
	if((res = loadUPSMetadata(&patch, &cache)) != RES_OK)
	{
		free(cache.buffer);
		return res;
	}

	// Scale up ROM if needed.
	if(patch.patchedRomSize > patch.baseRomSize)
	{
		*romSize = nextPow2(patch.patchedRomSize);
		// Zero extra ROM space to be patched.
		memset((char*)(LGY_ROM_LOC + patch.baseRomSize), 0x00, patch.patchedRomSize - patch.baseRomSize);
		// Pad up to the end of the virtual cart.
		memset((char*)(LGY_ROM_LOC + patch.patchedRomSize), 0xFF, *romSize - patch.patchedRomSize);
	}

	// Patch the ROM.
	u32 offset = 0;
	u8 *romBytes = ((u8*)LGY_ROM_LOC);
	while(hasDataLeft(&cache) && res == RES_OK)
	{
		offset += read_vuint(&patch, &res, &cache);
		if(res != RES_OK) break;

		// Use the expected exact ROM size.
		while(offset < patch.patchedRomSize)
		{
			u8 mask = readCache(&patch, &cache, &res);
			if(res != RES_OK) break;

			romBytes[offset] ^= mask;
			offset++;

			// Skip to the next block of changes if this one is over.
			if(mask == 0x00) break;
		}
	}

	free(cache.buffer);
	return res;
}

Result patchRom(const char *const gamePath, u32 *romSize) {
	Result res = RES_OK;

	//if X is held during launch, skip patching
	hidScanInput();
	if(hidKeysHeld() == KEY_X)
		return res;

	//get base path for game with 'gba' extension removed
	int gamePathLength = strlen(gamePath) + 1; //add 1 for '\0' character
	const int extensionOffset = gamePathLength-4;
	char *patchPathBase = (char*)calloc(gamePathLength, 1);

	char *patchPath = (char*)calloc(512, 1);

	if(patchPathBase != NULL && patchPath != NULL) {
		strcpy(patchPathBase, gamePath);
		memset(patchPathBase+extensionOffset, '\0', 3); //replace 'gba' with '\0' characters

		FHandle f;
		//check if patch file is present. If so, call appropriate patching function
		if((res = fOpen(&f, strcat(patchPathBase, "ips"), FA_OPEN_EXISTING | FA_READ)) == RES_OK)
		{
			res = patchIPS(f);

			if(res != RES_OK && res != RES_INVALID_PATCH) {
				ee_puts("An error has occurred while patching.\nContinuing is NOT recommended!\n\nPress Y+UP to proceed");
				while(1){
					hidScanInput();
					if(hidKeysHeld() == (KEY_Y | KEY_DUP) && hidKeysDown() != 0) break;
					if(shouldExit(hidGetExtraKeys(0))) power_off();
				}
			}

			fClose(f);
			goto cleanup;
		}
		//reset patchPathBase
		memset(patchPathBase+extensionOffset, '\0', 3);

		if ((res = fOpen(&f, strcat(patchPathBase, "ups"), FA_OPEN_EXISTING | FA_READ)) == RES_OK) 
		{
#ifndef NDEBUG
			RtcTimeDate before, after;
			MCU_getRtcTimeDate(&before);
#endif
			res = patchUPS(f, romSize);
#ifndef NDEBUG
			MCU_getRtcTimeDate(&after);
			debug_printf("Patching took: %us\n", elapsedSecs(&before, &after));
#endif

			if(res != RES_OK && res != RES_INVALID_PATCH) {
				ee_puts("An error has occurred while patching.\nContinuing is NOT recommended!\n\nPress Y+UP to proceed");
				while(1){
					hidScanInput();
					if(hidKeysHeld() == (KEY_Y | KEY_DUP) && hidKeysDown() != 0) break;
					if(shouldExit(hidGetExtraKeys(0))) power_off();
				}
			}

			fClose(f);
			goto cleanup;
		}

	} else {
		res = RES_OUT_OF_MEM;
	}

cleanup:
	//cleanup our resources
	free(patchPath);
	free(patchPathBase);

	if(res == RES_INVALID_PATCH) {
		ee_puts("Patch is not valid! Skipping...\n");
	} 
#ifndef NDEBUG	
	else {
		u64 sha1[3];
		sha((u32*)LGY_ROM_LOC, *romSize, (u32*)sha1, SHA_IN_BIG | SHA_1_MODE, SHA_OUT_BIG);
		debug_printf("New hash: '%016" PRIX64 "'\n", __builtin_bswap64(sha1[0]));
	}
#endif

	return res;
}
