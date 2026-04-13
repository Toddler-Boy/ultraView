#include "GUI_ultraSID.h"

//-----------------------------------------------------------------------------

constexpr auto	internalSamplerate = 44100;
constexpr auto	streamSamplerate = 48000;

void GUI_ultraSID::disableAudio ()
{
	if ( muted++ )
		return;

	curOutVol.set ( 0.0f );

	auto	maxTries = 5;
	while ( maxTries-- && ! curOutVol.restingAtZero () )
		juce::Thread::sleep ( 5 );

	inAudio.enter ();
	dspEffects.clearBuffers ();
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::enableAudio ()
{
	if ( --muted )
		return;

	curOutVol.set ( 1.0f );

	inAudio.exit ();
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::prepareToPlay ( int samplesPerBlockExpected, double sampleRate_ )
{
	// The sample rate that the audio device is running at.
	// This is used for resampling from the internal samplerate of 44.1kHz to the output samplerate of the audio device.
	sampleRate = int ( sampleRate_ );

	// Tell SID-player to always output at 44.1kHz
	player.setSamplerate ( internalSamplerate );

	// FIFO buffer (for resampling from internal samplerate to output samplerate)
	sidBuffer.setSize ( 2, samplesPerBlockExpected );
	resamplingFifo.setSize ( samplesPerBlockExpected, 2, sampleRate );
	resamplingFifo.setResamplingRatio ( internalSamplerate, double ( sampleRate ) );

	// Stream FIFI buffer (for receving C64u audio-stream)
	const auto	streamRatio = streamSamplerate / sampleRate_;
	const auto	streamBufferSize = int ( std::ceil ( samplesPerBlockExpected * streamRatio ) );

	streamBuffer.setSize ( 2, samplesPerBlockExpected );
	streamFifo.setSize ( 2, streamBufferSize * 2 );
	streamResamplingFifo.setResamplingRatio ( streamSamplerate, sampleRate_ );

	// Get output latency from audio device and set it to player for accurate display timing
	auto	audioDevice = deviceManager.getCurrentAudioDevice ();
	player.setOutputLatency ( audioDevice ? audioDevice->getOutputLatencyInSamples () : 0 );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::releaseResources ()
{
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::getNextAudioBlock ( const juce::AudioSourceChannelInfo& bufferToFill )
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
	#if 1
	{
		auto renderAudio = [ this ] ( juce::AudioBuffer<float>& buffer )
		{
			auto		buffers = buffer.getArrayOfWritePointers ();
			const auto	numSamples = buffer.getNumSamples ();

			if ( player.play ( buffers, numSamples ) )
			{
				dspEffects.process ( buffers, numSamples );
				mainScreen.sidebarRight.fftLeft.pushAudio ( buffers[ 0 ], nullptr, numSamples );
				mainScreen.sidebarRight.fftRight.pushAudio ( nullptr, buffers[ 1 ], numSamples );
				dspEffects.applyGlain ( buffers, numSamples );
			}
			else
			{
				buffer.clear ();
			}
		};

		if ( sampleRate == internalSamplerate )
		{
			// Render into output directly
			renderAudio ( *bufferToFill.buffer );
		}
		else
		{
			const auto	numSamples = bufferToFill.buffer->getNumSamples ();

			while ( resamplingFifo.samplesReady () < numSamples )
			{
				renderAudio ( sidBuffer );
				resamplingFifo.pushAudioBuffer ( sidBuffer );
			}

			resamplingFifo.popAudioBuffer ( *bufferToFill.buffer );
		}
	}
	#else
	{
		if ( sampleRate == streamSamplerate )
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
			}

			streamResamplingFifo.popAudioBuffer ( *bufferToFill.buffer );

			const auto	mag = bufferToFill.buffer->getMagnitude ( 0, numSamples );
			Z_INFO ( "Stream audio magnitude: " << mag );
		}
	}
	#endif

	//
	// Fade buffer in/out to avoid clicking between songs
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

void GUI_ultraSID::updateFX ()
{
	auto setToggle = [ this ] ( const SIDEffects::FXParameter param, const juce::String& name )
	{
		dspEffects.setFXParameter ( param, preferences->get<bool> ( "fx", name) ? 1.0f : 0.0f );
	};

	auto setValue = [ this ] ( const SIDEffects::FXParameter param, const juce::String& name )
	{
		dspEffects.setFXParameter ( param, preferences->get<int> ( "fx", name ) * 0.01f );
	};

	// Stereo processing
	setToggle ( SIDEffects::FXParameter::stereo_processing, "stereo-processing" );

	// Splitter
	setValue ( SIDEffects::FXParameter::magic_splitter_freq, "splitter-freq" );

	// Wide-mono
	setToggle ( SIDEffects::FXParameter::magic_wideMono_enabled, "wide-mono" );
	setValue ( SIDEffects::FXParameter::magic_wideMono_width, "wide-mono-width" );

	// Delay
	setToggle ( SIDEffects::FXParameter::magic_delay_enabled, "delay" );
	setValue ( SIDEffects::FXParameter::magic_delay_wet, "delay-wet" );
	setValue ( SIDEffects::FXParameter::magic_delay_feedback, "delay-feedback" );

	// Reverb
	setToggle ( SIDEffects::FXParameter::magic_reverb_enabled, "reverb" );
	setValue ( SIDEffects::FXParameter::magic_reverb_wet, "reverb-wet" );

	// Mid/Side
	setToggle ( SIDEffects::FXParameter::magic_midSide_enabled, "mid-side" );
	setValue ( SIDEffects::FXParameter::magic_midSide_width, "mid-side-width" );

	// Loudness
	setToggle ( SIDEffects::FXParameter::magic_loudness_enabled, "loudness" );

	// Noise
	setToggle ( SIDEffects::FXParameter::magic_noise_enabled, "noise" );
	setValue ( SIDEffects::FXParameter::magic_noise_volume, "noise-volume" );
	setValue ( SIDEffects::FXParameter::magic_noise_color, "noise-color" );
}
//-----------------------------------------------------------------------------
