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


enum
{
	SDMMC_ERR_NONE           =  0, // No error.
	SDMMC_ERR_INVAL_PARAM    =  1, // Invalid parameter.
	SDMMC_ERR_INITIALIZED    =  2, // Card is already initialized.
	SDMMC_ERR_GO_IDLE_STATE  =  3, // GO_IDLE_STATE CMD error.
	SDMMC_ERR_SEND_IF_COND   =  4, // SEND_IF_COND CMD error.
	SDMMC_ERR_IF_COND_RESP   =  5, // IF_COND response pattern mismatch or unsupported voltage.
	SDMMC_ERR_SEND_OP_COND   =  6, // SEND_OP_COND CMD error.
	SDMMC_ERR_OP_COND_TMOUT  =  7, // Card initialization timeout.
	SDMMC_ERR_VOLT_SUPPORT   =  8, // Voltage not supported.
	SDMMC_ERR_ALL_SEND_CID   =  9, // ALL_SEND_CID CMD error.
	SDMMC_ERR_SET_SEND_RCA   = 10, // SEND/SET_RELATIVE_ADDR CMD error.
	SDMMC_ERR_SEND_CSD       = 11, // SEND_CSD CMD error.
	SDMMC_ERR_SELECT_CARD    = 12, // SELECT_CARD CMD error.
	SDMMC_ERR_LOCKED         = 13, // Card is locked with a password.
	SDMMC_ERR_SET_CLR_CD     = 14, // SET_CLR_CARD_DETECT CMD error.
	SDMMC_ERR_SET_BUS_WIDTH  = 15, // Error on switching to a different bus width.
	SDMMC_ERR_SWITCH_HS      = 16, // Error on switching to high speed mode.
	SDMMC_ERR_SET_BLOCKLEN   = 17, // SET_BLOCKLEN CMD error.
	SDMMC_ERR_SEND_STATUS    = 18, // SEND_STATUS CMD error.
	SDMMC_ERR_CARD_STATUS    = 19, // The card returned an error via its status.
	SDMMC_ERR_NO_CARD        = 20, // Card unitialized or not inserted.
	SDMMC_ERR_SECT_RW        = 21, // Sector read/write error.
	SDMMC_ERR_WRITE_PROT     = 22  // Can't write. The write protection slider is locked.
};

// SD/MMC device numbers.
enum
{
	SDMMC_DEV_SLOT = 0, // SD card/MMC slot.
	SDMMC_DEV_eMMC = 1  // Builtin eMMC.
};

typedef struct
{
	u8 type;      // 0 = none, 1 = SDSC, 2 = SDHC/SDXC, 3 = SDUC, 4 = (e)MMC, 5 = High capacity (e)MMC.
	u8 spec_vers; // (e)MMC only SPEC_VERS from CSD. 0 for SD.
	u16 rca;      // Relative Card Address (RCA).
	u32 sectors;  // Size in 512 byte units.
	u32 clock;    // The current clock frequency in Hz.
	u32 cid[4];   // Raw CID with the CRC zeroed out.
	u16 ccc;      // SD/(e)MMC command class support from CSD. One per bit starting at 0.
	u8 busWidth;  // The current bus width used to talk to the card.
} SdmmcInfo;



/**
 * @brief      { function_description }
 *
 * @param[in]  devNum  The dev number
 *
 * @return     { description_of_the_return_value }
 */
u32 SDMMC_init(u8 devNum);

/**
 * @brief      { function_description }
 *
 * @param[in]  devNum  The dev number
 *
 * @return     { description_of_the_return_value }
 */
u32 SDMMC_deinit(u8 devNum);

/**
 * @brief      { function_description }
 *
 * @param[in]  devNum   The dev number
 * @param      infoOut  The information out
 */
void SDMMC_getCardInfo(u8 devNum, SdmmcInfo *const infoOut);

/**
 * @brief      { function_description }
 *
 * @param[in]  devNum  The dev number
 * @param      cidOut  The cid out
 *
 * @return     { description_of_the_return_value }
 */
u32 SDMMC_getCid(u8 devNum, u32 *const cidOut);

/**
 * @brief      { function_description }
 *
 * @param[in]  devNum  The dev number
 *
 * @return     { description_of_the_return_value }
 */
u32 SDMMC_getSectors(u8 devNum);

/**
 * @brief      { function_description }
 *
 * @param[in]  devNum  The dev number
 * @param[in]  sect    The sect
 * @param      buf     The buffer
 * @param[in]  count   The count
 *
 * @return     { description_of_the_return_value }
 */
u32 SDMMC_readSectors(u8 devNum, u32 sect, u32 *const buf, u16 count);

/**
 * @brief      { function_description }
 *
 * @param[in]  devNum  The dev number
 * @param[in]  sect    The sect
 * @param[in]  buf     The buffer
 * @param[in]  count   The count
 *
 * @return     { description_of_the_return_value }
 */
u32 SDMMC_writeSectors(u8 devNum, u32 sect, const u32 *const buf, u16 count);
