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

juce::String C64u_UDP_Receiver::start ( const juce::String& address )
{
	if ( address.isEmpty () )
		return "Cannot start C64u UDP receiver: IP is empty";

	// Extract port from address, default to 11000 for video and 11001 for audio if not specified
	auto	newPort = address.fromFirstOccurrenceOf ( ":", false, false ).getIntValue ();
	if ( ! newPort )
		newPort = type == video ? 11000 : 11001;

	const auto	addressOnly = address.upToFirstOccurrenceOf ( ":", false, false );

	juce::IPAddress	ip ( addressOnly );
	if ( ip.toString () != addressOnly )
		return "Invalid IP address format: " + address.quoted ();

	if ( ip.isIPv6 )
		return "Invalid IP address format: " + addressOnly.quoted () + " is not IP4 format, but IPv6";

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
		return "Failed to create C64u UDP socket";

	if ( ! socket->bindToPort ( newPort ) )
		return "Failed to bind C64u UDP socket to port " + juce::String ( newPort );

	socket->setEnablePortReuse ( true );

	currentMulticastAddress = {};
	if ( ip.address[ 0 ] >= 224 && ip.address[ 0 ] <= 239 )
	{
		if ( ! socket->joinMulticast ( addressOnly ) )
		{
			socket->shutdown ();
			socket.reset ();
			return "Failed to join C64u UDP multicast group at address " + address;
		}

		currentMulticastAddress = addressOnly;
	}

	lastReadTime = std::chrono::steady_clock::now ();
	startThread ( Priority::normal );

	return "Streaming " + juce::String ( type == video ? "video" : "audio" ) + " from " + address;
}
//-----------------------------------------------------------------------------

void C64u_UDP_Receiver::stop ()
{
	if ( ! socket )
		return;

	if ( currentMulticastAddress.isNotEmpty () )
		socket->leaveMulticast ( currentMulticastAddress );

	stopThread ( -1 );

	socket->shutdown ();
	socket.reset ();
}
//-----------------------------------------------------------------------------

void C64u_UDP_Receiver::setVideoBuffers ( uint8_t* buffer1, uint8_t* buffer2 )
{
	// Both pointers must be 16-byte aligned for the SIMD operations in processIncomingVideo
	jassert ( ( reinterpret_cast<uintptr_t>( buffer1 ) & 15 ) == 0 );
	jassert ( ( reinterpret_cast<uintptr_t>( buffer2 ) & 15 ) == 0 );

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
	// Make sure that a video-packet is aligned to 16-byte boundary
	// for SIMD processing in processIncomingVideo
	struct alignas ( 16 )
	{
		uint8_t	padding[ 4 ];
		uint8_t	header[ 12 ];
		uint8_t	payload[ 768 ];

	} tempBuffer;

	const auto	packetSize = type == video ? 780 : 770;

	while ( ! threadShouldExit () )
	{
		auto	dataReceived = false;

		if ( socket->waitUntilReady ( true, 50 ) > 0 )
		{
			const auto	bytesRead = socket->read ( &tempBuffer.header[ 0 ], packetSize, true );

			if ( bytesRead == packetSize )
			{
				if ( type == video )
					processIncomingVideo ( &tempBuffer.header[ 0 ] );
				else
					processIncomingAudio ( reinterpret_cast<const int16_t*> ( &tempBuffer.header[ 0 ] ) );

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

#include <immintrin.h>

namespace
{

#if 0
inline void convert ( const uint8_t* __restrict__ srcData, uint8_t* __restrict__ _dstBuffer )
{
	constexpr auto	payloadSize = 768;

	const auto*	srcNibbles = std::assume_aligned<16> ( srcData );
	auto*		dstBuffer = std::assume_aligned<16> ( _dstBuffer );

	auto    offset = 0;

	#pragma clang loop vectorize(assume_safety)
	for ( auto i = 0; i < payloadSize; ++i )
	{
		const auto	byte = srcNibbles[ i ];

		dstBuffer[ offset++ ] = byte & 0xF;
		dstBuffer[ offset++ ] = byte >> 4;
	}
}
#endif

inline void expand_nibbles_final ( const uint8_t* __restrict src, uint8_t* __restrict dst )
{
	constexpr auto	payloadSize = 768;

	// 16-byte alignment is required for these instructions
	const auto	s = ( const __m128i* )std::assume_aligned<16> ( src );
	auto		d = ( __m128i* )std::assume_aligned<16> ( dst );

	const auto	mask = _mm_set1_epi8 ( 0x0F );

	// Process 32 bytes of input (64 output bytes) per iteration
	for ( auto i = 0; i < ( payloadSize / 32 ); ++i )
	{
		// Load two full 128-bit blocks (32 bytes of nibbles)
		const auto	in0 = _mm_load_si128 ( s + ( i * 2 ) );     // 16 bytes
		const auto	in1 = _mm_load_si128 ( s + ( i * 2 ) + 1 ); // 16 bytes

		// Expand block 0 (Low half)
		const auto	in0_low = _mm_and_si128 ( in0, mask );
		const auto	in0_high = _mm_and_si128 ( _mm_srli_epi16 ( in0, 4 ), mask );

		// Expand block 1 (High half)
		const auto	in1_low = _mm_and_si128 ( in1, mask );
		const auto	in1_high = _mm_and_si128 ( _mm_srli_epi16 ( in1, 4 ), mask );

		// Interleave and store (this produces 4 output vectors)
		_mm_store_si128 ( d + ( i * 4 ) + 0, _mm_unpacklo_epi8 ( in0_low, in0_high ) );
		_mm_store_si128 ( d + ( i * 4 ) + 1, _mm_unpackhi_epi8 ( in0_low, in0_high ) );
		_mm_store_si128 ( d + ( i * 4 ) + 2, _mm_unpacklo_epi8 ( in1_low, in1_high ) );
		_mm_store_si128 ( d + ( i * 4 ) + 3, _mm_unpackhi_epi8 ( in1_low, in1_high ) );
	}
}

}

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

	const auto	offset = ( header->lineNumber & 0x7FFFu ) * header->pixelsPerLine;
	jassert ( offset < 384 * 272 );

	expand_nibbles_final ( data + sizeof ( C64uHeader ), videoBuffers[ videoBufferIndex ] + offset );

	#if 0
	constexpr auto	payloadSize = 780 - sizeof ( C64uHeader );

	const auto* __restrict__	srcNibbles = std::assume_aligned<16> ( data + sizeof ( C64uHeader ) );
	auto*		__restrict__	dstBuffer = std::assume_aligned<16> ( videoBuffers[ videoBufferIndex ] );

	for ( auto i = 0; i < payloadSize; ++i )
	{
		const auto	byte = srcNibbles[ i ];

		dstBuffer[ offset++ ] = byte & 0xF;
		dstBuffer[ offset++ ] = byte >> 4;
	}
	#endif

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
