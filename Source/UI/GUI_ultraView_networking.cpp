#include "GUI_ultraView.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

void GUI_ultraView::setTitleStatus ( const juce::String& status )
{
	juce::MessageManager::callAsync ( [ this, status ] {
		getTopLevelComponent ()->setName ( helpers::appTitle () + " - " + status );
	} );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::findC64OnNetwork ()
{
	auto	lastIP = settings->get<juce::String> ( "network/last-ip" ).trim ();

	netScanning = true;
	setTitleStatus ( strings->get ( "network/searching" ) );

	c64uScanner.scan ( [ this ] ( const juce::String& ip )
	{
		netScanning = false;

		if ( ip.isEmpty () )
		{
			Z_ERR ( "No C64 found on the network" );
			return;
		}

		Z_LOG ( "Found C64 at " + ip );

		const auto	ipOnly = ip.upToFirstOccurrenceOf ( " ", false, false );

		{
			const juce::ScopedLock	sl ( titleLock );
			connectedTitle = ip;
		}
		setTitleStatus ( ip );

		settings->set ( "network/last-ip", ipOnly );
		network.setBaseAddress ( "http://" + ipOnly );
		network.setC64uPassword ( settings->get<juce::String> ( "network/password" ) );

		setupNetworking ();

	}, lastIP );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::timerCallback ()
{
	reconnectTick ();
}
//-----------------------------------------------------------------------------

void GUI_ultraView::reconnectTick ()
{
	if ( netReceiving || netScanning || netProbing )
		return;

	const auto	lastIP = settings->get<juce::String> ( "network/last-ip" ).trim ();

	// A few quick probes of the known address, then a full sweep in case the
	// C64u came back under a new one
	if ( lastIP.isEmpty () || probeFailures >= 4 )
	{
		probeFailures = 0;
		findC64OnNetwork ();
		return;
	}

	netProbing = true;
	setTitleStatus ( strings->get ( "network/waiting" ) );

	network.setBaseAddress ( "http://" + lastIP );
	network.setC64uPassword ( settings->get<juce::String> ( "network/password" ) );

	network.get ( "v1/info", {}, [ this, lastIP ] ( const juce::var& result, const int httpCode )
	{
		netProbing = false;

		if ( httpCode < 200 || httpCode >= 300 )
		{
			++probeFailures;
			return;
		}

		probeFailures = 0;

		const auto	title = lastIP + " (" + result[ "hostname" ].toString () + ")";
		{
			const juce::ScopedLock	sl ( titleLock );
			connectedTitle = title;
		}
		setTitleStatus ( title );

		setupNetworking ();
	} );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::setupNetworking ()
{
	// Start stream
	network.get ( "v1/configs/Data Streams", {}, [ this ] ( const juce::var& result, const int httpCode )
	{
		if ( httpCode < 200 || httpCode >= 300 )
			return;

		const auto	videoAddress = result[ "Data Streams" ][ "Stream VIC to" ];
		const auto	audioAddress = result[ "Data Streams" ][ "Stream Audio to" ];

		network.put ( "v1/streams/video:start", { "ip", videoAddress } );
		network.put ( "v1/streams/audio:start", { "ip", audioAddress } );

		mainScreen.crt.setStreamAddress ( videoAddress );

		c64uReceiver.stop ();
		UI::sendGlobalMessage ( "stream-status audio {}", c64uReceiver.start ( audioAddress ).quoted () );
	} );
}
//-----------------------------------------------------------------------------
