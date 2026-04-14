#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class GUI_Toggle final : public juce::ToggleButton, public juce::Timer
{
public:
	GUI_Toggle ( const juce::String& name = {} );

	// juce::Component
	void enablementChanged () override;

	// juce::Button
	void paintButton ( juce::Graphics& g, bool isHover, bool isDown ) override;
	void buttonStateChanged () override;

	// juce::Timer
	void timerCallback () override;

private:
	float	animPosition = 0.0f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Toggle )
};
//-----------------------------------------------------------------------------
