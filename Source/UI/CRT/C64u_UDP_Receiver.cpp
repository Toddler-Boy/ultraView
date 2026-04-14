#include "C64u_UDP_Receiver.h"

//-----------------------------------------------------------------------------

C64u_UDP_Receiver::C64u_UDP_Receiver ( const streamType _type)
	: juce::Thread ( "C64u_UDP_Receiver_Thread:" + juce::String ( _type == video ? "Video" : "Audio" ) )
	, type ( _type )
{
}
//-----------------------------------------------------------------------------

C64u_UDP_Receiver::~C64u_UDP_Receiver ()
{
	stop ();
}
//-----------------------------------------------------------------------------

void C64u_UDP_Receiver::start ( const juce::String& address, int newPort )
{
	// Socket object already exists, call stop first
	jassert ( ! socket.get () );

	// Ensure buffers have been assigned before starting the thread
	jassert (	( type == video && videoBuffers[ 0 ] && videoBuffers[ 1 ] )
			||	( type == audio && audioBuffers[ 0 ] && audioBuffers[ 1 ] ) );

	// Set a callback function first
	jassert (	( type == video && onVideoFrame )
			||	( type == audio && onAudioChunk ) );

	hasTimedOut = true;

	socket = std::make_unique<juce::DatagramSocket> ( false );
	if ( ! socket )
	{
		Z_ERR ( "Failed to create C64u UDP socket" );
		return;
	}

	if ( ! socket->bindToPort ( newPort ) )
	{
		Z_ERR ( "Failed to bind C64u UDP socket to port " << newPort );
		return;
	}
	socket->setEnablePortReuse ( true );

	if ( ! socket->joinMulticast ( address ) )
	{
		Z_ERR ( "Failed to join C64u UDP multicast group at address " << address );
		socket->shutdown ();
		socket.reset ();
		return;
	}

	currentMulticastAddress = address;

	lastReadTime = std::chrono::steady_clock::now ();
	startThread ( Priority::normal );
}
//-----------------------------------------------------------------------------

void C64u_UDP_Receiver::stop ()
{
	if ( ! socket )
		return;

	socket->leaveMulticast ( currentMulticastAddress );

	stopThread ( 200 );

	socket->shutdown ();
	socket.reset ();
}
//-----------------------------------------------------------------------------

void C64u_UDP_Receiver::setVideoBuffers ( uint8_t* buffer1, uint8_t* buffer2 )
{
	videoBuffers[ 0 ] = buffer1;
	videoBuffers[ 1 ] = buffer2;
}
//-----------------------------------------------------------------------------

void C64u_UDP_Receiver::setAudioBuffers ( float* buffer1, float* buffer2 )
{
	audioBuffers[ 0 ] = buffer1;
	audioBuffers[ 1 ] = buffer2;
}
//-----------------------------------------------------------------------------

void C64u_UDP_Receiver::run ()
{
	// MTU size buffer
	uint8_t		tempBuffer[ 780 ];

	const auto	packetSize = type == video ? 780 : 770;

	while ( ! threadShouldExit () )
	{
		auto	dataReceived = false;

		if ( socket->waitUntilReady ( true, 50 ) > 0 )
		{
			const auto	bytesRead = socket->read ( tempBuffer, packetSize, true );

			if ( bytesRead == packetSize )
			{
				if ( type == video )
					processIncomingVideo ( tempBuffer );
				else
					processIncomingAudio ( reinterpret_cast<const int16_t*> ( tempBuffer ) );

				lastReadTime = std::chrono::steady_clock::now ();
				dataReceived = true;

				if ( onStatusChange && hasTimedOut )
					onStatusChange ( true );
			}
		}

		if ( ! dataReceived )
		{
			if ( hasTimedOut )
				continue;

			const auto	now = std::chrono::steady_clock::now ();
			if ( std::chrono::duration_cast<std::chrono::milliseconds>( now - lastReadTime ) >= timeoutThresholdMs )
			{
				hasTimedOut = true;

				if ( onStatusChange )
					onStatusChange ( false );
			}
		}
		else
		{
			hasTimedOut = false;
		}
	}
}
//-----------------------------------------------------------------------------

void C64u_UDP_Receiver::processIncomingVideo ( const uint8_t* data )
{
	struct C64uHeader
	{
		uint16_t	sequenceNumber;		// increments per packet
		uint16_t	frameNumber;
		uint16_t	lineNumber;			// bit 15 indicates last packet of frame
		uint16_t	pixelsPerLine;		// always 384
		uint8_t		linesPerPacket;		// always 4
		uint8_t		bitsPerPixel;		// always 4
		uint16_t	encodingType;		// always 0 (uncompressed)
	};

	// C64 Ultimate header
	const auto	header = reinterpret_cast<const C64uHeader*> ( data );

	// Check if this is a valid C64 Ultimate packet
	if ( header->pixelsPerLine != 384 || header->linesPerPacket != 4 || header->bitsPerPixel != 4 || header->encodingType != 0 )
		return;

	auto	offset = ( header->lineNumber & 0x7FFFu ) * header->pixelsPerLine;
	jassert ( offset < 384 * 272 );

	constexpr auto	payloadSize = 780 - sizeof ( C64uHeader );
	const auto	srcNibbles = data + sizeof ( C64uHeader );
	const auto	dstBuffer = videoBuffers[ videoBufferIndex ];

	for ( auto i = 0; i < payloadSize; ++i )
	{
		const auto	byte = srcNibbles[ i ];

		dstBuffer[ offset++ ] = byte & 0xF;
		dstBuffer[ offset++ ] = byte >> 4;
	}

	if ( header->lineNumber & 0x8000 )
	{
		constexpr auto	linesPerNTSCFrame = 240;
		constexpr auto	lastLineMarker = uint16_t ( 0x8000 + linesPerNTSCFrame - 4 );

		onVideoFrame ( videoBufferIndex, header->lineNumber == lastLineMarker );
		videoBufferIndex ^= 1;
	}
}
//-----------------------------------------------------------------------------

void C64u_UDP_Receiver::processIncomingAudio ( const int16_t* data )
{
	struct C64uHeader
	{
		uint16_t	sequenceNumber;		// increments per packet
	};

	// C64 Ultimate header
	const auto	header = reinterpret_cast<const C64uHeader*> ( data );

	constexpr auto	payloadSize = 770 - sizeof ( C64uHeader );
	constexpr auto	samplesPerPacket = payloadSize / 4;

	const auto	srcData = data + ( sizeof ( C64uHeader ) / sizeof ( *data ) );
	const auto	dstL = audioBuffers[ 0 ];
	const auto	dstR = audioBuffers[ 1 ];

	constexpr auto	scale = 1.0f / 32768.0f;

	for ( auto i = 0; i < samplesPerPacket; ++i )
	{
		dstL[ i ] = srcData[ i * 2     ] * scale;
		dstR[ i ] = srcData[ i * 2 + 1 ] * scale;
	}

	onAudioChunk ( header->sequenceNumber );
}
//-----------------------------------------------------------------------------
