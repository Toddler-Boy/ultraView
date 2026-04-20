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

	auto    local = juce::IPAddress ();
	for ( const auto& adapter : juce::IPAddress::getAllAddresses () )
	{
		const auto	firstByte = adapter.address[ 0 ];
		if ( firstByte == 10 || firstByte == 172 || firstByte == 192 )
		{
			local = adapter;
			break;
		}
	}

	if ( local.isNull () )
	{
		Z_ERR ( "C64uScanner failed to get local IP address" );

		if ( callback )
			callback ( {} );

		return;
	}

	const auto	base = local.toString ().upToLastOccurrenceOf ( ".", true, false );

	// Let's try IP 64 first, as large portion of users will probably pick that one...
	auto	lastIndex = 64;
	if ( const auto lastBase = lastIP.upToLastOccurrenceOf ( ".", true, false ); base == lastBase )
		lastIndex = lastIP.fromLastOccurrenceOf ( ".", false, false ).getIntValue ();

	// Get this machine's IP address, so we can skip it in the scan
	const auto	thisMachine = local.toString ().fromLastOccurrenceOf ( ".", false, false ).getIntValue ();

	juce::ThreadPool	pool ( 20, juce::Thread::osDefaultStackSize, juce::Thread::Priority::low );

	Z_LOG ( "C64uScanner loop started" );

	for ( auto i = 0; i < 256; ++i )
	{
		const auto	ip = ( i + lastIndex ) & 255;

		if ( ip == 0 || ip == 255 || ip == thisMachine )
			continue;

		auto    targetIP = base + juce::String ( ip );

		pool.addJob ( [ &pool, targetIP, this ]
		{
			juce::StreamingSocket   socket;

			if ( socket.connect ( targetIP, 80, 200 ) )
			{
				if ( auto hostName = isActualC64u ( socket ); hostName.isNotEmpty () )
				{
					if ( callback )
						callback ( targetIP + " (" + hostName + ")" );

					pool.removeAllJobs ( true, 300 );
				}
			}
			else
			{
				Z_ERR ( "C64uScanner failed to connect to " + targetIP );
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
