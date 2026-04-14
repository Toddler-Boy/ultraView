#include "GUI_VIC2_Palette.h"

#include "UI/Misc/colodore.h"

//-----------------------------------------------------------------------------

GUI_VIC2_Palette::GUI_VIC2_Palette ()
{
	setOpaque ( true );

	setSettings ( 0, 50.0f, 100.0f, 50.0f, false );

	setInterceptsMouseClicks ( false, false );
}
//-----------------------------------------------------------------------------

void GUI_VIC2_Palette::paint ( juce::Graphics& g )
{
	auto	b = getLocalBounds ().toFloat ().expanded ( 0.0f, 1.0f );
	const auto	w = b.getWidth () / palette.size ();

	for ( const auto& col : palette )
	{
		g.setColour ( col );
		g.fillRect ( b.removeFromLeft ( w ).withWidth ( w + 1.0f ) );
	}
}
//-----------------------------------------------------------------------------

void GUI_VIC2_Palette::setSettings ( const int standard, const float brightness, const float contrast, const float saturation, const bool earlyLuma )
{
	colodore	colo;

	const auto	yuvPal = colo.generateYUV ( standard, brightness, contrast, saturation, earlyLuma );
	const auto	rgbPal = colo.generateRGB ( standard, yuvPal );

	for ( auto index = 0; const auto col : rgbPal )
		palette[ index++ ] = juce::Colour ( col );

	repaint ();
}
//-----------------------------------------------------------------------------
