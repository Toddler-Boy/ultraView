#include "GUI_Stripes.h"

//-----------------------------------------------------------------------------

GUI_Stripes::GUI_Stripes ()
{
	setName ( "stripes" );
	setInterceptsMouseClicks ( false, false );
}
//-----------------------------------------------------------------------------

void GUI_Stripes::paint ( juce::Graphics& g )
{
	static juce::Colour	colors[] =
	{
		juce::Colour ( 0xff'F20029 ),
		juce::Colour ( 0xff'FD7801 ),
		juce::Colour ( 0xff'F2F540 ),
		juce::Colour ( 0xff'48CC31 ),
		juce::Colour ( 0xff'14A3FD ),
	};

	constexpr auto	numStripes = int ( std::size ( colors ) );

	auto	b = getLocalBounds ().toFloat ().reduced ( 0.0f, 0.5f );
	const auto	h = b.getHeight () / ( numStripes * 2 - 1 );

	for ( const auto col : colors )
	{
		g.setColour ( col.withMultipliedSaturation ( 0.75f ) );
		g.fillRoundedRectangle ( b.removeFromTop ( h ), h / 2.0f );
		b.removeFromTop ( h );
	}
}
//-----------------------------------------------------------------------------
