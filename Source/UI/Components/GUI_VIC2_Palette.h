#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class GUI_VIC2_Palette final : public juce::Component
{
public:
	GUI_VIC2_Palette ();

	// juce::Component
	void paint ( juce::Graphics& g ) override;

	// this
	void setSettings ( const int standard, const float brightness, const float contrast, const float saturation, const bool earlyLuma );

private:
	std::array<juce::Colour, 16>	palette;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_VIC2_Palette )
};
//-----------------------------------------------------------------------------
