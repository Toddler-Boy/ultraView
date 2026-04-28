#include "C64u_Scanner.h"
#include "NetworkHardwareChecker.h"

//-----------------------------------------------------------------------------

C64uScanner::C64uScanner () : juce::Thread ( "C64uScanner" )
{
}
//-----------------------------------------------------------------------------

C64uScanner::~C64uScanner ()
{
	stopThread ( -1 );
}
//-----------------------------------------------------------------------------

void C64uScanner::scan ( ScannerCallback _callback, const juce::String& _lastIP )
{
	callback = std::move ( _callback );
	lastIP = std::move ( _lastIP );

	startThread ( juce::Thread::Priority::low );
}
//-----------------------------------------------------------------------------

void C64uScanner::run ()
{
	Z_LOG ( "C64uScanner started" );

	NetworkHardwareChecker	hardware;

	auto	privateNetworks = juce::IPAddress::getAllAddresses ();

	// Remove all non-private network adapters
	privateNetworks.removeIf ( [ &hardware ] ( const juce::IPAddress& addr )
	{
		const auto&	b = addr.address;

		const auto	isPrivate = ( b[ 0 ] == 10 ) ||
								( b[ 0 ] == 192 && b[ 1 ] == 168 ) ||
								( b[ 0 ] == 172 && ( b[ 1 ] >= 16 && b[ 1 ] <= 31 ) );

		// It's a public IP, remove it
		if ( ! isPrivate )
			return true;

		// Then check if it's currently wired and active
		return ! hardware.isWiredAndActive ( addr );
	} );

	// Sort descending, so that 192.168.x.x is scanned first, as it's the most common private network range
	std::ranges::sort ( privateNetworks, [] ( const juce::IPAddress& a, const juce::IPAddress& b )
	{
		return a.address[ 0 ] > b.address[ 0 ];
	} );

	if ( privateNetworks.isEmpty ()  )
	{
		Z_ERR ( "C64uScanner failed to find any private network adapters" );

		if ( callback )
			callback ( {} );

		return;
	}

	// Create complete list of IPs to scan
	juce::StringArray	ipsToScan;

	for ( const auto& ip : privateNetworks )
	{
		const auto	base = ip.toString ().upToLastOccurrenceOf ( ".", true, false );

		// Let's try IP 64 first, as large portion of users will probably pick that one...
		constexpr auto	firstIndex = 64;
		for ( auto i = 0; i < 256; ++i )
		{
			const auto	scanIp = ( i + firstIndex ) & 255;

			if ( scanIp == 0 || scanIp == 255 )
				continue;

			ipsToScan.add ( base + juce::String ( scanIp ) );
		}

		// Remove local IP from the list, as we don't want to scan it
		ipsToScan.removeString ( ip.toString () );
	}

	if ( lastIP.isNotEmpty () )
	{
		ipsToScan.removeString ( lastIP );
		ipsToScan.insert ( 0, lastIP );
	}

	juce::ThreadPool	pool ( 20, juce::Thread::osDefaultStackSize, juce::Thread::Priority::low );

	Z_LOG ( "C64uScanner loop started" );

	for ( const auto& targetIP : ipsToScan )
	{
		pool.addJob ( [ &pool, targetIP, this ]
		{
			juce::StreamingSocket   socket;

			if ( socket.connect ( targetIP, 80, 200 ) )
			{
				Z_LOG ( "Scanned device on " + targetIP );

				if ( auto hostName = isActualC64u ( socket ); hostName.isNotEmpty () )
				{
					if ( callback )
						callback ( targetIP + " (" + hostName + ")" );

					pool.removeAllJobs ( false, 300 );
				}
			}
		} );
	}

	while ( pool.getNumJobs () > 0 )
	{
		if ( threadShouldExit () )
		{
			pool.removeAllJobs ( false, 300 );
			return;
		}

		wait ( 50 );
	}
}
//-----------------------------------------------------------------------------

juce::String C64uScanner::isActualC64u ( juce::StreamingSocket& socket )
{
	static const juce::String request = "GET /v1/info HTTP/1.1\r\n\r\n";

	socket.write ( request.toRawUTF8 (), request.length () );

	char	buffer[ 1024 ];
	if ( auto numBytes = socket.read ( buffer, sizeof ( buffer ) - 1, true ); numBytes > 0 )
	{
		buffer[ numBytes ] = 0;
		const juce::String    response ( buffer );

		if ( response.contains ( "HTTP/1.1 200 OK\r\n" ) && response.containsIgnoreCase ( "Ultimate" ) )
		{
			const auto	bodyStart = response.indexOf ( "\r\n\r\n" );
			const auto	json = juce::JSON::parse ( response.substring ( bodyStart + 4 ) );

			Z_INFO ( "C64uScanner found a C64u at " + socket.getHostName () + " with hostname: " + json[ "hostname" ].toString () );

			return json[ "hostname" ].toString ();
		}
	}
	else
	{
		Z_ERR ( "C64uScanner failed to read from " + socket.getHostName () );
	}
	return {};
}
//-----------------------------------------------------------------------------
