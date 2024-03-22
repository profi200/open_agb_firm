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
#include "arm11/drivers/hid.h"
#include "drivers/lgy_common.h"
#include "arm11/fmt.h"
#include "fs.h"
#include "arm11/patch.h"
#include "arm11/power.h"
#include "drivers/sha.h"


#define min(a, b)  ((size_t) (a) <= (size_t) (b) ? (size_t) (a) : (size_t) (b))


typedef struct
{
	u8 *buffer;
	u16 cacheSize;
	u16 cacheOffset;
	u16 maxCacheSize;
} Cache;



static u8 readCache(const FHandle patchHandle, Cache *cache, Result *res) {
	u8 result = (cache->buffer)[(cache->cacheOffset)++];
	if((cache->cacheOffset) >= (cache->cacheSize)) {
		(cache->cacheSize) = min((cache->maxCacheSize), ((fSize(patchHandle)-12) - fTell(patchHandle)));
		*res = fRead(patchHandle, (cache->buffer), (cache->cacheSize), NULL);
		(cache->cacheOffset) = 0;
	}

	return result;
}

static Result patchIPS(const FHandle patchHandle) {
	ee_puts("IPS patch found! Patching...");

	const u16 bufferSize = 512;
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
static uintmax_t read_vuint(const FHandle patchFile, Result *res, Cache *cache) {
	uintmax_t result = 0, shift = 0;

	uint8_t octet = 0;
	for (;;) {
		//*res = fRead(patchFile, &octet, 1, NULL);
		octet = readCache(patchFile, cache, res);
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

static Result patchUPS(const FHandle patchHandle, u32 *romSize) {
	Result res = RES_OK;

	Cache cache = {
		(u8*)calloc(512, 1), //buffer
		0,                   //cache size
		0,                   //cache offset
		512                  //max cache size
	};
	if(cache.buffer == NULL) {
		return RES_OUT_OF_MEM;
	}

	//read data into cache for first time
	cache.cacheSize = min(cache.maxCacheSize, (fSize(patchHandle)-12)-fTell(patchHandle));
	res = fRead(patchHandle, cache.buffer, cache.cacheSize, NULL);
	if(res != RES_OK) { free(cache.buffer); return res; }

	ee_puts("UPS patch found! Patching...");

	//verify patch is UPS (magic number is "UPS1")
	u8 magic[] = {0x00, 0x00, 0x00, 0x00}; 
	bool isValidPatch = false;
	for(u8 i=0; i<4; i++) {
		magic[i] = readCache(patchHandle, &cache, &res);
		if(res != RES_OK) break;
	}

	if(res == RES_OK) {
		if(memcmp(&magic, "UPS1", 4) == 0) {
			isValidPatch = true;
		} else {
			res = RES_INVALID_PATCH;
		}
	}

	if(isValidPatch) {
		//get rom size
		u32 baseRomSize = (u32)read_vuint(patchHandle, &res, &cache);
		if(res != RES_OK) { free(cache.buffer); return res; }
		//get patched rom size
		u32 patchedRomSize = (u32)read_vuint(patchHandle, &res, &cache);
		if(res != RES_OK) { free(cache.buffer); return res; }

		debug_printf("Base size:    0x%lx\nPatched size: 0x%lx\n", baseRomSize, patchedRomSize);

		if(patchedRomSize > baseRomSize) {
			//scale up rom
			*romSize = nextPow2(patchedRomSize);
			//check if upscaled rom is too big
			if(*romSize > LGY_MAX_ROM_SIZE) {
				ee_puts("Patched ROM exceeds 32MB! Skipping patching...");
				free(cache.buffer);
				return RES_INVALID_PATCH; 
			}

			memset((char*)(LGY_ROM_LOC + baseRomSize), 0xFFu, *romSize - baseRomSize); //fill out extra rom space
			memset((char*)(LGY_ROM_LOC + baseRomSize), 0x00u, patchedRomSize - baseRomSize); //fill new patch area with 0's
		}

		uintmax_t patchFileSize = fSize(patchHandle);

		uintmax_t offset = 0;
		u8 readByte = 0;
		u8 *romBytes = ((u8*)LGY_ROM_LOC);

		while(fTell(patchHandle) < (patchFileSize-12) && res==RES_OK) {
			offset += read_vuint(patchHandle, &res, &cache);
			if(res != RES_OK) break;

			while(offset<*romSize) {
				readByte = readCache(patchHandle, &cache, &res);
				if(res != RES_OK) break;

				if(readByte == 0x00) {
					offset++;
					break; 
				}
				romBytes[offset] ^= readByte;
				offset++;
			}
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
					if(hidGetExtraKeys(0) & (KEY_POWER_HELD | KEY_POWER)) power_off();
				}
			}

			fClose(f);
			goto cleanup;
		}
		//reset patchPathBase
		memset(patchPathBase+extensionOffset, '\0', 3);

		if ((res = fOpen(&f, strcat(patchPathBase, "ups"), FA_OPEN_EXISTING | FA_READ)) == RES_OK) 
		{
			res = patchUPS(f, romSize);

			if(res != RES_OK && res != RES_INVALID_PATCH) {
				ee_puts("An error has occurred while patching.\nContinuing is NOT recommended!\n\nPress Y+UP to proceed");
				while(1){
					hidScanInput();
					if(hidKeysHeld() == (KEY_Y | KEY_DUP) && hidKeysDown() != 0) break;
					if(hidGetExtraKeys(0) & (KEY_POWER_HELD | KEY_POWER)) power_off();
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
