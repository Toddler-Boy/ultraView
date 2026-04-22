#pragma once

#include <JuceHeader.h>

#include "UI/Components/GUI_ListBox.h"
#include "UI/Components/GUI_SettingsBox.h"

//-----------------------------------------------------------------------------

class GUI_Browser final : public juce::Component, private juce::Thread
{
public:
	GUI_Browser ();
	~GUI_Browser () override;

private:
	void run () override;

	GUI_SettingsBox	background { "background" };
		GUI_ListBox		listbox;

	std::vector<browserEntry>	browserEntries;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Browser )
};
//-----------------------------------------------------------------------------
