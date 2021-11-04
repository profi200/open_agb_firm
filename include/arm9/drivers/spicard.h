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


#define SPICARD_REGS_BASE  (IO_MEM_ARM9_ONLY + 0xD800)

typedef struct
{
	vu32 cnt;       // 0x00
	vu8  cs;        // 0x04 32 bit but can be accessed as u8.
	u8 _0x5[3];
	vu32 blklen;    // 0x08
	vu32 fifo;      // 0x0C
	vu8  fifo_stat; // 0x10 32 bit but can be accessed as u8.
	u8 _0x11[3];
	vu32 autopoll;  // 0x14
	vu32 int_mask;  // 0x18
	vu32 int_stat;  // 0x1C
} Spic;
static_assert(offsetof(Spic, int_stat) == 0x1C, "Error: Member int_stat of Spic is not at offset 0x1C!");

ALWAYS_INLINE Spic* getSpicRegs(void)
{
	return (Spic*)SPICARD_REGS_BASE;
}


// REG_SPIC_CNT
typedef enum
{
	SPIC_CLK_512KHz = 0u,
	SPIC_CLK_1MHz   = 1u,
	SPIC_CLK_2MHz   = 2u,
	SPIC_CLK_4MHz   = 3u,
	SPIC_CLK_8MHz   = 4u,
	SPIC_CLK_16MHz  = 5u,

	// Not a real clock setting. Or with clock
	// to set chip select high after transfer.
	SPIC_CLK_CS_HIGH = 1u<<3
} SpicClk;

#define SPIC_BUS_1BIT        (0u)
#define SPIC_BUS_4BIT        (1u<<12)
#define SPIC_DIR_R           (0u)     // Direction read.
#define SPIC_DIR_W           (1u<<13) // Direction write.
#define SPIC_EN              (1u<<15) // Enable.

// REG_SPIC_CS
#define SPIC_CS_HIGH         (0u)

// SPIC_FIFO_STAT
#define SPIC_FIFO_BUSY       (1u)

// REG_SPIC_AUTOPOLL
// Shifts.
#define SPIC_AP_TMOUT_SHIFT  (16u) // Auto poll register timeout shift.
#define SPIC_AP_OFF_SHIFT    (24u) // Auto poll register bit offset shift.
#define SPIC_AP_BIT_SHIFT    (30u) // Auto poll register compare bit shift.

#define SPIC_AP_START        (1u<<31) // Auto poll start.

// REG_SPIC_INT_MASK Bit set = disabled.
// REG_SPIC_INT_STAT Status and acknowledge.
#define SPIC_INT_TRAN_END    (1u)    // Transfer end. Also fires on each auto poll try.
#define SPIC_INT_AP_MATCH    (1u<<1) // Auto poll bit match.
#define SPIC_INT_AP_TMOUT    (1u<<2) // Auto poll timeout.


// cmd     Is the command byte to send.
// tmout   Is the timeout. Must be 0-15. Tries = 31<<(SpicClk + timeout).
// off     Is the bit offset in the response byte. Must be 0-7.
// cmpBit  Is the bit to compare (0 or 1).
#define MAKE_AP_PARAMS(cmd, tmout, off, cmpBit) ((u32)(cmpBit)<<30 | (u32)(off)<<24 | (u32)(tmout)<<16 | (cmd))



/**
 * @brief      Initializes the SPI bus. Call this only once.
 */
void SPICARD_init(void);

/**
 * @brief      Automatically polls a bit of the command response.
 *
 * @param[in]  clk       The clock frequency to use.
 * @param[in]  apParams  The parameters. Use the MAKE_AP_PARAMS macro.
 *
 * @return     Returns false on timeout and true on bit match.
 */
bool SPICARD_autoPollBit(SpicClk clk, u32 apParams);

/**
 * @brief      Writes and/or reads data to/from a SPI device.
 *
 * @param[in]  clk      The clock frequency to use.
 * @param[in]  in       Input data pointer for write.
 * @param      out      Output data pointer for read.
 * @param[in]  inSize   Input size. Must be <= 0x1FFFFF.
 * @param[in]  outSize  Output size. Must be <= 0x1FFFFF.
 */
void SPICARD_writeRead(SpicClk clk, const u32 *in, u32 *out, u32 inSize, u32 outSize);
