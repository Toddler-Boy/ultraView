#include "GUI_ultraView.h"

//-----------------------------------------------------------------------------

void GUI_ultraView::c64_reboot ()
{
	network.put ( "v1/machine:reboot" );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::c64_run ( const juce::String& type, const juce::MemoryBlock& mb, const juce::String& filename )
{
	network.post ( "v1/runners:run_" + type, mb, [ filename ] ( const juce::var& response, const int statusCode )
	{
		if ( statusCode != 200 )
		{
			Z_ERR ( "Failed to upload file: " << filename.quoted () << "\n" << response[ "errors" ].toString () );
		}
	} );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::c64_forceSystemMode ( const juce::String& mode )
{
	network.put ( "v1/configs/U64 Specific Settings/System Mode", { "value", mode } );
}
//-----------------------------------------------------------------------------
