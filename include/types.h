#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
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

#include <inttypes.h>    // printf()/scanf() macros
#include <stdalign.h>    // alignas(alignment_in_bytes)
#include <stdbool.h>     // bool, true/false
#include <stddef.h>      // size_t, NULL...
#include <stdint.h>      // uint8_t, uint16_t...
#include <stdnoreturn.h> // noreturn keyword
#include <unistd.h>      // ssize_t



#define PACKED   __attribute__((packed))
#define USED     __attribute__((used))
#define UNUSED   __attribute__((unused))
#define NAKED    __attribute__((naked))
#define WEAK     __attribute__((weak))


typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef volatile uint8_t  vu8;
typedef volatile uint16_t vu16;
typedef volatile uint32_t vu32;
typedef volatile uint64_t vu64;

typedef volatile int8_t  vs8;
typedef volatile int16_t vs16;
typedef volatile int32_t vs32;
typedef volatile int64_t vs64;


typedef struct
{
	u32 data[3];
} _u96;

typedef struct
{
	u32 data[4];
} _u128;

typedef struct
{
	u32 data[16];
} _u512;
