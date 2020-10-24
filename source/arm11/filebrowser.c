#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "error_codes.h"
#include "fs.h"
#include "arm11/hardware/hid.h"
#include "arm11/fmt.h"
#include "hardware/gfx.h"


#define MAX_DIR_ENTRIES  (510u)
#define DIR_READ_BLOCKS  (10u)
#define SCREEN_COLS      (53u - 1) // - 1 because the console inserts a newline after the last line otherwise.
#define SCREEN_ROWS      (24u)


typedef struct
{
	u8 type;       // 0 = file, 1 = dir
	char str[255];
} DirListEnt;



// num including null terminator.
static size_t safeStrcpy(char *const dst, const char *const src, size_t num)
{
	if(num == 0) return 0;

	const size_t len = strlen(src) + 1;
	if(len > num)
	{
		*dst = '\0';
		return 1;
	}

	strcpy(dst, src);
	return len;
}

int dlistCompare(const void *a, const void *b)
{
	const DirListEnt *const entA = (DirListEnt*)a;
	const DirListEnt *const entB = (DirListEnt*)b;

	if(entA->type != entB->type) return (int)entB->type - entA->type;

	const char *strA = entA->str;
	const char *strB = entB->str;
	int res = *strA - *strB;
	while(*strA != '\0' && *strB != '\0' && res == 0) res = *++strA - *++strB;

	return res;
}

static Result scanDir(const char *const path, DirListEnt *const dList, u32 *num, const char *const filter)
{
	FILINFO *const fi = (FILINFO*)malloc(sizeof(FILINFO) * DIR_READ_BLOCKS);
	if(fi == NULL) return RES_OUT_OF_MEM;

	*num = 0;

	Result res;
	DHandle dh;
	if((res = fOpenDir(&dh, path)) == RES_OK)
	{
		u32 read;
		u32 dListPos = 0;
		const u32 filterLen = strlen(filter);
		do
		{
			if((res = fReadDir(dh, fi, DIR_READ_BLOCKS, &read)) != RES_OK) break;
			if(dListPos + read > MAX_DIR_ENTRIES) break;

			for(u32 i = 0; i < read; i++)
			{
				const u8 isDir = (fi[i].fattrib & AM_DIR ? 1u : 0u);
				if(isDir == 0) // File
				{
					const u32 entLen = strlen(fi[i].fname);
					if(entLen <= filterLen || strcmp(filter, fi[i].fname + entLen - filterLen) != 0)
						continue;
				}

				dList[dListPos].type = isDir;
				safeStrcpy(dList[dListPos].str, fi[i].fname, 255);
				dListPos++;
			}
		} while(read == DIR_READ_BLOCKS);
		*num = dListPos;

		fCloseDir(dh);
	}

	free(fi);

	qsort(dList, *num, sizeof(DirListEnt), dlistCompare);

	return res;
}

static void showDirList(const DirListEnt *const dList, u32 num, u32 start)
{
	// Clear screen.
	ee_printf("\x1b[2J");

	const u32 listLength = (num - start > SCREEN_ROWS ? start + SCREEN_ROWS : num);
	for(u32 i = start; i < listLength; i++)
	{
		const char *const printStr =
			(dList[i].type == 0 ? "\x1b[%lu;H\x1b[37m %.51s" : "\x1b[%lu;H\x1b[33m %.51s");

		ee_printf(printStr, i - start, dList[i].str);
	}
}

Result browseFiles(const char *const basePath, char selected[512])
{
	if(basePath == NULL || selected == NULL) return RES_INVALID_ARG;
	// TODO: Check if the base path is empty.

	char *curDir = (char*)malloc(512);
	if(curDir == NULL) return RES_OUT_OF_MEM;
	safeStrcpy(curDir, basePath, 512);

	DirListEnt *const dList = (DirListEnt*)malloc(sizeof(DirListEnt) * MAX_DIR_ENTRIES);
	if(dList == NULL) return RES_OUT_OF_MEM;

	Result res;
	u32 num;
	if((res = scanDir(curDir, dList, &num, ".gba")) != RES_OK) goto end;
	showDirList(dList, num, 0);

	s32 cursorPos = 0; // Within the entire list.
	u32 windowPos = 0; // Window start position within the list.
	s32 oldCursorPos = 0;
	while(1)
	{
		ee_printf("\x1b[%lu;H ", oldCursorPos - windowPos);      // Clear old cursor.
		ee_printf("\x1b[%lu;H\x1b[37m>", cursorPos - windowPos); // Draw cursor.

		u32 kDown;
		do
		{
			GFX_waitForVBlank0();

			hidScanInput();
			if(hidGetExtraKeys(0) & (KEY_POWER_HELD | KEY_POWER)) goto end;
			kDown = hidKeysDown();
		} while(kDown == 0);

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
			showDirList(dList, num, windowPos);
		}
		if((u32)cursorPos >= windowPos + SCREEN_ROWS)
		{
			windowPos = cursorPos - (SCREEN_ROWS - 1);
			showDirList(dList, num, windowPos);
		}

		if(kDown & (KEY_A | KEY_B))
		{
			u32 pathLen = strlen(curDir);

			if(kDown & KEY_A && num != 0)
			{
				// TODO: !!! Insecure !!!
				if(curDir[pathLen - 1] != '/') curDir[pathLen++] = '/';
				safeStrcpy(curDir + pathLen, dList[cursorPos].str, 255);

				if(dList[cursorPos].type == 0)
				{
					safeStrcpy(selected, curDir, 512);
					break;
				}
			}
			if(kDown & KEY_B)
			{
				char *tmpPathPtr = curDir + pathLen;
				while(*--tmpPathPtr != '/');
				if(*(tmpPathPtr - 1) == ':') tmpPathPtr++;
				*tmpPathPtr = '\0';
			}

			if((res = scanDir(curDir, dList, &num, ".gba")) != RES_OK) break;
			cursorPos = 0;
			windowPos = 0;
			showDirList(dList, num, 0);
		}
	}

end:
	free(dList);
	free(curDir);

	// Clear screen.
	ee_printf("\x1b[2J");

	return res;
}
