#pragma once

#include <cmath>

#if defined(_MSC_VER) && !defined(__clang__) && !defined(__INTEL_COMPILER)
	#define fxinline __forceinline
#else
	#define fxinline inline __attribute__((always_inline))
#endif

namespace fast
{
	fxinline float lerp ( const float a, const float b, const float t )	{	return a + t * ( b - a );	}
	fxinline float pow2 ( const float a ) { return a * a;  }
	fxinline float tanh ( const float x )
	{
		const auto	ax = std::fabs ( x );
		const auto	x2 = x * x;
		const auto	z = x * ( 0.773062670268356f + ax + ( 0.757118539838817f + 0.0139332362248817f * x2 * x2 ) * x2 * ax );

		return z / ( 0.795956503022967f + std::fabs ( z ) );
	}

	fxinline float sin ( const float x )
	{
		return -0.000182690409228785f * x * x * x * x * x * x * x
				+ 0.00830460224186793f * x * x * x * x * x
				- 0.166651012143690f * x * x * x
				+ x;
	}

	fxinline bool isEqual ( const float a, const float b )	{	return std::abs ( a - b ) < 1e-6f;	}
}
//-----------------------------------------------------------------------------

namespace fx::helpers
{
	constexpr auto dbToLin = [] ( float db ) -> float { return std::pow ( 10.0f, db * 0.05f ); };
}
//-----------------------------------------------------------------------------

class SmoothedValue
{
public:
	SmoothedValue ( const float initVal = 0.0f ) noexcept { setAndSnap ( initVal ); }

	void set ( const float val ) noexcept { dstValue = val; }
	[[ nodiscard ]] float get () const noexcept { return curValue; }
	void snap () noexcept	{ curValue = dstValue; }

	void step () noexcept { curValue = fast::lerp ( curValue, dstValue, fast ); }
	void stepSlow () noexcept { curValue = fast::lerp ( curValue, dstValue, slow ); }

	void setAndSnap ( const float val ) {
		set ( val );
		snap ();
	}

	[[ nodiscard ]] float getAndStep () noexcept {
		const auto	ret = get ();
		step ();
		return ret;
	}

	[[ nodiscard ]] float getAndStepSlow () noexcept {
		const auto	ret = get ();
		stepSlow ();
		return ret;
	}

	static constexpr auto	minVolume = 1.0f / 65535.0f;
	static constexpr auto	normVolume = 1.0f - minVolume;

	[[ nodiscard ]] bool restingAtZero () const noexcept { return dstValue < minVolume && curValue < minVolume; }
	[[ nodiscard ]] bool restingAtOne () const noexcept { return dstValue >= normVolume && curValue >= normVolume; }

private:
	static constexpr auto	fast = ( 1.0f / ( 44.1f * 3.0234f ) );
	static constexpr auto	slow = ( 1.0f / ( 44.1f * 1000.0f ) );

	float	curValue = 0.0f;
	float	dstValue = 0.0f;
};
//-----------------------------------------------------------------------------
