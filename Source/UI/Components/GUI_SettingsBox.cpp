#include "GUI_SettingsBox.h"

#include "UI/SID_LookAndFeel.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_SettingsBox::GUI_SettingsBox ( const juce::String& n )
	: juce::Component ( n )
{
}
//-----------------------------------------------------------------------------

void GUI_SettingsBox::paint ( juce::Graphics& g )
{
	constexpr auto	blend = 0.067f;

	g.setColour ( UI::getShade ( blend ) );
	SID_LookAndFeel::drawOutlinedRect ( g, getLocalBounds ().toFloat (), 10.0f, 1.0f, UI::getShade ( blend * 2.0f ) );
}
//-----------------------------------------------------------------------------
