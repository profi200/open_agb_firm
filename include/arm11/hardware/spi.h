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
#define NSPI_INT_AP_SUCCESS  (1u<<1) // Auto poll.
#define NSPI_INT_AP_TIMEOUT  (1u<<2) // Auto poll.


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
	NSPI_DEV_POWERMAN   = 0u, // Unused DS(i) mode power management.
	NSPI_DEV_NVRAM      = 1u, // WiFi SPI flash.
	NSPI_DEV_TWL_CODEC  = 2u,
	NSPI_DEV_CTR_CODEC  = 3u,
	NSPI_DEV_UNUSED5    = 4u, // Unused "CS2".
	NSPI_DEV_UNUSED6    = 5u, // Unused "CS3".
	NSPI_DEV_UNUSED7    = 6u  // Debugger?
} SpiDevice;


// cmd is the command byte to send.
// timeout is the timeout. Must be 0-15. Tries = 31<<(NspiClk + timeout).
// offset is the bit offset to poll for. Must be 0-7.
// bitSet is what to poll for (0 or 1).
#define MAKE_AP_PARAMS(cmd, timeout, offset, bitSet) ((u32)(bitSet)<<30 | (u32)(offset)<<24 | (u32)(timeout)<<16 | (cmd))



/**
 * @brief      Initializes the SPI buses. Call this only once.
 */
void NSPI_init(void);

/**
 * @brief      Automatically polls a bit of the command response.
 *
 * @param[in]  dev        The device ID. See table above.
 * @param[in]  ap_params  The parameters. Use the macro above.
 *
 * @return     Returns false on failure/timeout and true on success.
 */
bool NSPI_autoPollBit(SpiDevice dev, u32 ap_params);

/**
 * @brief      Writes and/or reads data to/from a SPI device.
 *
 * @param[in]  dev      The device ID. See table above.
 * @param[in]  in       Input data pointer for write.
 * @param      out      Output data pointer for read.
 * @param[in]  inSize   Input size. Must be <= 0x1FFFFF.
 * @param[in]  outSize  Output size. Must be <= 0x1FFFFF.
 */
void NSPI_writeRead(SpiDevice dev, const u32 *in, u32 *out, u32 inSize, u32 outSize);

/**
 * @brief      Deselect SPI device (chip select).
 *
 * @param[in]  dev   The device ID. See table above.
 */
void NSPI_deselect(SpiDevice dev);
