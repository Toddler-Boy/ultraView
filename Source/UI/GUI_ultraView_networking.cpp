#include "GUI_ultraView.h"

//-----------------------------------------------------------------------------

void GUI_ultraView::findC64OnNetwork ()
{
	c64uScanner.scan ( [ this ] ( const juce::StringArray& ip )
	{
		for ( const auto& address : ip )
		{
			Z_LOG ( "Found C64 at " + address );
		}
//		network.setBaseAddress ( "http://" + ip );
//		setupNetworking ();
	} );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::setupNetworking ()
{
	// Start stream
	network.get ( "v1/configs/Data Streams", [ this ] ( const juce::StringPairArray& result, const int httpCode )
	{
		if ( httpCode != 200 )
			return;

		const auto	videoAddress = result[ "Data Streams/Stream VIC to" ];
		const auto	audioAddress = result[ "Data Streams/Stream Audio to" ];

		network.put ( "v1/streams/video:start", {}, nullptr, { "ip", videoAddress } );
		network.put ( "v1/streams/audio:start", {}, nullptr, { "ip", audioAddress } );

		mainScreen.crt.setStreamAddress ( videoAddress );
		UI::sendGlobalMessage ( "stream-status audio {}", c64uReceiver.start ( audioAddress ).quoted () );
	} );
}
//-----------------------------------------------------------------------------
