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
#define SCREEN_COLS      (52u)
#define SCREEN_ROWS      (24u)


typedef struct
{
	u32 num;
	const char *strPtrs[MAX_DIR_ENTRIES];
	u8 entTypes[MAX_DIR_ENTRIES]; // 0 = file, 1 = dir
	char strBufs[MAX_DIR_ENTRIES][256];
} DirList;



// num including null terminator.
size_t safeStrcpy(char *const dst, const char *const src, size_t num)
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

static Result scanDir(const char *const path, DirList *const dList)
{
	FILINFO *const fi = (FILINFO*)malloc(DIR_READ_BLOCKS * sizeof(FILINFO));
	if(fi == NULL) return RES_OUT_OF_MEM;

	memset(dList, 0, sizeof(DirList));

	Result res;
	DHandle dh;
	if((res = fOpenDir(&dh, path)) == RES_OK)
	{
		u32 read;
		u32 totalRead = 0;
		do
		{
			if((res = fReadDir(dh, fi, DIR_READ_BLOCKS, &read)) != RES_OK) break;
			if(totalRead + read > MAX_DIR_ENTRIES) break;

			for(u32 i = 0; i < read; i++)
			{
				const u32 dListPos = totalRead + i;
				dList->strPtrs[dListPos] = dList->strBufs[dListPos];

				// Mark as dir.
				if(fi[i].fattrib & AM_DIR) dList->entTypes[dListPos] = 1;

				safeStrcpy(dList->strBufs[dListPos], fi[i].fname, 256);
			}

			totalRead += read;
		} while(read == DIR_READ_BLOCKS);
		dList->num = totalRead;

		fCloseDir(dh);
	}

	free(fi);

	return res;
}

static void showDirList(const DirList *const dList, u32 start)
{
	// Clear screen.
	ee_printf("\x1b[2J");

	const u32 listLength = (dList->num - start > SCREEN_ROWS ? start + SCREEN_ROWS : dList->num);
	for(u32 i = start; i < listLength; i++)
	{
		const char *const printStr =
			(dList->entTypes[i] == 0 ? "\x1b[%lu;H\x1b[37m %.51s" : "\x1b[%lu;H\x1b[33m %.51s");

		ee_printf(printStr, i - start, dList->strPtrs[i]);
	}
}

// TODO: Handle empty dirs.
Result browseFiles(const char *const basePath, char selected[512])
{
	if(basePath == NULL || selected == NULL) return RES_INVALID_ARG;

	char *curDir = (char*)malloc(512);
	if(curDir == NULL) return RES_OUT_OF_MEM;
	safeStrcpy(curDir, basePath, 512);

	DirList *const dList = (DirList*)malloc(sizeof(DirList));
	if(dList == NULL) return RES_OUT_OF_MEM;

	Result res;
	if((res = scanDir(curDir, dList)) != RES_OK) goto end;
	showDirList(dList, 0);

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
			if(hidGetExtraKeys(0) & KEY_POWER) goto end;
			kDown = hidKeysDown();
		} while(kDown == 0);

		oldCursorPos = cursorPos;
		if(kDown & KEY_DRIGHT) cursorPos += SCREEN_ROWS;
		if(kDown & KEY_DLEFT)  cursorPos -= SCREEN_ROWS;
		if(kDown & KEY_DUP)    cursorPos -= 1;
		if(kDown & KEY_DDOWN)  cursorPos += 1;

		if(cursorPos < 0)                     cursorPos = dList->num - 1; // Wrap to end of list.
		if((u32)cursorPos > (dList->num - 1)) cursorPos = 0;              // Wrap to start of list.

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
			if(kDown & KEY_A)
			{
				u32 pathLen = strlen(curDir);
				// TODO: !!! Insecure !!!
				if(curDir[pathLen - 1] != '/') curDir[pathLen++] = '/';
				safeStrcpy(curDir + pathLen, dList->strPtrs[cursorPos], 512);

				if(dList->entTypes[cursorPos] == 0)
				{
					safeStrcpy(selected, curDir, 512);
					break;
				}
			}
			if(kDown & KEY_B)
			{
				const u32 pathLen = strlen(curDir);
				if(curDir[pathLen - 2] != ':')
				{
					char *tmpPathPtr = curDir + pathLen;
					while(*--tmpPathPtr != '/');
					*tmpPathPtr = '\0';
				}
			}

			if((res = scanDir(curDir, dList)) != RES_OK) break;
			cursorPos = 0;
			windowPos = 0;
			showDirList(dList, 0);
		}
	}

end:
	free(dList);
	free(curDir);

	// Clear screen.
	ee_printf("\x1b[2J");

	return res;
}
