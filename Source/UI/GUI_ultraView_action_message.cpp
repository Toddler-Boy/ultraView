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
	else if ( cmd == "stream-status" )
	{
		Z_INFO ( "Stream status " << params[ 0 ] << " " << params[ 1 ] );
	}
	else if ( cmd == "c64run" )
	{
		Z_INFO ( "Upload " << params[ 0 ].quoted () );

		juce::MemoryBlock	mb;
		if ( ! juce::File ( params[ 0 ] ).loadFileAsData ( mb ) )
		{
			Z_ERR ( "Failed to load file: " << params[ 0 ].quoted () );
			return;
		}

		const auto	extension = params[ 0 ].fromLastOccurrenceOf ( ".", false, false ).toLowerCase ();

		network.post ( "v1/runners:run_" + extension, mb, [ this, params ] ( const juce::var& response, const int statusCode )
		{
			if ( statusCode != 200 )
			{
				Z_ERR ( "Failed to upload file: " << params[ 0 ].quoted () << "\n" << response[ "errors" ].toString () );
				return;
			}

			// Switch C64 into PAL mode
			network.put ( "v1/configs/U64 Specific Settings/System Mode", {}, nullptr, { "value", "PAL" } );
		} );
	}
	else
	{
		Z_ERR ( "Unknown action: " << message );
	}
}
//-----------------------------------------------------------------------------
