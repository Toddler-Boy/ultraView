#include "GUI_ultraView_logo.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_ultraView_logo::GUI_ultraView_logo ( const juce::String& name, const juce::String& resource )
	: juce::Component ( name )
{
	setInterceptsMouseClicks ( false, false );

	auto	[ logo, _ ] = UI::getSVG ( resource );

	path = logo->getOutlineAsPath ();
}
//-----------------------------------------------------------------------------

void GUI_ultraView_logo::paint ( juce::Graphics& g )
{
	auto	b = getLocalBounds ().toFloat ();

	// Draw logo
	g.setColour ( UI::getShade ( 1.0f ) );
	g.fillPath ( path, path.getTransformToScaleToFit ( b, true, juce::Justification::centred ) );
}
//-----------------------------------------------------------------------------
