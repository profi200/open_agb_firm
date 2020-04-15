#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2018 derrek, profi200
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



//////////////////////////////////
//             HASH             //
//////////////////////////////////

#define HASH_ENABLE         (1u) // Also used as busy flag
#define HASH_FINAL_ROUND    (1u<<1)
#define HASH_IN_DMA_ENABLE  (1u<<2) // Without this NDMA startup is never fires
#define HASH_INPUT_BIG      (1u<<3)
#define HASH_INPUT_LITTLE   (0u)
#define HASH_OUTPUT_BIG     (HASH_INPUT_BIG)
#define HASH_OUTPUT_LITTLE  (HASH_INPUT_LITTLE)
#define HASH_MODE_256       (0u)
#define HASH_MODE_224       (1u<<4)
#define HASH_MODE_1         (2u<<4)
#define HASH_MODE_MASK      (HASH_MODE_1 | HASH_MODE_224 | HASH_MODE_256)


/**
 * @brief      Sets input mode, endianess and starts the hash operation.
 *
 * @param[in]  params  Mode and input endianess bitmask.
 */
void HASH_start(u8 params);

/**
 * @brief      Hashes the data pointed to.
 *
 * @param[in]  data  Pointer to data to hash.
 * @param[in]  size  Size of the data to hash.
 */
void HASH_update(const u32 *data, u32 size);

/**
 * @brief      Generates the final hash.
 *
 * @param      hash       Pointer to memory to copy the hash to.
 * @param[in]  endianess  Endianess bitmask for the hash.
 */
void HASH_finish(u32 *const hash, u8 endianess);

/**
 * @brief      Returns the current HASH engine state.
 *
 * @param      out   Pointer to memory to copy the state to.
 */
void HASH_getState(u32 *const out);

/**
 * @brief      Hashes a single block of data and outputs the hash.
 *
 * @param[in]  data           Pointer to data to hash.
 * @param[in]  size           Size of the data to hash.
 * @param      hash           Pointer to memory to copy the hash to.
 * @param[in]  params         Mode and input endianess bitmask.
 * @param[in]  hashEndianess  Endianess bitmask for the hash.
 */
void hash(const u32 *data, u32 size, u32 *const hash, u8 params, u8 hashEndianess);
