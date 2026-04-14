#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class GUI_Disabler final : public juce::Component
{
public:
	GUI_Disabler ();

// 	void paint ( juce::Graphics& g ) override
// 	{
// 		g.setColour ( juce::Colours::deeppink );
// 		g.drawRect ( getLocalBounds () );
// 	}

	void enablementChanged () override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Disabler )
};
//-----------------------------------------------------------------------------
