#pragma once

#include <JuceHeader.h>

//#include "UI/Badge/GUI_ultraSID_badge.h"
#include "UI/CRT/GUI_CRT.h"

//-----------------------------------------------------------------------------

class GUI_Main final : public juce::Component
{
public:
	GUI_Main ( juce::AudioDeviceManager& deviceManager );

	// juce::Component
	void resized () override;

	gin::LayoutSupport	layout { *this };

	// GUI_ultraSID_Badge	badge;
	GUI_CRT		crt;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Main )
};
//-----------------------------------------------------------------------------
