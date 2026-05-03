#pragma once

#include <JuceHeader.h>

#include "UI/Components/GUI_Label.h"
#include "UI/Components/GUI_SearchBar.h"
#include "UI/Components/GUI_SettingsBox.h"
#include "UI/Components/GUI_SVG_Button.h"

#include "GUI_ListBox.h"

#include "Globals/Settings.h"

//-----------------------------------------------------------------------------

class GUI_Browser final : public juce::Component, private juce::Thread
{
public:
	GUI_Browser ();
	~GUI_Browser () override;

	void refreshBrowserEntries ();

private:
	void run () override;

	juce::SharedResourcePointer<Settings>	settings;

	std::vector<browserEntry>	browserEntries;
	std::vector<browserEntry*>	browserEntryPtrs;
	juce::String	searchString;

	juce::File							lastBrowsedDir;
	std::unique_ptr<juce::FileChooser>	directoryChooser;

	GUI_SettingsBox	background { "background" };
		GUI_Label			curPath { "", 15.0f, 500 };
		GUI_SVG_Button		changePath { "changePath", { "browser/path" } };
		GUI_SearchBar		searchBar;
		GUI_Label			info { "", 16.0f, 600 };
		GUI_ListBox			listbox;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Browser )
};
//-----------------------------------------------------------------------------
