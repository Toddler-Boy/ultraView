#include "colodore.h"
#include <cmath>
#include <numbers>

//-----------------------------------------------------------------------------

colodore::colodore ()
{
	// Pre-calculate gamma-map
	static_assert ( sizeof ( gammaMap ) == 256, "gamma map has to be 256 entries" );

	constexpr auto gamma_pepto = [] ( float value ) -> uint8_t
	{
		constexpr auto	gammasrc = 2.8f;		// PAL
		constexpr auto	gammatgt = 2.2f;		// sRGB

		// reverse gamma correction of source
		static auto	srcFactor = std::pow ( 255.0f, 1.0f - gammasrc );
		value = std::clamp ( srcFactor * std::pow ( value, gammasrc ), 0.0f, 255.0f );

		// apply gamma correction for target
		static auto	dstFactor = std::pow ( 255.0f, 1.0f - 1.0f / gammatgt );
		value = std::clamp ( dstFactor * std::pow ( value, 1.0f / gammatgt ), 0.0f, 255.0f );

		return uint8_t ( value + 0.5f );
	};

	for ( auto input = 0; input < 256; ++input )
		gammaMap[ input ] = gamma_pepto ( float ( input ) );
}
//-----------------------------------------------------------------------------

colodore::yuvPalette colodore::generateYUV ( const int standard, float brightness /*= 50.0f*/, float contrast /*= 100.0f*/, float saturation /*= 50.0f */, const bool earlyLuma /*= false */) const
{
	brightness -= standard ? 70.0f : 50.0f;															// NTSC has a stronger brightness bias
	contrast = std::lerp ( 0.4f, 1.2f, contrast * 0.01f );											// Contrast is boosted 20%
	saturation = std::lerp ( 0.0f, 80.0f, ( saturation + ( standard ? 0.0f : 5.0f ) ) * 0.01f );	// PAL needs more saturation

	yuvPalette	dst;

	constexpr auto	lumaFactor = 256.0f / 32.0f;

	constexpr auto	chromaSector = 360.0f / 16.0f;
	constexpr auto	chromaRadian = std::numbers::pi_v<float> / 180.0f;

	const auto	chromaOrigin = chromaSector / 2.0f - ( standard ? 33.0f : 0.0f );

	for ( auto i = 0; auto eLcLaS : firstL_revL_Angle )
	{
		const auto	lumaSrc = eLcLaS[ earlyLuma ? 0 : 1 ];
		const auto	angleSrc = eLcLaS[ 2 ];

		auto	y = lumaSrc * lumaFactor;
		auto	u = 0.0f;
		auto	v = 0.0f;

		if ( angleSrc )
		{
			const auto	newAng = ( chromaOrigin + angleSrc * chromaSector ) * chromaRadian;

			u = std::cos ( newAng );
			v = std::sin ( newAng );
		}

		u *= saturation;	// apply saturation
		v *= saturation;

		y *= contrast;		// apply contrast
		u *= contrast;
		v *= contrast;

		y += brightness;	// apply brightness

		dst[ i++ ] = { y, u, v };
	}

	return dst;
}
//-----------------------------------------------------------------------------

colodore::shaderPalette colodore::generateYUV_YIQ ( const float phaseOffset, const bool earlyLuma /*= false */ ) const
{
	shaderPalette	dst;

	dst.resize ( 3 * 16 * 3 );

	constexpr auto	lumaFactor = 1.0f / 32.0f;

	constexpr auto	chromaSector = 360.0f / 16.0f;
	constexpr auto	chromaRadian = std::numbers::pi_v<float> / 180.0f;

	constexpr auto	chromaOriginPAL = chromaSector / 2.0f;
	constexpr auto	chromaOriginNTSC = chromaOriginPAL - 33.0f;

	constexpr auto	chromaScale = 100.0f / 256.0f;

	const auto	oddCos = std::cos ( phaseOffset * chromaRadian );
	const auto	oddSin = std::sin ( phaseOffset * chromaRadian );

	for ( auto j = 0; auto eLcLaS : firstL_revL_Angle )
	{
		const auto	lumaSrc = eLcLaS[ earlyLuma ? 0 : 1 ];
		const auto	angleSrc = eLcLaS[ 2 ];

		auto	y = lumaSrc * lumaFactor;

		auto	u = 0.0f;
		auto	v = 0.0f;

		auto	i = 0.0f;
		auto	q = 0.0f;

		if ( angleSrc )
		{
			const auto	newAngPAL = ( chromaOriginPAL + angleSrc * chromaSector ) * chromaRadian;
			u = std::cos ( newAngPAL ) * chromaScale;
			v = std::sin ( newAngPAL ) * chromaScale;

			const auto	newAngNTSC = ( chromaOriginNTSC + angleSrc * chromaSector ) * chromaRadian;
			i = std::sin ( newAngNTSC ) * chromaScale;
			q = std::cos ( newAngNTSC ) * chromaScale;
		}

		auto store = [ &dst ] ( const int offset, const float luma, const float chroma1, const float chroma2 )
		{
			dst[ offset ]		= luma;
			dst[ offset + 1 ]	= chroma1;
			dst[ offset + 2 ]	= chroma2;
		};

		// PAL Even YUV
		store ( j,				y, u * oddCos - v * oddSin,			v * oddCos + u * oddSin );
		// PAL Odd YUV
		store ( j + 1 * 16 * 3,	y, u * oddCos - v * oddSin * -1.0f,	v * oddCos + u * oddSin * -1.0f );
		// NTSC YIQ (Sony decoder matrix)
		store ( j + 2 * 16 * 3,	y, i,								q );

		j += 3;
	}

	return dst;
}
//-----------------------------------------------------------------------------

colodore::rgbPalette colodore::generateRGB ( const int standard, const yuvPalette& src ) const
{
	rgbPalette	dst;

	switch ( standard )
	{
		case 0:	// PAL
			for ( auto index = 0; auto [ y, u, v ] : src )
				dst[ index++ ] = yuv2rgb ( uint8_t ( std::clamp ( y, 0.0f, 255.0f ) ), u, v );
			break;

		case 1:	// NTSC (Sony decoder matrix)
			for ( auto index = 0; auto [ y, q, i ] : src )
				dst[ index++ ] = yiq2rgb_sony ( uint8_t ( std::clamp ( y, 0.0f, 255.0f ) ), i, q );
			break;
	}

	return dst;
}
//-----------------------------------------------------------------------------
