#pragma once

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

#include <assert.h>
#include "types.h"
#include "mem_map.h"


#define CSND_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x3000)

typedef struct
{
	vu16 cnt;
	vs16 sr;               // Samplerate.
	union
	{
		struct
		{
			vu16 vol_r;    // 0-0x8000.
			vu16 vol_l;    // 0-0x8000.
		};
		vu32 vol;          // R and L combined.
	};
	union
	{
		struct
		{
			vu16 capvol_r; // Unconfirmed. 0-0x8000.
			vu16 capvol_l; // Unconfirmed. 0-0x8000.
		};
		vu32 capvol;       // R and L combined.
	};
	vu32 st_addr;          // Start address and playback position.
	vu32 size;             // Size in bytes.
	vu32 lp_addr;          // Loop restart address.
	vu32 st_adpcm;         // Start IMA-ADPCM state.
	vu32 lp_adpcm;         // Loop Restart IMA-ADPCM state.
} CsndCh;
static_assert(offsetof(CsndCh, lp_adpcm) == 0x1C, "Error: Member lp_adpcm of CsndCh is not at offset 0x1C!");

typedef struct
{
	vu16 cnt;
	u8 _0x2[2];
	vs16 sr;   // Samplerate.
	u8 _0x6[2];
	vu32 size; // Capture length in bytes.
	vu32 addr; // Address.
} CsndCap;
static_assert(offsetof(CsndCap, addr) == 0xC, "Error: Member addr of CsndCap is not at offset 0xC!");

typedef struct
{
	vu16 master_vol; // CSND master volume 0-0x8000.
	vu16 unk_cnt;
	u8 _0x4[0xc];
	vu32 unk010;     // FIFO related?
	vu8  unk014;     // FIFO related?
	u8 _0x15[0x3eb];
	CsndCh ch[32];   // 32 sound channels. PSG on channel 8-13 and noise 14-15.
	CsndCap cap[2];  // 2 capture units for right and left side.
} Csnd;
static_assert(offsetof(Csnd, cap[1].addr) == 0x81C, "Error: Member cap[1].addr of Csnd is not at offset 0x81C!");

ALWAYS_INLINE Csnd* getCsndRegs(void)
{
	return (Csnd*)CSND_REGS_BASE;
}

ALWAYS_INLINE CsndCh* getCsndChRegs(u8 ch)
{
	return &getCsndRegs()->ch[ch];
}

ALWAYS_INLINE CsndCap* getCsndCapRegs(u8 ch)
{
	return &getCsndRegs()->cap[ch];
}


// REG_CSND_CH_CNT
#define CSND_CH_DUTY(d)        (d)      // For PSG (channel 8-13) only. In 12.5% units. 0 = high/12.5%.
#define CSND_CH_LINEAR_INTERP  (1u<<6)  // Linear interpolation
#define CSND_CH_HOLD           (1u<<7)  // Hold last sample after one shot.
#define CSND_CH_PLAYING        (1u<<14)
#define CSND_CH_START          (1u<<15)

enum
{
	CSND_CH_RPT_MANUAL    = 0u<<10,
	CSND_CH_RPT_LOOP      = 1u<<10,
	CSND_CH_RPT_ONE_SHOT  = 2u<<10
};

enum
{
	CSND_CH_FMT_PCM8      = 0u<<12, // Signed PCM8
	CSND_CH_FMT_PCM16     = 1u<<12, // Signed PCM16 little endian
	CSND_CH_FMT_IMA_ADPCM = 2u<<12,
	CSND_CH_FMT_PSG_NOISE = 3u<<12
};


// REG_CSND_CAP_CNT
#define CSND_CAP_RPT_LOOP      (0u)
#define CSND_CAP_RPT_ONE_SHOT  (1u)
#define CSND_CAP_FMT_PCM16     (0u)     // Signed PCM16 little endian
#define CSND_CAP_FMT_PCM8      (1u<<1)  // Signed PCM8
#define CSND_CAP_UNK2          (1u<<2)
#define CSND_CAP_START         (1u<<15)


// Samplerate helpers
#define CSND_SAMPLERATE(s)  (-(s16)(67027964u / (s)))
#define CSND_PSG_FREQ(f)    (CSND_SAMPLERATE(32u * (f)))



/**
 * @brief      Initializes the CSND hardware.
 */
void CSND_init(void);

/**
 * @brief      Calculates the left and right volumes.
 *
 * @param[in]  lvol  The left volume.
 * @param[in]  rvol  The right volume.
 *
 * @return     The volume pair needed for CSND_setupCh().
 */
static inline u32 CSND_calcVol(float lvol, float rvol)
{
	return (u32)(lvol * 32768.0f)<<16 | (u16)(rvol * 32768.0f);
}

/**
 * @brief      Sets up a channel for sound playback (in paused state).
 *
 * @param[in]  ch          The sound channel. 0-31.
 * @param[in]  sampleRate  The sample rate.
 * @param[in]  vol         The L/R volume pair.
 * @param[in]  data        The start address.
 * @param[in]  data2       The loop restart address.
 * @param[in]  size        The size.
 * @param[in]  flags       The flags.
 */
void CSND_setupCh(u8 ch, s16 sampleRate, u32 vol, const u32 *const data, const u32 *const data2, u32 size, u16 flags);

/**
 * @brief      Pauses or unpauses a sound channel.
 *
 * @param[in]  ch       The sound channel. 0-31.
 * @param[in]  playing  The play state.
 */
static inline void CSND_setChState(u8 ch, bool playing)
{
	CsndCh *const csndCh = getCsndChRegs(ch);
	csndCh->cnt = (csndCh->cnt & ~CSND_CH_PLAYING) | ((u16)playing<<14);
}

/**
 * @brief      Returns the current audio buffer position (address).
 *
 * @param[in]  ch    The sound channel. 0-31.
 *
 * @return     The playback position (address).
 */
static inline u32 CSND_getChPos(u8 ch)
{
	return getCsndChRegs(ch)->st_addr;
}

/**
 * @brief      Stops a sound channel.
 *
 * @param[in]  ch    The sound channel. 0-31.
 */
static inline void CSND_stopCh(u8 ch)
{
	getCsndChRegs(ch)->cnt = 0; // Stop.
}


/**
 * @brief      Captures the output of all left/right sound channels combined.
 *
 * @param[in]  ch          The capture side. 0 = right, 1 = left.
 * @param[in]  sampleRate  The sample rate.
 * @param      data        The output address.
 * @param[in]  size        The size.
 * @param[in]  flags       The flags.
 */
void CSND_startCap(u8 ch, s16 sampleRate, u32 *const data, u32 size, u16 flags);

/**
 * @brief      Returns the current capture buffer position (address).
 *
 * @param[in]  ch    The capture side. 0 = right, 1 = left.
 *
 * @return     The capture position (address).
 */
static inline u32 CSND_getCapPos(u8 ch)
{
	return getCsndCapRegs(ch)->addr;
}

/**
 * @brief      Stops a capture channel.
 *
 * @param[in]  ch    The capture side. 0 = right, 1 = left.
 */
static inline void CSND_stopCap(u8 ch)
{
	getCsndCapRegs(ch)->cnt = 0;
}
