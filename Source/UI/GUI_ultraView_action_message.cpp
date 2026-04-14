#include "GUI_ultraView.h"

//-----------------------------------------------------------------------------

void GUI_ultraView::actionListenerCallback ( const juce::String& message )
{
	if ( message.isEmpty () )
		return;

	const auto	[ cmd, params ] = helpers::parseActionMessage ( message );

	if ( cmd == "volumeChanged" )
	{
//		updateVolume ();
	}
	else if ( cmd == "settingChanged" )
	{
		if ( params[ 0 ] == "ui" && params[ 1 ] == "allow-screensaver" )
			juce::Desktop::setScreenSaverEnabled ( preferences->get<bool> ( params[ 0 ], params[ 1 ] ) );
	}
	else if ( cmd == "restoreState" )
	{
		restoreState ();
	}
	else
	{
		Z_ERR ( "Unknown action: " << message );
	}
}
//-----------------------------------------------------------------------------
