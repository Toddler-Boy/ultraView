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
		if ( extension == "crt" )
		{
			const auto	cartType = juce::ByteOrder::bigEndianShort ( static_cast<const char*>( mb.getData () ) + 22 );

			if ( cartType == 32 )
			{
				c64_reboot ();
				juce::Timer::callAfterDelay ( 2000, [ this, mb, filename = params[ 0 ] ] {	c64_run ( "crt", mb, filename );	} );
				return;
			}
		}

		c64_run ( extension, mb, params[ 0 ] );
	}
	else if ( cmd == "c64action" )
	{
		network.put ( "v1/machine:" + params[ 0 ], {} );
	}
	else
	{
		Z_ERR ( "Unknown action: " << message );
	}
}
//-----------------------------------------------------------------------------
