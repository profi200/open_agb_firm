#pragma once

/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2024 profi200
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


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	u16 magic;       // "BM"
	u32 fileSize;
	u16 reserved;
	u16 reserved2;
	u32 pixelOffset; // From file start.
} PACKED BmpHeader;
static_assert(sizeof(BmpHeader) == 14);

typedef enum
{
	BI_RGB       =  0u,
	BI_RLE8      =  1u, // 8 bit per pixel only.
	BI_RLE4      =  2u, // 4 bit per pixel only.
	BI_BITFIELDS =  3u, // BitmapV2Infoheader RGB masks, BitmapV3Infoheader+ RGBA masks.
	BI_JPEG      =  4u, // BitmapV4Infoheader+.
	BI_PNG       =  5u, // BitmapV4Infoheader+.
	BI_CMYK      = 11u, // Only Windows Metafile (WMF) CMYK.
	BI_CMYKRLE8  = 12u, // Only Windows Metafile (WMF) CMYK.
	BI_CMYKRLE4  = 13u  // Only Windows Metafile (WMF) CMYK.
} BitmapCompr;

typedef struct
{
	u32 headerSize;      // Size of this header. 40 bytes.
	s32 width;
	s32 height;          // If >=0, pixel lines are in order bottom to top. Otherwise top to bottom.
	u16 colorPlanes;     // Must be 1.
	u16 bitsPerPixel;    // 1, 4, 8, 16, 24, 32.
	u32 compression;     // See BitmapCompr enum.
	u32 imageSize;       // Can be 0 if compression is BI_RGB.
	s32 xPixelsPerMeter;
	s32 yPixelsPerMeter;
	u32 colorsUsed;
	u32 colorsImportant;
} PACKED Bitmapinfoheader;
static_assert(sizeof(Bitmapinfoheader) == 0x28);

typedef struct
{
	BmpHeader header;
	Bitmapinfoheader dib;
} PACKED BmpV1;
static_assert(sizeof(BmpV1) == 0x36);

// Note: Technically this is BMP V2 but we use the shorter Bitmapinfoheader (V1).
//       The color masks are only needed for compression BI_BITFIELDS.
typedef struct
{
	BmpHeader header;
	Bitmapinfoheader dib;
	u32 rMask;
	u32 gMask;
	u32 bMask;
} PACKED BmpV1WithMasks;
static_assert(sizeof(BmpV1WithMasks) == 0x42);

#ifdef __cplusplus
} // extern "C"
#endif