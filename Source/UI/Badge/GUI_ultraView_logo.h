#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class GUI_ultraView_logo final : public juce::Component
{
public:
	GUI_ultraView_logo ( const juce::String& name, const juce::String& resource );

	void paint ( juce::Graphics& g ) override;

private:
	juce::Path	path;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_ultraView_logo )
};
//-----------------------------------------------------------------------------
