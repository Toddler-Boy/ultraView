#include "GUI_Main.h"

GUI_Main::GUI_Main ( juce::AudioDeviceManager& deviceManager )
	: juce::Component ( "main" )
{
	addAndMakeVisible ( crt );
}
//-----------------------------------------------------------------------------

void GUI_Main::resized ()
{
	crt.setBounds ( getLocalBounds () );
}
//-----------------------------------------------------------------------------
