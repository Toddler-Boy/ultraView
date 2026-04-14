#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class GUI_Slider : public juce::Slider
{
public:
	GUI_Slider ( SliderStyle style, TextEntryBoxPosition textBoxPosition )
		: juce::Slider ( style, textBoxPosition ) {}

	void mouseEnter ( const juce::MouseEvent& evt ) override
	{
		juce::Slider::mouseEnter ( evt );
		setMouseCursor ( juce::MouseCursor::PointingHandCursor );
	}

	void mouseExit ( const juce::MouseEvent& evt ) override
	{
		juce::Slider::mouseExit ( evt );
		setMouseCursor ( juce::MouseCursor::NormalCursor );
	}

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Slider )
};
//-----------------------------------------------------------------------------
