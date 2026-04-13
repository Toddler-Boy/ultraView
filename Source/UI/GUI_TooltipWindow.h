#pragma once

#include <JuceHeader.h>

// Subclass as a workaround for JUCE's TooltipWindow not being able to return the correct desktop scale factor
class GUI_TooltipWindow : public juce::TooltipWindow
{
public:
	float getDesktopScaleFactor () const override	{	return juce::Component::getDesktopScaleFactor ();	}
};
