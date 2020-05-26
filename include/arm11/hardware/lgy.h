#pragma once

#include "types.h"


// All values in BCD.
typedef struct
{
	union
	{
		struct
		{
			u8 h;
			u8 min;
			u8 s;
			u8 unused;
		};
		u32 time;
	};
	union
	{
		struct
		{
			u8 y;
			u8 mon;
			u8 d;
			u8 dow; // Day of week.
		};
		u32 date;
	};
} GbaRtc;



void LGY_prepareLegacyMode(void);
bool LGY_setGbaRtc(GbaRtc rtc);
bool LGY_getGbaRtc(GbaRtc *out);
void LGY_switchMode(void);
void LGY_handleEvents(void);
void LGY_deinit(void);
