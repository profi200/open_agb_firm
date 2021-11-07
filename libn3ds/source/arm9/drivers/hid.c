/*
 * This code is part of libctru (https://github.com/devkitPro/libctru)
 */

#include "types.h"
#include "mem_map.h"


#define HID_REGS_BASE     (IO_MEM_ARM9_ARM11 + 0x46000)
#define REG_HID_PAD       (*((vu16*)(HID_REGS_BASE + 0x0)) ^ 0xFFFFu)
#define REG_HID_PADCNT     *((vu16*)(HID_REGS_BASE + 0x2))


static u32 g_kHeld, g_kDown, g_kUp;



void hidScanInput(void)
{
	u32 kOld = g_kHeld;
	g_kHeld = REG_HID_PAD;
	g_kDown = (~kOld) & g_kHeld;
	g_kUp = kOld & (~g_kHeld);
}

u32 hidKeysHeld(void)
{
	return g_kHeld;
}

u32 hidKeysDown(void)
{
	return g_kDown;
}

u32 hidKeysUp(void)
{
	return g_kUp;
}
