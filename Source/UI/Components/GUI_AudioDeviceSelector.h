#include <JuceHeader.h>

#include "GUI_ComboBox.h"

//-----------------------------------------------------------------------------

class GUI_AudioDeviceSelector : public juce::Component, private juce::ChangeListener
{
public:
	GUI_AudioDeviceSelector ();
	~GUI_AudioDeviceSelector () override;

	// juce::Component
	void resized () override;

	// this
	void setAudioDeviceManager ( juce::AudioDeviceManager& m );
	void setCurrentOutputDevice ( const juce::String& deviceName );
	juce::String getCurrentOutputDevice () const { return currentOutputDevice; }

private:
	juce::AudioIODeviceType*	type = nullptr;
	juce::AudioDeviceManager*	manager = nullptr;

	juce::String				defaultOutputDevice;
	juce::String				currentOutputDevice;

	GUI_ComboBox				outputDeviceDropDown { "device" };

	juce::ScopedMessageBox		messageBox;

	// juce::ChangeListener
	void changeListenerCallback ( juce::ChangeBroadcaster* /*source*/ ) override;

	// this
	juce::StringArray getOutputNames ();

	void updateConfig ( bool updateOutputDevice, bool updateSampleRate, bool updateBufferSize );
	void updateAllControls ();
	void updateSelectedOutput ();
	void updateOutputsComboBox ();
	void updateSampleRateComboBox ( juce::AudioIODevice* currentDevice );
	void updateBufferSizeComboBox ( juce::AudioIODevice* currentDevice );

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_AudioDeviceSelector )
};
//-----------------------------------------------------------------------------
