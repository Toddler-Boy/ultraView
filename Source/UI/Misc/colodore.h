//
// 	All functions within this mini-library are derived from the Javascript
// 	code on https://www.colodore.com/ written by Philip "pepto" Timmermann
// 	with explicit written permission.
//
// 	You may use this code under the terms of the GPL v3 (see www.gnu.org/licenses).
//
// 	THIS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
// 	EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
// 	DISCLAIMED.
//

#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <algorithm>

//-----------------------------------------------------------------------------

class colodore final
{
public:
	colodore ();

	using yuvPalette = std::array<std::array<float, 3>, 16>;
	using rgbPalette = std::array<uint32_t, 16>;

	[[ nodiscard ]] yuvPalette generateYUV ( const int standard, float brightness = 50.0f, float contrast = 100.0f, float saturation = 50.0f, const bool earlyLuma = false ) const;
	[[ nodiscard ]] rgbPalette generateRGB ( const int standard, const yuvPalette& src ) const;

	inline uint32_t yuv2rgb ( const uint8_t y, const float u, const float v ) const
	{
		return	0xFF000000
				| gammaMap[ uint8_t ( std::clamp ( y + 1.140f * v, 0.0f, 255.0f ) ) ] << 16
				| gammaMap[ uint8_t ( std::clamp ( y - 0.396f * u - 0.581f * v, 0.0f, 255.0f ) ) ] << 8
				| gammaMap[ uint8_t ( std::clamp ( y + 2.029f * u, 0.0f, 255.0f ) ) ];
	}

	inline uint32_t yiq2rgb ( const uint8_t y, const float i, const float q ) const
	{
		return	0xFF000000
				| uint8_t ( std::clamp ( y + 0.956f * i + 0.619f * q, 0.0f, 255.0f ) ) << 16
				| uint8_t ( std::clamp ( y - 0.272f * i - 0.647f * q, 0.0f, 255.0f ) ) << 8
				| uint8_t ( std::clamp ( y - 1.106f * i + 1.703f * q, 0.0f, 255.0f ) );
	}

	inline uint32_t yiq2rgb_sony ( const uint8_t y, const float i, const float q ) const
	{
		return	0xFF000000
			| uint8_t ( std::clamp ( y + 1.630f * i + 0.317f * q, 0.0f, 255.0f ) ) << 16
			| uint8_t ( std::clamp ( y - 0.378f * i - 0.466f * q, 0.0f, 255.0f ) ) << 8
			| uint8_t ( std::clamp ( y - 1.089f * i + 1.677f * q, 0.0f, 255.0f ) );
	}
	//-----------------------------------------------------------------------------

	using shaderPalette = std::vector<float>;

	[[ nodiscard ]] shaderPalette generateYUV_YIQ ( const float phaseOffset, const bool earlyLuma = false ) const;

private:
	int8_t	firstL_revL_Angle[ 16 ][ 3 ] = {
		{	 0,		 0,		0,		},	// Black
		{	32,		32,		0,		},	// White
		{	 8,		10,		4,		},	// Red
		{	24,		20,		4 + 8,	},	// Cyan
		{	16,		12,		2,		},	// Purple
		{	16,		16,		2 + 8,	},	// Green
		{	 8,		 8,		7 + 8,	},	// Blue
		{	24,		24,		7,		},	// Yellow
		{	16,		12,		5,		},	// Orange
		{	 8,		 8,		6,		},	// Brown
		{	16,		16,		4,		},	// Light Red
		{	 8,		10,		0,		},	// Dark Grey
		{	16,		15,		0,		},	// Grey
		{	24,		24,		2 + 8,	},	// Light Green
		{	16,		15,		7 + 8,	},	// Light Blue
		{	24,		20,		0,		},	// Light Grey
	};

	std::array<uint8_t, 256>	gammaMap;
};
//-----------------------------------------------------------------------------
