#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class GUI_SettingsBox final : public juce::Component
{
public:
	GUI_SettingsBox ( const juce::String& n = "" );

	// juce::Component
	void paint ( juce::Graphics& g ) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_SettingsBox )
};
//-----------------------------------------------------------------------------
