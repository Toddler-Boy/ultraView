#include "C64u_Scanner.h"

//-----------------------------------------------------------------------------

C64uScanner::C64uScanner () : juce::Thread ( "C64uScanner" )
{
}
//-----------------------------------------------------------------------------

C64uScanner::~C64uScanner ()
{
	stopThread ( 100 );
}
//-----------------------------------------------------------------------------

void C64uScanner::scan ( ScannerCallback _callback )
{
	callback = std::move ( _callback );

	startThread ( juce::Thread::Priority::low );
}
//-----------------------------------------------------------------------------

void C64uScanner::run ()
{
	auto    local = juce::IPAddress::getLocalAddress ();

	if ( local.isNull () )
	{
		if ( callback )
			callback ( {} );

		return;
	}

	// Extract subnet (e.g., 192.168.1.)
	auto    base = local.toString ().upToLastOccurrenceOf ( ".", true, false );
	const auto  thisMachine = local.toString ().fromLastOccurrenceOf ( ".", false, false ).getIntValue ();

	juce::ThreadPool	pool ( 20, 0, juce::Thread::Priority::low );

	for ( auto i = 1; i <= 254; ++i )
	{
		if ( i == thisMachine )
			continue;

		auto    targetIP = base + juce::String ( i );

		pool.addJob ( [ targetIP, this ]
		{
			juce::StreamingSocket   socket;

			if ( socket.connect ( targetIP, 80, 200 ) )
			{
				if ( auto hostName = isActualC64u ( socket ); hostName.isNotEmpty () )
				{
					const juce::ScopedLock sl ( resultsLock );

					foundDevices.add ( targetIP + " " + hostName );
				}
			}
		} );
	}

	while ( pool.getNumJobs () > 0 )
	{
		if ( threadShouldExit () )
		{
			pool.removeAllJobs ( true, 300 );
			return;
		}

		wait ( 50 );
	}

	if ( callback )
		callback ( foundDevices );
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
		juce::String    response ( buffer );

		if ( response.contains ( "HTTP/1.1 200 OK\r\n" ) && response.containsIgnoreCase ( "Ultimate" ) )
		{
			const auto	bodyStart = response.indexOf ( "\r\n\r\n" );
			const auto	json = juce::JSON::parse ( response.substring ( bodyStart + 4 ) );

			return json[ "hostname" ].toString ();
		}
	}
	return {};
}
//-----------------------------------------------------------------------------
