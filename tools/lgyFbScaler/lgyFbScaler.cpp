/*
 *   Copyright (C) 2022 profi200
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

#include <cstdio>
#include <cinttypes>
#include <exception>
#include <memory>
#include "lodepng.h"


#define NDEBUG  (1)


typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;


typedef struct
{
	u16 oWidth;
	u16 oHight;
	u16 width;
	u16 hight;
	s16 vMatrix[8 * 6];
	s16 hMatrix[8 * 6];
	u8  vPatt;
	u8  hPatt;
	u8  vLen;
	u8  hLen;
} ScalerParams;

struct Pixel final
{
	u8 m_r, m_g, m_b, m_a;


	// Constructors
	Pixel(void) noexcept {}
	Pixel(u8 r, u8 g, u8 b, u8 a = 0xFFu) noexcept : m_r(r), m_g(g), m_b(b), m_a(a) {}
	Pixel(const Pixel&) noexcept = delete; // Copy
	Pixel(Pixel&&) noexcept = delete;      // Move

	// Destructors
	//~Pixel(void) noexcept = default;

	// Operators
	Pixel& operator =(const Pixel&) noexcept = default; // Copy
	Pixel& operator =(Pixel&&) noexcept = default;      // Move

	// Functions
};

class Scaler final
{
	const Pixel *m_buf;
	const s16 *const m_matrix;
	s16 m_pos;
	const u16 m_lineLen;
	const u8 m_patt;
	const u8 m_pattLen;
	u8 m_pattPos;


	// Constructors
	Scaler(void) noexcept = delete;
	Scaler(const Scaler&) noexcept = delete; // Copy
	Scaler(Scaler&&) noexcept = delete;      // Move

	// Destructors
	//~Scaler(void) noexcept = default;

	// Operators
	Scaler& operator =(const Scaler&) noexcept = delete; // Copy
	Scaler& operator =(Scaler&&) noexcept = delete;      // Move

	const Pixel& operator [](u8 idx) const noexcept
	{
		//idx = (idx > 5 ? 5 : idx);

		idx = 5 - idx;
		u32 bufPos;
		if(m_pos - idx <= 0)               bufPos = 0;             // Output first pixel for lower out of bounds.
		else if(m_pos - idx >= m_lineLen)  bufPos = m_lineLen - 1; // Output last pixel for upper out of bounds.
		else                               bufPos = m_pos - idx;   // Normal pixel window offset.

		return m_buf[bufPos];
	}

	// Functions


public:
	// Constructors
	Scaler(const Pixel *buf, u16 lineLen, const s16 *matrix, u8 patt, u8 pattLen) noexcept
	: m_buf(buf), m_matrix(matrix), m_pos(0), m_lineLen(lineLen), m_patt(patt), m_pattLen(pattLen), m_pattPos(0)
	{
	}

	// Destructors

	// Operators

	// Functions
	Pixel calcPixel(void) const noexcept
	{
		s32 r = 0, g = 0, b = 0;
		const s16 *matrix = &m_matrix[m_pattPos];
		for(u8 i = 0; i < 6; i++)
		{
			const Pixel &pixel = (*this)[i];
			const s16 mult = matrix[i * 8];

			r += pixel.m_r * mult;
			g += pixel.m_g * mult;
			b += pixel.m_b * mult;
		}

		// Clamp the result.
		r = (r > 0x3FC000 ? 0x3FC000 : r);
		r = (r < 0 ? 0 : r);
		g = (g > 0x3FC000 ? 0x3FC000 : g);
		g = (g < 0 ? 0 : g);
		b = (b > 0x3FC000 ? 0x3FC000 : b);
		b = (b < 0 ? 0 : b);

		return Pixel(r / 0x4000, g / 0x4000, b / 0x4000);
	}

	void next(void) noexcept
	{
		if(m_patt & 1u<<m_pattPos++)
		{
			if(m_pos == 0) m_pos = 3; // Hardware loads 3 more pixels after the first.
			else           m_pos++;
		}

		if(m_pattPos == m_pattLen) m_pattPos = 0;
	}

	void nextLine(void) noexcept
	{
		m_buf += m_lineLen;
		m_pos = 0;
	}

	void resetLine(void) noexcept
	{
		m_pos = 0;
		m_pattPos = 0;
	}
};


static void scaleFrame(Pixel *buf, const ScalerParams &params)
{
	std::unique_ptr<Pixel[]> tmpBuf(new(std::nothrow) Pixel[512 * 512]);
	const u16 oHight = params.oHight;
	const u16 width = params.width;

	{
		Scaler scaler(buf, params.oWidth, params.hMatrix, params.hPatt, params.hLen);
		for(u16 h = 0; h < oHight; h++)
		{
			for(u16 w = 0; w < width; w++)
			{
				tmpBuf[width * h + w] = scaler.calcPixel();
				scaler.next();
			}

			scaler.nextLine();
		}
	}

	{
		std::unique_ptr<Pixel[]> tmpLine(new(std::nothrow) Pixel[512]);
		const u16 hight = params.hight;
		Scaler scaler(tmpLine.get(), oHight, params.vMatrix, params.vPatt, params.vLen);
		for(u16 w = 0; w < width; w++)
		{
			for(u16 h = 0; h < oHight; h++)
				tmpLine[h] = tmpBuf[width * h + w];

			for(u16 h = 0; h < hight; h++)
			{
				buf[width * h + w] = scaler.calcPixel();
				scaler.next();
			}
			scaler.resetLine();
		}
	}
}

// TODO: More validation.
static bool parseMatrix(const char *const file, ScalerParams &params)
{
	char buf[1024] = {};
	FILE *f = fopen(file, "r");
	fread(buf, 1023, 1, f);
	fclose(f);

	// Parse horizontal params.
	const char *token;
	if((token = strtok(buf, "\t ,\n\r")) == nullptr) return false;
	params.hPatt = strtoul(token, nullptr, 2) & 0xFFu;

	if((token = strtok(nullptr, "\t ,\n\r")) == nullptr) return false;
	params.hLen = strtoul(token, nullptr, 10) & 0xFFu;
	if(params.hLen > 8 || params.hLen < 1) return false;

	for(u8 i = 0; i < 48; i++)
	{
		if((token = strtok(nullptr, "\t ,\n\r")) == nullptr) break;
		params.hMatrix[i] = (s16)strtol(token, nullptr, 0) & ~15u; // Bits 0-3 are not used.
	}

	// Parse vertical params.
	if((token = strtok(nullptr, "\t ,\n\r")) == nullptr) return false;
	params.vPatt = strtoul(token, nullptr, 2) & 0xFFu;

	if((token = strtok(nullptr, "\t ,\n\r")) == nullptr) return false;
	params.vLen = strtoul(token, nullptr, 10) & 0xFFu;
	if(params.vLen > 8 || params.vLen < 1) return false;

	for(u8 i = 0; i < 48; i++)
	{
		if((token = strtok(nullptr, "\t ,\n\r")) == nullptr) break;
		params.vMatrix[i] = (s16)strtol(token, nullptr, 0) & ~15u; // Bits 0-3 are not used.
	}

	return true;
}

// Compile with "g++ -std=c++17 -s -flto -O2 -fstrict-aliasing -ffunction-sections -Wall -Wextra -I./lodepng -Wl,--gc-sections ./lodepng/lodepng.cpp ./lgyFbScaler.cpp -o ./lgyFbScaler"
int main(int argc, char const *argv[])
{
	unsigned char *inBuf;
	u32 oWidth, oHight;
	u32 lpngErr;
	if((lpngErr = lodepng_decode32_file(&inBuf, &oWidth, &oHight, argv[1])))
	{
		fprintf(stderr, "lodepng error: %s", lodepng_error_text(lpngErr));
		return 1;
	}

	if(oWidth > 512 || oHight > 512)
	{
		fputs("Error: Input image too big.", stderr);
		free(inBuf);
		return 2;
	}

	std::unique_ptr<Pixel[]> outBuf(new(std::nothrow) Pixel[512 * 512]);
	memcpy(outBuf.get(), inBuf, sizeof(Pixel) * oWidth * oHight);
	free(inBuf);

	static ScalerParams params = {(u16)oWidth, (u16)oHight};
	if(!parseMatrix(argv[2], params))
	{
		fputs("Failed to parse matrix file.", stderr);
		return 3;
	}

	const float scaleX = (float)params.hLen / __builtin_popcount(params.hPatt);
	const float scaleY = (float)params.vLen / __builtin_popcount(params.vPatt);
	params.width = oWidth * scaleX;
	params.hight = oHight * scaleY;
	printf("Output width: %" PRIu16 " (x%f)\nOutput hight: %" PRIu16 " (x%f)\n",
	       params.width, scaleX, params.hight, scaleY);

	scaleFrame(outBuf.get(), params);

	if((lpngErr = lodepng_encode32_file(argv[3], reinterpret_cast<const unsigned char*>(outBuf.get()), params.width, params.hight)))
	{
		fprintf(stderr, "lodepng error: %s", lodepng_error_text(lpngErr));
		return 4;
	}

	return 0;
}
