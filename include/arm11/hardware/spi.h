#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2019 derrek, profi200
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

#include "types.h"


// REG_NSPI_CNT
#define NSPI_BUS_1BIT        (0u)
#define NSPI_BUS_4BIT        (1u<<12)
#define NSPI_DIR_READ        (0u)
#define NSPI_DIR_WRITE       (1u<<13)
#define NSPI_ENABLE          (1u<<15)

// REG_NSPI_CS
#define NSPI_DESELECT        (0u)

// NSPI_FIFO_STAT
#define NSPI_FIFO_BUSY       (1u)

// REG_NSPI_AUTOPOLL
#define NSPI_AUTOPOLL_START  (1u<<31)

// REG_NSPI_INT_MASK Bit set = disabled.
// REG_NSPI_INT_STAT Status and aknowledge.
#define NSPI_INT_TRANSF_END  (1u)    // Also fires on each auto poll try.
#define NSPI_INT_AP_SUCCESS  (1u<<1) // Auto poll
#define NSPI_INT_AP_TIMEOUT  (1u<<2) // Auto poll


// Old interface clocks.
enum
{
	SPI_CLK_4MHz   = 0u,
	SPI_CLK_2MHz   = 1u,
	SPI_CLK_1MHz   = 2u,
	SPI_CLK_512KHz = 3u,
	SPI_CLK_8MHz   = 4u  // Only in DSi/3DS mode.
};

// New interface clocks.
enum
{
	NSPI_CLK_512KHz = 0u,
	NSPI_CLK_1MHz   = 1u,
	NSPI_CLK_2MHz   = 2u,
	NSPI_CLK_4MHz   = 3u,
	NSPI_CLK_8MHz   = 4u,
	NSPI_CLK_16MHz  = 5u
};

typedef enum
{
	NSPI_DEV_POWERMAN   = 0u, // Unused DS(i) mode power management
	NSPI_DEV_NVRAM      = 1u, // WiFi SPI flash
	NSPI_DEV_TWL_CODEC  = 2u,
	NSPI_DEV_CTR_CODEC  = 3u,
	NSPI_DEV_UNUSED5    = 4u, // Unused "CS2".
	NSPI_DEV_UNUSED6    = 5u, // Unused "CS3".
	NSPI_DEV_UNUSED7    = 6u  // Debugger?
} SpiDevice;



/**
 * @brief      Initializes the SPI buses. Call this only once.
 */
void NSPI_init(void);

/**
 * @brief      Automatically polls a bit of the command response. Use with the macro below.
 *
 * @param[in]  dev     The device ID. See table above.
 * @param[in]  params  The parameters. Use the macro below.
 *
 * @return     Returns false on failure/timeout and true on success.
 */
bool _NSPI_autoPollBit(SpiDevice dev, u32 params);

/**
 * @brief      Writes and/or reads data to/from a SPI device.
 *
 * @param[in]  dev      The device ID. See table above.
 * @param[in]  in       Input data pointer for write.
 * @param      out      Output data pointer for read.
 * @param[in]  inSize   Input size. Must be <= 0x1FFFFF.
 * @param[in]  outSize  Output size. Must be <= 0x1FFFFF.
 * @param[in]  done     Set to true if this is the last transfer (chip select).
 */
void NSPI_writeRead(SpiDevice dev, const u32 *in, u32 *out, u32 inSize, u32 outSize, bool done);


/**
 * @brief      Automatically polls a bit of the command response.
 *
 * @param[in]  dev      The device ID. See table above.
 * @param[in]  cmd      The command.
 * @param[in]  timeout  The timeout. Must be 0-15. Tries = 31<<NspiClk + timeout.
 * @param[in]  off      The bit offset. Must be 0-7.
 * @param[in]  bitSet   Poll for a set ur unset bit.
 *
 * @return     Returns false on failure/timeout and true on success.
 */
#define NSPI_autoPollBit(dev, cmd, timeout, off, bitSet) _NSPI_autoPollBit(dev, (bitSet)<<30 | (off)<<24 | (timeout)<<16 | (cmd))
