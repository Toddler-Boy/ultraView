#include <algorithm>

#include "GUI_ultraView.h"

//-----------------------------------------------------------------------------

void GUI_ultraView::initAudio ()
{
	// Open the device ourselves, so sample-rate and block size are respected
	{
		juce::AudioDeviceManager::AudioDeviceSetup	preferred;
		preferred.sampleRate = internalSamplerate;
		preferred.bufferSize = internalSamplerate / 100;	// 10ms

		if ( const auto error = deviceManager.initialise ( 0, 2, nullptr, true, {}, &preferred ); error.isNotEmpty () )
			Z_WARN ( "Audio device init: " << error );
	}

	// Attach audio callback
	setAudioChannels ( 0, 2 );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::disableAudio ()
{
	if ( muted++ )
		return;

	curOutVol.set ( 0.0f );

	auto	maxTries = 5;
	while ( maxTries-- && ! curOutVol.restingAtZero () )
		juce::Thread::sleep ( 5 );

	inAudio.enter ();
}
//-----------------------------------------------------------------------------

void GUI_ultraView::enableAudio ()
{
	if ( --muted )
		return;

	curOutVol.set ( 1.0f );

	inAudio.exit ();
}
//-----------------------------------------------------------------------------

void GUI_ultraView::prepareToPlay ( int samplesPerBlockExpected, double sampleRate_ )
{
	// The sample rate that the audio device is running at.
	// This is used for resampling from the internal samplerate of 48kHz to the output samplerate of the audio device.
	sampleRate = int ( sampleRate_ );

	Z_INFO ( "Audio device running at " + juce::String ( sampleRate ) + " Hz with " + juce::String ( samplesPerBlockExpected ) + "-sample blocks" );

	// FIFO buffer (for resampling from internal samplerate to output samplerate)
	resamplingFifo.setSize ( samplesPerBlockExpected, 2, sampleRate );
	resamplingFifo.setResamplingRatio ( internalSamplerate, double ( sampleRate ) );

	// Stream FIFI buffer (for receving C64u audio-stream)
	const auto	streamRatio = internalSamplerate / sampleRate_;
	const auto	streamBufferSize = int ( std::ceil ( samplesPerBlockExpected * streamRatio ) );

	streamBuffer.setSize ( 2, int ( streamBufferSize * 1.5 ) );

	// The receiver only pushes whole packets, so tiny device blocks must not
	// shrink the FIFO below packet granularity
	streamFifo.setSize ( 2, std::max ( streamBufferSize * 4, c64uBuffer.getNumSamples () * 4 ) );
	streamResamplingFifo.setResamplingRatio ( internalSamplerate, sampleRate_ );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::releaseResources ()
{
}
//-----------------------------------------------------------------------------

void GUI_ultraView::getNextAudioBlock ( const juce::AudioSourceChannelInfo& bufferToFill )
{
	if ( bufferToFill.buffer == nullptr )
		return;

	//
	// Just clear buffer if we are blocked
	//
	if ( ! inAudio.tryEnter () )
	{
		bufferToFill.clearActiveBufferRegion ();
		return;
	}

	//
	// Fill output buffer
	//
	{
		if ( sampleRate == internalSamplerate )
		{
			streamFifo.read ( *bufferToFill.buffer );
		}
		else
		{
			const auto	numSamples = bufferToFill.buffer->getNumSamples ();
			while ( streamResamplingFifo.samplesReady () < numSamples )
			{
				if ( streamFifo.getNumReady () < streamBuffer.getNumSamples () )
				{
					bufferToFill.clearActiveBufferRegion ();
					inAudio.exit ();
					return;
				}

				streamFifo.read ( streamBuffer );
				streamResamplingFifo.pushAudioBuffer ( streamBuffer );
			}

			streamResamplingFifo.popAudioBuffer ( *bufferToFill.buffer );

			//const auto	mag = bufferToFill.buffer->getMagnitude ( 0, numSamples );
			//Z_INFO ( "Stream audio magnitude: " << mag );
		}
	}

	//
	// Fade buffer in/out to avoid clicking when muting/unmuting
	//
	auto isMuting = [ & ]	{	return muted && ! curOutVol.restingAtZero ();	};
	auto isUnmuting = [ & ] {	return ! muted && ! curOutVol.restingAtOne ();	};

	if ( isMuting () || isUnmuting () )
	{
		const auto	numSamples = bufferToFill.buffer->getNumSamples ();
		const auto	buffers = bufferToFill.buffer->getArrayOfWritePointers ();

		for ( auto i = 0; i < numSamples; ++i )
		{
			const auto	vol = curOutVol.getAndStep ();
			buffers[ 0 ][ i ] *= vol;
			buffers[ 1 ][ i ] *= vol;
		}
	}

	inAudio.exit ();
}
//-----------------------------------------------------------------------------
