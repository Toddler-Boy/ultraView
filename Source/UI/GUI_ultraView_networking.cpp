#include "GUI_ultraView.h"

#include "Globals/constants.h"
#include "ultra-shared/Helpers/PlatformHelper.h"

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

	c64uScanner.scan ( [ this ] ( const juce::String& ip, const bool passwordRequired )
	{
		netScanning = false;

		if ( passwordRequired )
		{
			// Remember the machine so the probe loop watches it; rescanning
			// can't help until the password matches
			settings->set ( "network/last-ip", ip );
			setTitleStatus ( strings->get ( "network/password-required" ) );
			return;
		}

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

	// A heal cycle is still judging its streams; restarting them here would
	// flip the targets back and forth forever
	if ( healWatchdog.isTimerRunning () )
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

		// A refusal means the machine is there; keep watching it instead of
		// sweeping the network
		if ( httpCode == 403 )
		{
			setTitleStatus ( strings->get ( "network/password-required" ) );
			return;
		}

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

// Streams aimed elsewhere get this long to deliver before healStreams pulls
// them over, counted from the last start answer
constexpr auto	healWatchdogMs = 750;

#if JUCE_WINDOWS
// The verdict pass needs no grace on Windows: the firewall query settles
// whether the silence is a saved block rule
constexpr auto	blockedVerdictMs = healWatchdogMs;
#else
// A healed stream gets this long before its silence counts as locally blocked;
// generous, so a user still facing the OS permission prompt can answer it
constexpr auto	blockedVerdictMs = 5000;
#endif

//-----------------------------------------------------------------------------

void GUI_ultraView::setupNetworking ()
{
	// Start stream
	network.get ( "v1/configs/Data Streams", {}, [ this ] ( const juce::var& result, const int httpCode )
	{
		if ( httpCode < 200 || httpCode >= 300 )
			return;

		const auto	videoAddress = result[ "Data Streams" ][ "Stream VIC to" ].toString ();
		const auto	audioAddress = result[ "Data Streams" ][ "Stream Audio to" ].toString ();

		network.put ( "v1/streams/video:start", { "ip", videoAddress } );
		network.put ( "v1/streams/audio:start", { "ip", audioAddress }, [ this, videoAddress, audioAddress ] ( const juce::var&, const int )
		{
			juce::MessageManager::callAsync ( [ this, videoAddress, audioAddress ]
			{
				videoStreamTarget = videoAddress;
				audioStreamTarget = audioAddress;
				healWatchdog.startTimer ( healWatchdogMs );
			} );
		} );

		mainScreen.crt.setStreamAddress ( videoAddress );

		c64uReceiver.stop ();
		UI::sendGlobalMessage ( "stream-status audio {}", c64uReceiver.start ( audioAddress ).quoted () );
	} );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::healStreams ()
{
	healWatchdog.stopTimer ();

	const auto	ownIP = C64uScanner::localAddressFor ( settings->get<juce::String> ( "network/last-ip" ) );
	if ( ownIP.isEmpty () )
		return;

	// A target pointing at this machine has nothing left to heal; anything
	// else (multicast group, another machine) gets pulled over, keeping an
	// explicit port. The C64u's stored config stays untouched.
	const auto	retarget = [ &ownIP ] ( const juce::String& address )
	{
		const auto	port = address.fromFirstOccurrenceOf ( ":", false, false );
		return port.isEmpty () ? ownIP : ownIP + ":" + port;
	};

	auto	healed = false;		// pulled over, judge again next round
	auto	blocked = false;	// aimed at this machine yet silent: dropped locally

	if ( mainScreen.crt.isStreamStalled () )
	{
		if ( videoStreamTarget.upToFirstOccurrenceOf ( ":", false, false ) != ownIP )
		{
			videoStreamTarget = retarget ( videoStreamTarget );
			Z_LOG ( "Video stream is silent, retargeting it to " + videoStreamTarget );

			network.put ( "v1/streams/video:stop" );
			network.put ( "v1/streams/video:start", { "ip", videoStreamTarget } );

			mainScreen.crt.setStreamAddress ( videoStreamTarget );
			healed = true;
		}
		else
		{
			blocked = true;
		}
	}

	if ( c64uReceiver.isStalled () )
	{
		if ( audioStreamTarget.upToFirstOccurrenceOf ( ":", false, false ) != ownIP )
		{
			audioStreamTarget = retarget ( audioStreamTarget );
			Z_LOG ( "Audio stream is silent, retargeting it to " + audioStreamTarget );

			network.put ( "v1/streams/audio:stop" );
			network.put ( "v1/streams/audio:start", { "ip", audioStreamTarget } );

			c64uReceiver.stop ();
			UI::sendGlobalMessage ( "stream-status audio {}", c64uReceiver.start ( audioStreamTarget ).quoted () );
			healed = true;
		}
		else
		{
			blocked = true;
		}
	}

	if ( healed )
		healWatchdog.startTimer ( blockedVerdictMs );
	else if ( blocked )
	{
#if JUCE_WINDOWS
		// The firewall answers with certainty: no block rule means the user
		// simply hasn't decided yet, stay quiet
		if ( firewallBlocksThisApp () )
			showFirewallNotice ();
#else
		showFirewallNotice ();
#endif
	}
}
//-----------------------------------------------------------------------------

void GUI_ultraView::showFirewallNotice ()
{
	if ( std::exchange ( firewallNoticeShown, true ) )
		return;

	Z_ERR ( "Inbound stream data is being blocked on this machine" );

#if JUCE_WINDOWS || JUCE_MAC
 #if JUCE_WINDOWS
	const auto	messageKey = "network/blocked-windows";
	const auto	buttonKey = "network/blocked-open-windows";
 #else
	const auto	messageKey = "network/blocked-mac";
	const auto	buttonKey = "network/blocked-open-mac";
 #endif

	juce::NativeMessageBox::showAsync ( juce::MessageBoxOptions ()
											.withIconType ( juce::MessageBoxIconType::WarningIcon )
											.withTitle ( strings->get ( "network/blocked-title" ) )
											.withMessage ( strings->get ( messageKey ) )
											.withButton ( strings->get ( buttonKey ) )
											.withButton ( strings->get ( "network/blocked-close" ) )
											.withAssociatedComponent ( this ),
										[] ( const int result )
										{
											if ( result != 0 )
												return;

 #if JUCE_WINDOWS
											juce::ChildProcess ().start ( "control /name Microsoft.WindowsFirewall /page pageConfigureApps" );
 #else
											juce::URL ( "x-apple.systempreferences:com.apple.preference.security?Privacy_LocalNetwork" ).launchInDefaultBrowser ();
 #endif
										} );
#endif
}
//-----------------------------------------------------------------------------
