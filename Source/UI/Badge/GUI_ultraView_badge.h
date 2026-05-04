#pragma once

#include <JuceHeader.h>

#include "GUI_ultraView_logo.h"
#include "GUI_Stripes.h"

//-----------------------------------------------------------------------------

class GUI_ultraView_Badge final : public juce::Component
{
public:
	GUI_ultraView_Badge ();

	void paint ( juce::Graphics& g ) override;

private:
	GUI_ultraView_logo	logoCl { "chickenlips", "logos/chickenlips" };
	GUI_ultraView_logo	logoUltraView { "logo", "logos/ultraview" };
	GUI_Stripes			stripes;
	GUI_ultraView_logo	logo64 { "64", "logos/64" };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_ultraView_Badge )
};
//-----------------------------------------------------------------------------
