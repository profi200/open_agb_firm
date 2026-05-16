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

#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "error_codes.h"
#include "fs.h"
#include "util.h"
#include "arm11/allocator/fcram.h"
#include "arm11/drivers/hid.h"
#include "arm11/fmt.h"
#include "drivers/gfx.h"


#define DIR_READ_BLOCKS   (10u)
#define SCREEN_COLS       (53u - 1) // - 1 because the console inserts a newline after the last line otherwise.
#define SCREEN_ROWS       (24u)
#define DLIST_GROW_SIZE   (128u)

enum DirEntryType {
	ENTRY_FILE = 0,
	ENTRY_DIRECTORY = 1
};

typedef struct __attribute__((__packed__)) {
	char type;
	char *name;
} DirEntry;

typedef struct {
	u32 capacity;
	u32 size;
	DirEntry entries[];
} DirList;


DirList *dlistNew()
{
	DirList *dlist = fcramAlloc(sizeof(DirList) + (sizeof(DirEntry) * DLIST_GROW_SIZE));
	if (!dlist)
		return NULL;
	dlist->capacity = DLIST_GROW_SIZE;
	dlist->size = 0;
	return dlist;
}

void dlistFree(DirList *dlist)
{
	for (u32 i = 0; i < dlist->size; i++)
	{
		fcramFree(dlist->entries[i].name);
	}
	fcramFree(dlist);
}

DirList *dlistGrow(DirList *dlist)
{
	size_t newCapacity = dlist->capacity + DLIST_GROW_SIZE;
	DirList *newList = fcramAlloc(sizeof(DirList) + (sizeof(DirEntry) * newCapacity));
	if (!newList)
	{
		fcramFree(dlist);
		return NULL;
	}
	newList->capacity = newCapacity;
	memcpy(newList->entries, dlist->entries, sizeof(DirEntry) * dlist->size);
	fcramFree(dlist);
	return newList;
}

int dlistCompare(const void *a, const void *b)
{
	const DirEntry *entA = (const DirEntry*)a;
	const DirEntry *entB = (const DirEntry*)b;

	// Compare the entry type. Dirs have priority over files.
	if(entA->type != entB->type) return (int)entB->type - (int)entA->type;
/*
	// Compare the string.
	int res;
	const char *nameA = entA->name;
	const char *nameB = entB->name;
	do
	{
		res = *++nameA - *++nameB;
	} while(res == 0 && *nameA != '\0' && *nameB != '\0');

	return res;
*/
	return strcmp(entA->name, entB->name);
}

static Result scanDir(const char *const path, const char *const filter, DirList **dListOut)
{
    if(dListOut == NULL) return RES_INVALID_ARG;
    if(*dListOut != NULL) {
        fcramFree(*dlistOut);
        *dlistOut = NULL;
    }
	FILINFO *const fis = (FILINFO*)fcramAlloc(sizeof(FILINFO) * DIR_READ_BLOCKS);
	if(fis == NULL) return RES_OUT_OF_MEM;
	DirList *dList = dlistNew();
	if(!dList) return RES_OUT_OF_MEM;

	Result res;
	DHandle dh;
	if((res = fOpenDir(&dh, path)) == RES_OK)
	{
		u32 read;           // Number of entries read by fReadDir().
		u32 numEntries = 0; // Total number of processed entries.
		const u32 filterLen = strlen(filter);
		do
		{
			if((res = fReadDir(dh, fis, DIR_READ_BLOCKS, &read)) != RES_OK) break;

			for(u32 i = 0; i < read; i++)
			{
				const char entType = (fis[i].fattrib & AM_DIR ? ENTRY_DIRECTORY : ENTRY_FILE);
				const u32 nameLen = strlen(fis[i].fname);
				if(entType == ENTRY_FILE)
				{
					if(nameLen <= filterLen || strcmp(filter, fis[i].fname + nameLen - filterLen) != 0
					   || fis[i].fname[0] == '.')
						continue;
				}
				if (dList->size == dList->capacity)
				{
					dList = dlistGrow(dList);
					if (!dList)
					{
						res = RES_OUT_OF_MEM;
						goto bail;
					}
				}
				dList->entries[numEntries].name = (char*)fcramAlloc(nameLen + 1);
				if(!dList->entries[numEntries].name)
				{
					res = RES_OUT_OF_MEM;
					goto bail;
				}
				safeStrcpy(dList->entries[numEntries].name, fis[i].fname, nameLen + 1);
				dList->entries[numEntries].type = entType;
				dList->size = ++numEntries;
			}
		} while(read != 0);

		fCloseDir(dh);
	}

	fcramFree(fis);

	qsort(dList->entries, dList->size, sizeof(DirEntry), dlistCompare);

	*dListOut = dList;

	return res;
bail:
	fCloseDir(dh);
	fcramFree(fis);
	dlistFree(dList);
	return res;
}

static void showDirList(const DirList *const dList, u32 start)
{
	// Clear screen.
	ee_printf("\x1b[2J");

	const u32 listLength = (dList->size - start > SCREEN_ROWS ? start + SCREEN_ROWS : dList->size);
	for(u32 i = start; i < listLength; i++)
	{
		const char *const printStr =
			(dList->entries[i].type == ENTRY_FILE ? "\x1b[%lu;H\x1b[37;1m %.52s" : "\x1b[%lu;H\x1b[33;1m %.52s");

		ee_printf(printStr, i - start + 1, dList->entries[i].name);
	}
}

char *pathAppend(char *src, size_t *srcSize, char *dst)
{
	if (!src || !dst || !srcSize) return NULL;
	size_t srcLen = strlen(src);
	size_t dstLen = strlen(dst);
	bool addSlash = src[srcLen - 1] != '/';
	size_t newLen = srcLen + dstLen + (addSlash ? 2 : 1);
	if (newLen >= *srcSize)
	{
		char *newSrc = (char*)realloc(src, newLen);
		if (!newSrc)  goto bail;
		src = newSrc;
		*srcSize = newLen;
	}
	if (addSlash)
		src[srcLen++] = '/';
	memcpy(src + srcLen, dst, dstLen);
	src[srcLen + dstLen] = '\0';
	return src;
bail:
	free(src);
	return NULL;
}

Result browseFiles(const char *const basePath, char **selected, char **lastPath)
{
	if(basePath == NULL || selected == NULL || lastPath == NULL) return RES_INVALID_ARG;

	size_t curDirCapacity = strlen(basePath) + 1;
	char *curDir = (char*)malloc(curDirCapacity);
	if(curDir == NULL) return RES_OUT_OF_MEM;
	safeStrcpy(curDir, basePath, curDirCapacity);

	DirList *dList = NULL;
	Result res;
	if((res = scanDir(curDir, ".gba", &dList)) != RES_OK) goto end;
	showDirList(dList, 0);

	s32 cursorPos = 0; // Within the entire list.
	u32 windowPos = 0; // Window start position within the list.
	s32 oldCursorPos = 0;
	while(1)
	{
		ee_printf("\x1b[%lu;H ", oldCursorPos - windowPos + 1);      // Clear old cursor.
		ee_printf("\x1b[%lu;H\x1b[37m>", cursorPos - windowPos + 1); // Draw cursor.
		GFX_flushBuffers();

		u32 kDown;
		do
		{
			GFX_waitForVBlank0();

			hidScanInput();
			if(hidGetExtraKeys(0) & (KEY_POWER_HELD | KEY_POWER)) goto end;
			kDown = hidKeysDown();
		} while(kDown == 0);

		const u32 num = dList->size;
		if(num != 0)
		{
			oldCursorPos = cursorPos;
			if(kDown & KEY_DRIGHT)
			{
				cursorPos += SCREEN_ROWS;
				if((u32)cursorPos > num) cursorPos = num - 1;
			}
			if(kDown & KEY_DLEFT)
			{
				cursorPos -= SCREEN_ROWS;
				if(cursorPos < -1) cursorPos = 0;
			}
			if(kDown & KEY_DUP)    cursorPos -= 1;
			if(kDown & KEY_DDOWN)  cursorPos += 1;
		}

		if(cursorPos < 0)              cursorPos = num - 1; // Wrap to end of list.
		if((u32)cursorPos > (num - 1)) cursorPos = 0;       // Wrap to start of list.

		if((u32)cursorPos < windowPos)
		{
			windowPos = cursorPos;
			showDirList(dList, windowPos);
		}
		if((u32)cursorPos >= windowPos + SCREEN_ROWS)
		{
			windowPos = cursorPos - (SCREEN_ROWS - 1);
			showDirList(dList, windowPos);
		}

		if(kDown & (KEY_A | KEY_B))
		{

			if(kDown & KEY_A && num != 0)
			{

				if(dList->entries[cursorPos].type == ENTRY_FILE)
				{
					char *lastPathBuf = malloc(strlen(curDir) + 1);
					if(!lastPathBuf)
					{
						res = RES_OUT_OF_MEM;
						goto end;
					}
					strcpy(lastPathBuf, curDir);
					curDir = pathAppend(curDir, &curDirCapacity, dList->entries[cursorPos].name);
					if (!curDir)
					{
						free(lastPathBuf);
						res = RES_OUT_OF_MEM;
						goto end;
					}
					*selected = curDir;
					*lastPath = lastPathBuf;
					curDir = NULL;
					break;
				} else {
				}

				curDir = pathAppend(curDir, &curDirCapacity, dList->entries[cursorPos].name);
				if (!curDir)
				{
					res = RES_OUT_OF_MEM;
					goto end;
				}
			}
			if(kDown & KEY_B)
			{
				char *tmpPathPtr = curDir + strlen(curDir);
				while(*--tmpPathPtr != '/');
				if(*(tmpPathPtr - 1) == ':') tmpPathPtr++;
				*tmpPathPtr = '\0';
			}

			if((res = scanDir(curDir, ".gba", &dList)) != RES_OK) break;
			cursorPos = 0;
			windowPos = 0;
			showDirList(dList, 0);
		}
	}

end:
	if (dList != NULL)
	{
		dlistFree(dList);
	}
	if (curDir != NULL)
	{
		free(curDir);
	}
	// Clear screen.
	ee_printf("\x1b[2J");

	return res;
}
