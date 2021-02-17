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
#include "util.h"


#define NDMA_REGS_BASE       (IO_MEM_ARM9_ONLY + 0x2000)
#define REG_NDMA_GLOBAL_CNT  *((vu32*)(NDMA_REGS_BASE + 0x00))

typedef struct
{
	vu32 src_addr;
	vu32 dst_addr;
	vu32 total_cnt;   // Total repeat length in words.
	vu32 log_blk_cnt; // Logical block size in words.
	vu32 int_cnt;     // Timing/interval settings.
	vu32 fill_data;
	vu32 cnt;
} NdmaCh;
static_assert(offsetof(NdmaCh, cnt) == 0x18, "Error: Member cnt of NdmaCh is not at offset 0x18!");

ALWAYS_INLINE NdmaCh* getNdmaChRegs(u8 ch)
{
	return &((NdmaCh*)(NDMA_REGS_BASE + 4))[ch];
}


// REG_NDMA_GLOBAL_CNT
#define NDMA_ROUND_ROBIN(n)    (intLog2(n)<<16 | 1u<<31 | 1u) // n = number of CPU cycles.
#define NDMA_HIGHEST_PRIO      (1u)

// REG_NDMA_INT_CNT
#define NDMA_INT_SYS_FREQ      (0u)

// REG_NDMA_CNT
#define NDMA_DST_UPDATE_INC    (0u)
#define NDMA_DST_UPDATE_DEC    (1u<<10)
#define NDMA_DST_UPDATE_FIXED  (2u<<10)
#define NDMA_DST_ADDR_RELOAD   (1u<<12) // Reloads on logical block end.
#define NDMA_SRC_UPDATE_INC    (0u)
#define NDMA_SRC_UPDATE_DEC    (1u<<13)
#define NDMA_SRC_UPDATE_FIXED  (2u<<13)
#define NDMA_SRC_UPDATE_FILL   (3u<<13)
#define NDMA_SRC_ADDR_RELOAD   (1u<<15)
// The block length is 2^n words (Example: 2^15 = 32768 words = 0x20000 bytes).
#define NDMA_BURST_WORDS(n)    (intLog2(n)<<16)
#define NDMA_IMMEDIATE_MODE    (16u<<24)
#define NDMA_TOTAL_CNT_MODE    (0u)
#define NDMA_REPEATING_MODE    (1u<<29)
#define NDMA_IRQ_ENABLE        (1u<<30)
#define NDMA_ENABLE            (1u<<31)


enum
{
	NDMA_STARTUP_TIMER0       =  0u<<24,
	NDMA_STARTUP_TIMER1       =  1u<<24,
	NDMA_STARTUP_TIMER2       =  2u<<24,
	NDMA_STARTUP_TIMER3       =  3u<<24,
	NDMA_STARTUP_CTRCARD0     =  4u<<24, // Fires with SPICARD aswell but seems to be broken.
	NDMA_STARTUP_CTRCARD1     =  5u<<24, // Fires with SPICARD aswell but seems to be broken.
	NDMA_STARTUP_TMIO1        =  6u<<24,
	NDMA_STARTUP_TMIO3        =  7u<<24, // Unconfirmed.
	NDMA_STARTUP_AES_IN       =  8u<<24, // AES write fifo.
	NDMA_STARTUP_AES_OUT      =  9u<<24, // AES read fifo.
	NDMA_STARTUP_SHA_IN       = 10u<<24,
	NDMA_STARTUP_SHA_OUT      = 11u<<24, // For chaining.
	NDMA_STARTUP_UNK_12       = 12u<<24,
	NDMA_STARTUP_UNK_13       = 13u<<24,
	NDMA_STARTUP_UNK_14       = 14u<<24,
	NDMA_STARTUP_MMC_AES_SHA  = 15u<<24  // Unconfirmed.
};



/**
 * @brief      Initializes all NDMA channels.
 */
void NDMA_init(void);

/**
 * @brief      Copies data using the NDMA engine.
 *
 * @param      dest    Pointer to destination memory. Must be 4 bytes aligned.
 * @param      source  Pointer to source data. Must be 4 bytes aligned.
 * @param[in]  size    The size of the data. Must be multiple of 4.
 */
void NDMA_copy(u32 *dest, const u32 *source, u32 size);

/**
 * @brief      Fills memory with the given value using the NDMA engine.
 *
 * @param      dest   Pointer to destination memory. Must be 4 bytes aligned.
 * @param[in]  value  The value each 32-bit word will be set to.
 * @param[in]  size   The size of the memory to fill. Must be multiple of 4.
 */
void NDMA_fill(u32 *dest, u32 value, u32 size);
