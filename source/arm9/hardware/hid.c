/*
 * This code is part of libctru (https://github.com/devkitPro/libctru)
 */

#include "types.h"
#include "mem_map.h"


#define HID_REGS_BASE     (IO_MEM_ARM9_ARM11 + 0x46000)
#define REG_HID_PAD       (*((vu16*)(HID_REGS_BASE + 0x0)) ^ 0xFFFFu)
#define REG_HID_PADCNT     *((vu16*)(HID_REGS_BASE + 0x2))


static u32 kHeld, kDown, kUp;



void hidScanInput(void)
{
	u32 kOld = kHeld;
	kHeld = REG_HID_PAD;
	kDown = (~kOld) & kHeld;
	kUp = kOld & (~kHeld);
}

u32 hidKeysHeld(void)
{
	return kHeld;
}

u32 hidKeysDown(void)
{
	return kDown;
}

u32 hidKeysUp(void)
{
	return kUp;
}
