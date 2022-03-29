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
#include "arm11/drivers/spiflash.h"
#include "arm11/drivers/spi.h"



bool spiflash_get_status(void)
{
	alignas(4) u8 cmd[4];

	cmd[0] = SPIFLASH_RDSR;
	NSPI_writeRead(NSPI_DEV_CS_HIGH | NSPI_DEV_NVRAM, (u32*)cmd, (u32*)cmd, 1, 1);

	if(cmd[0] & 1) return false;
	return true;
}

void spiflash_read(u32 offset, u32 size, u32 *buf)
{
	offset = __builtin_bswap32(offset & 0x00FFFFFFu) | SPIFLASH_READ;

	NSPI_writeRead(NSPI_DEV_CS_HIGH | NSPI_DEV_NVRAM, &offset, buf, 4, size);
}
