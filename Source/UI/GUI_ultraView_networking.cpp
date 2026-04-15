#include "GUI_ultraView.h"

//-----------------------------------------------------------------------------

void GUI_ultraView::findC64OnNetwork ()
{
	const auto	lastIP = settings->get<juce::String> ( "Network/last-ip" ).trim ();

	c64uScanner.scan ( [ this ] ( const juce::String& ip )
	{
		if ( ip.isEmpty () )
		{
			getTopLevelComponent ()->setName ( ProjectInfo::projectName + juce::String ( " " ) + ProjectInfo::versionString + juce::String ( " - Not connected" ) );

			Z_ERR ( "No C64 found on the network" );
			return;
		}

		Z_LOG ( "Found C64 at " + ip );

		const auto	ipOnly = ip.upToFirstOccurrenceOf ( " ", false, false );

		getTopLevelComponent ()->setName ( ProjectInfo::projectName + juce::String ( " " ) + ProjectInfo::versionString + juce::String ( " - " ) + ip );

		settings->set ( "Network/last-ip", ipOnly );
		network.setBaseAddress ( "http://" + ipOnly );

		setupNetworking ();

	}, lastIP );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::setupNetworking ()
{
	// Start stream
	network.get ( "v1/configs/Data Streams", [ this ] ( const juce::var& result, const int httpCode )
	{
		if ( httpCode < 200 || httpCode >= 300 )
			return;

		const auto	videoAddress = result[ "Data Streams" ][ "Stream VIC to" ];
		const auto	audioAddress = result[ "Data Streams" ][ "Stream Audio to" ];

		network.put ( "v1/streams/video:start", {}, nullptr, { "ip", videoAddress } );
		network.put ( "v1/streams/audio:start", {}, nullptr, { "ip", audioAddress } );

		mainScreen.crt.setStreamAddress ( videoAddress );
		UI::sendGlobalMessage ( "stream-status audio {}", c64uReceiver.start ( audioAddress ).quoted () );
	} );
}
//-----------------------------------------------------------------------------
