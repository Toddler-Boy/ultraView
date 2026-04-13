#include "GUI_Main.h"

GUI_Main::GUI_Main ( juce::AudioDeviceManager& deviceManager )
	: juce::Component ( "main" )
	, pages ( deviceManager )
	, sidebarLeft ( pages )
{
	// Sidebar left (main menu and mini-playlists)
	sidebarLeft.addAndMakeVisible ( badge );
	addAndMakeVisible ( sidebarLeft );

	// Sidebar right (STIL and visualizations)
	addAndMakeVisible ( sidebarRight );

	// Footer with transport, volume control, etc.
	addAndMakeVisible ( footer );

	// Pages (search, playlist, history, crt settings, etc.)
	addAndMakeVisible ( pages );
}
//-----------------------------------------------------------------------------

void GUI_Main::resized ()
{
	const static juce::StringArray	sidebarForbidden { "settings", "crt" };

	layout.setConstant ( "sidebarRightAllowed", sidebarForbidden.contains ( pages.getPage () ) ? 0 : 1 );

	layout.setLayout ( {	paths::getDataRoot ( "UI/layouts/constants.json" ),
							paths::getDataRoot ( "UI/layouts/screens/main.json" ) } );
}
//-----------------------------------------------------------------------------
