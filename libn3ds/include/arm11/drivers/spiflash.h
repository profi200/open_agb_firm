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


#define SPIFLASH_PP    (0x02u) // Page Program (0x100 bytes).
#define SPIFLASH_READ  (0x03u) // Read.
#define SPIFLASH_WRDI  (0x04u) // Write Disable.
#define SPIFLASH_RDSR  (0x05u) // Read Status Register.
#define SPIFLASH_WREN  (0x06u) // Write Enable.
#define SPIFLASH_PW    (0x0Au) // Page Write (0x100 bytes).
#define SPIFLASH_FAST  (0x0Bu) // Fast Read.
#define SPIFLASH_RDP   (0xABu) // Release from Deep Power-down.
#define SPIFLASH_DP    (0xB9u) // Deep Power-Down.
#define SPIFLASH_SE    (0xD8u) // Sector Erase (0x10000 bytes).
#define SPIFLASH_PE    (0xDBu) // Page Erase (0x100 bytes).
#define SPIFLASH_RDID  (0x9Fu) // Read JEDEC Identification.



// true if spiflash is installed, false otherwise
bool spiflash_get_status(void);
void spiflash_read(u32 offset, u32 size, u32 *buf);
