#include "GUI_ultraView.h"

//-----------------------------------------------------------------------------

void GUI_ultraView::setupNetworking ()
{
	// Start stream
	network.get ( "v1/configs/Data Streams", [ this ] ( const juce::StringPairArray& result, const int httpCode )
	{
		const auto	videoAddress = result[ "Data Streams/Stream VIC to" ];
		const auto	audioAddress = result[ "Data Streams/Stream Audio to" ];

		network.put ( "v1/streams/video:start", {}, nullptr, { "ip", videoAddress } );
		network.put ( "v1/streams/audio:start", {}, nullptr, { "ip", audioAddress } );

		mainScreen.crt.setStreamAddress ( videoAddress );
		UI::sendGlobalMessage ( "stream-status audio {}", c64uReceiver.start ( audioAddress ).quoted () );
	} );
}
//-----------------------------------------------------------------------------
