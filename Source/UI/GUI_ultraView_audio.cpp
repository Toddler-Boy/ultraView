#include "GUI_ultraView.h"

//-----------------------------------------------------------------------------

constexpr auto	internalSamplerate = 48000;

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

	// FIFO buffer (for resampling from internal samplerate to output samplerate)
	resamplingFifo.setSize ( samplesPerBlockExpected, 2, sampleRate );
	resamplingFifo.setResamplingRatio ( internalSamplerate, double ( sampleRate ) );

	// Stream FIFI buffer (for receving C64u audio-stream)
	const auto	streamRatio = internalSamplerate / sampleRate_;
	const auto	streamBufferSize = int ( std::ceil ( samplesPerBlockExpected * streamRatio ) );

	streamBuffer.setSize ( 2, streamBufferSize );
	streamFifo.setSize ( 2, streamBufferSize * 2 );
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
				if ( streamFifo.getNumReady () >= streamBuffer.getNumSamples () )
				{
					streamFifo.read ( streamBuffer );
					streamResamplingFifo.pushAudioBuffer ( streamBuffer );
				}
				else
				{
					bufferToFill.clearActiveBufferRegion ();
					inAudio.exit ();
					return;
				}
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
