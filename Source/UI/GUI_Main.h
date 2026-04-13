#pragma once

#include <JuceHeader.h>

#include "UI/Badge/GUI_ultraSID_badge.h"
#include "UI/Footer/GUI_Footer.h"
#include "UI/Pages/GUI_Pages.h"
#include "UI/SidebarLeft/GUI_SidebarLeft.h"
#include "UI/SidebarRight/GUI_SidebarRight.h"

//-----------------------------------------------------------------------------

class GUI_Main final : public juce::Component
{
public:
	GUI_Main ( juce::AudioDeviceManager& deviceManager );

	// juce::Component
	void resized () override;

	gin::LayoutSupport	layout { *this };

	GUI_ultraSID_Badge	badge;
	GUI_Pages			pages;
	GUI_SidebarLeft		sidebarLeft;
	GUI_SidebarRight	sidebarRight;
	GUI_Footer			footer;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Main )
};
//-----------------------------------------------------------------------------
