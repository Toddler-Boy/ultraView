#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class GUI_Stripes final : public juce::Component
{
public:
	GUI_Stripes ();

	void paint ( juce::Graphics& g ) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Stripes )
};
//-----------------------------------------------------------------------------
