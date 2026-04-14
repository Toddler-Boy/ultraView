#include "GUI_AudioDeviceSelector.h"

//-----------------------------------------------------------------------------

GUI_AudioDeviceSelector::GUI_AudioDeviceSelector ()
	: juce::Component ( "output" )
{
	// Output device drop-down
	outputDeviceDropDown.onChange = [ this ] { updateConfig ( true, false, false ); };

	addAndMakeVisible ( outputDeviceDropDown );
}
//-----------------------------------------------------------------------------

GUI_AudioDeviceSelector::~GUI_AudioDeviceSelector ()
{
	if ( manager )
		manager->removeChangeListener ( this );
}
//-----------------------------------------------------------------------------

void GUI_AudioDeviceSelector::resized ()
{
	outputDeviceDropDown.setBounds ( getLocalBounds () );
}
//-----------------------------------------------------------------------------

void GUI_AudioDeviceSelector::setAudioDeviceManager ( juce::AudioDeviceManager& m )
{
	if ( manager )
		manager->removeChangeListener ( this );

	manager = &m;
	type = m.getAvailableDeviceTypes ()[ 0 ];

	type->scanForDevices ();

	defaultOutputDevice = manager->getAudioDeviceSetup ().outputDeviceName;

	manager->addChangeListener ( this );

	updateAllControls ();
}
//-----------------------------------------------------------------------------

void GUI_AudioDeviceSelector::setCurrentOutputDevice ( const juce::String& deviceName )
{
	const auto	devs = getOutputNames ();
	const auto	index = std::max ( 0, devs.indexOf ( deviceName ) );

	currentOutputDevice = devs[ index ];
	outputDeviceDropDown.setSelectedId ( index + 1, juce::sendNotification );
}
//-----------------------------------------------------------------------------

juce::StringArray GUI_AudioDeviceSelector::getOutputNames ()
{
	auto	devs = type->getDeviceNames ( false );

	devs.sortNatural ();
	devs.insert ( 0, "System default" );

	return devs;
}
//-----------------------------------------------------------------------------

void GUI_AudioDeviceSelector::updateConfig ( bool updateOutputDevice, bool updateSampleRate, bool updateBufferSize )
{
	auto	config = manager->getAudioDeviceSetup ();

	juce::String	error;

	if ( updateOutputDevice )
	{
		currentOutputDevice = outputDeviceDropDown.getText ();
		config.outputDeviceName = outputDeviceDropDown.getSelectedId () <= 1 ? defaultOutputDevice : currentOutputDevice;

		config.useDefaultOutputChannels = true;

		error = manager->setAudioDeviceSetup ( config, true );

		updateSelectedOutput ();
	}
/*	else if ( updateSampleRate )
	{
		if ( sampleRateDropDown->getSelectedId () > 0 )
		{
			config.sampleRate = sampleRateDropDown->getSelectedId ();
			error = manager->setAudioDeviceSetup ( config, true );
		}
	}
	else if ( updateBufferSize )
	{
		if ( bufferSizeDropDown->getSelectedId () > 0 )
		{
			config.bufferSize = bufferSizeDropDown->getSelectedId ();
			error = manager->setAudioDeviceSetup ( config, true );
		}
	}*/

	if ( error.isNotEmpty () )
		messageBox = juce::AlertWindow::showScopedAsync ( juce::MessageBoxOptions ().withIconType ( juce::MessageBoxIconType::WarningIcon )
																	  .withTitle ( "Error when trying to open audio device!" )
																	  .withMessage ( error )
																	  .withButton ( "OK" ),
												   nullptr );
}
//-----------------------------------------------------------------------------

void GUI_AudioDeviceSelector::updateAllControls ()
{
	updateOutputsComboBox ();

	if ( auto* currentDevice = manager->getCurrentAudioDevice () )
	{
		updateSampleRateComboBox ( currentDevice );
		updateBufferSizeComboBox ( currentDevice );
	}
	else
	{
		jassert ( manager && ! manager->getCurrentAudioDevice () ); // not the correct device type!

/*		sampleRateDropDown.reset ();
		bufferSizeDropDown.reset ();*/

		outputDeviceDropDown.setSelectedId ( 1, juce::dontSendNotification );
	}

	sendLookAndFeelChange ();
}
//-----------------------------------------------------------------------------

void GUI_AudioDeviceSelector::changeListenerCallback ( juce::ChangeBroadcaster* /*source*/ )
{
	updateAllControls ();
}
//-----------------------------------------------------------------------------

void GUI_AudioDeviceSelector::updateSelectedOutput ()
{
	const auto	devs = getOutputNames ();
	const auto	index = std::max ( 0, devs.indexOf ( currentOutputDevice ) );

	outputDeviceDropDown.setSelectedId ( index + 1, juce::dontSendNotification );
}
//-----------------------------------------------------------------------------

void GUI_AudioDeviceSelector::updateOutputsComboBox ()
{
	outputDeviceDropDown.clear ( juce::dontSendNotification );

	const auto	devs = getOutputNames ();

	for ( auto i = 0; i < devs.size (); ++i )
		outputDeviceDropDown.addItem ( devs[ i ], i + 1 );

	outputDeviceDropDown.setSelectedId ( -1, juce::dontSendNotification );

	updateSelectedOutput ();
}
//-----------------------------------------------------------------------------

void GUI_AudioDeviceSelector::updateSampleRateComboBox ( juce::AudioIODevice* currentDevice )
{
/*	if ( ! sampleRateDropDown )
	{
		sampleRateDropDown = std::make_unique<GUI_ComboBox> ( "sample-rate" );
		addAndMakeVisible ( sampleRateDropDown.get () );
	}
	else
	{
		sampleRateDropDown->clear ();
		sampleRateDropDown->onChange = nullptr;
	}

	const auto getFrequencyString = [] ( int rate ) { return juce::String ( rate ) + " Hz"; };

	for ( auto rate : currentDevice->getAvailableSampleRates () )
	{
		const auto intRate = juce::roundToInt ( rate );
		sampleRateDropDown->addItem ( getFrequencyString ( intRate ), intRate );
	}

	const auto intRate = juce::roundToInt ( currentDevice->getCurrentSampleRate () );
	sampleRateDropDown->setText ( getFrequencyString ( intRate ), juce::dontSendNotification );

	sampleRateDropDown->onChange = [ this ] { updateConfig ( false, true, false ); };*/
}
//-----------------------------------------------------------------------------

void GUI_AudioDeviceSelector::updateBufferSizeComboBox ( juce::AudioIODevice* currentDevice )
{
/*	if ( ! bufferSizeDropDown )
	{
		bufferSizeDropDown = std::make_unique<GUI_ComboBox> ( "buffer-size" );
		addAndMakeVisible ( bufferSizeDropDown.get () );
	}
	else
	{
		bufferSizeDropDown->clear ();
		bufferSizeDropDown->onChange = nullptr;
	}

	auto	currentRate = currentDevice->getCurrentSampleRate ();

	if ( juce::exactlyEqual ( currentRate, 0.0 ) )
		currentRate = 44100.0;

	auto getBufferSizeText = [] ( int bs, double currentRate )
	{
		return juce::String ( bs ) + " samples (" + juce::String ( bs * 1000.0 / currentRate, 1 ) + " ms)";
	};

	for ( auto bs : currentDevice->getAvailableBufferSizes () )
		bufferSizeDropDown->addItem ( getBufferSizeText ( bs, currentRate ), bs );

	const auto bufferSizeSamples = currentDevice->getCurrentBufferSizeSamples ();
	bufferSizeDropDown->setText ( getBufferSizeText ( bufferSizeSamples, currentRate ), juce::dontSendNotification );

	bufferSizeDropDown->onChange = [ this ]
	{
		updateConfig ( false, false, true );
	};*/
}
//-----------------------------------------------------------------------------
