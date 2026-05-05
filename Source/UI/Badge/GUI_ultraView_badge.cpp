#include "GUI_ultraView_badge.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_ultraView_Badge::GUI_ultraView_Badge ()
	: juce::Component ( "badge" )
{
	setInterceptsMouseClicks ( false, false );

	addAndMakeVisible ( logoCl );
	addAndMakeVisible ( logoUltraView );
	addAndMakeVisible ( stripes );
	addAndMakeVisible ( logo64 );
}
//-----------------------------------------------------------------------------

void GUI_ultraView_Badge::paint ( juce::Graphics& g )
{
	const auto	b = getLocalBounds ().toFloat ();

	// Draw logo
	g.setColour ( UI::getShade ( 0.1f ) );
	g.fillRoundedRectangle ( b, b.getHeight () / 2.0f );
}
//-----------------------------------------------------------------------------
