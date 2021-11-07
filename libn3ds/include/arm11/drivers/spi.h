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


#define NSPI1_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x60800)
#define NSPI2_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x42800)
#define NSPI3_REGS_BASE  (IO_MEM_ARM9_ARM11 + 0x43800)

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
} NspiBus;
static_assert(offsetof(NspiBus, int_stat) == 0x1C, "Error: Member int_stat of NspiBus is not at offset 0x1C!");

enum
{
	SPI_BUS1 = 0u,
	SPI_BUS2 = 1u,
	SPI_BUS3 = 2u
};

ALWAYS_INLINE NspiBus* getNspiBusRegs(u8 busId)
{
	NspiBus *nspiBus;
	switch(busId)
	{
		case SPI_BUS1:
			nspiBus = (NspiBus*)NSPI1_REGS_BASE;
			break;
		case SPI_BUS2:
			nspiBus = (NspiBus*)NSPI2_REGS_BASE;
			break;
		case SPI_BUS3:
			nspiBus = (NspiBus*)NSPI3_REGS_BASE;
			break;
		default:
			nspiBus = NULL;
	}

	return nspiBus;
}


// REG_NSPI_CNT
enum
{
	NSPI_CLK_512KHZ = 0u,
	NSPI_CLK_1MHZ   = 1u,
	NSPI_CLK_2MHZ   = 2u,
	NSPI_CLK_4MHZ   = 3u,
	NSPI_CLK_8MHZ   = 4u,
	NSPI_CLK_16MHZ  = 5u
};

enum
{
	NSPI_CS_0 = 0u<<6,
	NSPI_CS_1 = 1u<<6,
	NSPI_CS_2 = 2u<<6
};

#define NSPI_BUS_1BIT        (0u)
#define NSPI_BUS_4BIT        (1u<<12)
#define NSPI_DIR_R           (0u)     // Direction read.
#define NSPI_DIR_W           (1u<<13) // Direction write.
#define NSPI_EN              (1u<<15) // Enable.

// REG_NSPI_CS
#define NSPI_CS_HIGH         (0u)

// NSPI_FIFO_STAT
#define NSPI_FIFO_BUSY       (1u)

// REG_NSPI_AUTOPOLL
// Shifts.
#define NSPI_AP_TMOUT_SHIFT  (16u) // Auto poll register timeout shift.
#define NSPI_AP_OFF_SHIFT    (24u) // Auto poll register bit offset shift.
#define NSPI_AP_BIT_SHIFT    (30u) // Auto poll register compare bit shift.

#define NSPI_AP_START        (1u<<31) // Auto poll start.

// REG_NSPI_INT_MASK Bit set = disabled.
// REG_NSPI_INT_STAT Status and acknowledge.
#define NSPI_INT_TRAN_END    (1u)    // Transfer end. Also fires on each auto poll try.
#define NSPI_INT_AP_MATCH    (1u<<1) // Auto poll bit match.
#define NSPI_INT_AP_TMOUT    (1u<<2) // Auto poll timeout.

// TODO: Implement old SPI interfaces.
// Old interface clocks.
/*enum
{
	SPI_CLK_4MHZ   = 0u,
	SPI_CLK_2MHZ   = 1u,
	SPI_CLK_1MHZ   = 2u,
	SPI_CLK_512KHZ = 3u,
	SPI_CLK_8MHZ   = 4u  // Only in DSi/3DS mode.
};*/


typedef enum
{
	NSPI_DEV_POWERMAN   = 0u, // DS(i) mode power management.
	NSPI_DEV_NVRAM      = 1u, // WiFi SPI flash.
	NSPI_DEV_TWL_CODEC  = 2u, // DSi mode SPI interface.
	NSPI_DEV_CTR_CODEC  = 3u, // 3DS mode SPI interface.
	//NSPI_DEV_UNUSED5    = 4u, // Unused.
	//NSPI_DEV_UNUSED6    = 5u, // Unused.
	//NSPI_DEV_UNUSED7    = 6u, // Debugger?

	// Not a real device. Or with device number
	// to set chip select high after transfer.
	NSPI_DEV_CS_HIGH    = 1u<<7
} SpiDevice;


// cmd     Is the command byte to send.
// tmout   Is the timeout. Must be 0-15. Tries = 31<<(NspiClk + timeout).
// off     Is the bit offset in the response byte. Must be 0-7.
// cmpBit  Is the bit to compare (0 or 1).
#define MAKE_AP_PARAMS(cmd, tmout, off, cmpBit) ((u32)(cmpBit)<<30 | (u32)(off)<<24 | (u32)(tmout)<<16 | (cmd))



/**
 * @brief      Initializes the SPI buses. Call this only once.
 */
void NSPI_init(void);

/**
 * @brief      Automatically polls a bit of the command response.
 *
 * @param[in]  dev       The device ID. See SpiDevice table.
 * @param[in]  apParams  The parameters. Use the MAKE_AP_PARAMS macro.
 *
 * @return     Returns false on timeout and true on bit match.
 */
bool NSPI_autoPollBit(SpiDevice dev, u32 apParams);

/**
 * @brief      Writes and/or reads data to/from a SPI device.
 *
 * @param[in]  dev      The device ID. See SpiDevice table.
 * @param[in]  in       Input data pointer for write.
 * @param      out      Output data pointer for read.
 * @param[in]  inSize   Input size. Must be <= 0x1FFFFF.
 * @param[in]  outSize  Output size. Must be <= 0x1FFFFF.
 */
void NSPI_writeRead(SpiDevice dev, const u32 *in, u32 *out, u32 inSize, u32 outSize);
