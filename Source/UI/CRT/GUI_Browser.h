#pragma once

#include <JuceHeader.h>

#include "UI/Components/GUI_Label.h"
#include "UI/Components/GUI_SearchBar.h"
#include "UI/Components/GUI_SettingsBox.h"

#include "GUI_ListBox.h"

//-----------------------------------------------------------------------------

class GUI_Browser final : public juce::Component, private juce::Thread
{
public:
	GUI_Browser ();
	~GUI_Browser () override;

private:
	std::string normalizeString ( const std::string& input );

	void run () override;

	GUI_SettingsBox	background { "background" };
		GUI_SearchBar	searchBar;
		GUI_Label		info { "", 16.0f, 600 };
		GUI_ListBox		listbox;

	std::vector<browserEntry>	browserEntries;
	std::vector<browserEntry*>	browserEntryPtrs;

	juce::String	searchString;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Browser )
};
//-----------------------------------------------------------------------------
