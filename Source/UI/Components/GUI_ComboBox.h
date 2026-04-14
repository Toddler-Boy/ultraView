#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class GUI_ComboBox final : public juce::ComboBox
{
public:
	explicit GUI_ComboBox ( const juce::String& componentName = {} )
		: juce::ComboBox ( componentName )	{}

	juce::MouseCursor getMouseCursor () override {	return juce::MouseCursor::PointingHandCursor;	}

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_ComboBox )
};
//-----------------------------------------------------------------------------
