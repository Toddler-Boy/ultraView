#pragma once

#include <JuceHeader.h>

#include <chrono>

//-----------------------------------------------------------------------------

class C64u_UDP_Receiver : private juce::Thread
{
public:
	enum streamType : int8_t
	{
		video,
		audio,
	};

	C64u_UDP_Receiver ( const streamType type );
	~C64u_UDP_Receiver () override;

	// Assigns the raw arrays for the two framebuffers. Size is 384 * 272
	void setVideoBuffers ( uint8_t* buffer1, uint8_t* buffer2 );

	// Assigns the raw arrays for the two audio-buffers. Size is 192
	void setAudioBuffers ( float* buffer1, float* buffer2 );

	// Start/Stop receiving data
	[[ nodiscard ]] juce::String start ( const juce::String& address );
	void stop ();

	// UI Callbacks
	std::function<void ( int finishedBufferIndex, bool isNTSC )> onVideoFrame;
	std::function<void ( int finishedBufferIndex )> onAudioChunk;
	std::function<void ( bool receiving )> onStatusChange;

private:
	// juce::Thread
	void run () override;

	// this
	void processIncomingVideo ( const uint8_t* data );
	void processIncomingAudio ( const int16_t* data );

	// Video buffer management
	uint8_t*	videoBuffers[ 2 ] = { nullptr, nullptr };
	int			videoBufferIndex = 0;

	// Audio buffer management
	float*		audioBuffers[ 2 ] = { nullptr, nullptr };

	// Status Flags
	bool		hasTimedOut = true;

	// Timing
	std::chrono::milliseconds				timeoutThresholdMs { std::chrono::milliseconds ( 1000 ) };
	std::chrono::steady_clock::time_point	lastReadTime;

	// Networking
	std::unique_ptr<juce::DatagramSocket>	socket = nullptr;
	juce::String			currentMulticastAddress;

	// Type of stream this receiver is handling (video or audio)
	const streamType		type;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( C64u_UDP_Receiver )
};
//-----------------------------------------------------------------------------