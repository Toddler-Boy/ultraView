#pragma once

//-----------------------------------------------------------------------------

namespace VIC2_Render
{
	constexpr auto	outerUnscaledWidth = 384;
	constexpr auto	outerUnscaledHeight = 272;

	struct settings
	{
		enum colorStandard : int8_t { PAL, NTSC };
		colorStandard	standard = PAL;

		bool	firstLuma = false;

		// All three values between 0.0 and 100.0
		float	brightness = 50.0f;
		float	contrast = 100.0f;
		float	saturation = 50.0f;

		bool	raw = false;

		[[ nodiscard ]] bool needsNewPalette ( const settings& other ) const
		{
			return firstLuma != other.firstLuma || raw != other.raw;
		}
	};
}
