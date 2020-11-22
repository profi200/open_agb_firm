#pragma once

#include "types.h"



noreturn void _a7_overlay_stub(void);
extern const u32 _a7_overlay_stub_size[];

noreturn void _a7_stub_start(void);
extern u16 _a7_stub9_swi[]; // Final ARM9 mem location.
extern const u32 _a7_stub_size[];
