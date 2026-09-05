#pragma once

#include <JuceHeader.h>

#include "UI/Badge/GUI_ultraView_badge.h"

#include "ultra-shared/UI/Components/GUI_Label.h"
#include "ultra-shared/UI/Components/GUI_SearchBar.h"
#include "ultra-shared/UI/Components/GUI_SettingsBox.h"
#include "ultra-shared/UI/Components/GUI_SVG_Button.h"
#include "ultra-shared/UI/Components/GUI_TextEditorEx.h"

#include "GUI_ListBox.h"

#include "Config/Settings.h"

//-----------------------------------------------------------------------------

class GUI_Browser final : public juce::Component, private juce::Thread
{
public:
	GUI_Browser ();
	~GUI_Browser () override;

	void refreshBrowserEntries ();

	// Keyboard into the search field, its text selected
	void focusSearch ();

	[[ nodiscard ]] GUI_VersionPill& versionPill ()	{	return badge.version;	}

private:
	void run () override;

	juce::SharedResourcePointer<Settings>	settings;
	juce::SharedResourcePointer<Strings>	strings;

	std::vector<browserEntry>	browserEntries;
	std::vector<browserEntry*>	browserEntryPtrs;
	juce::String	searchString;

	juce::File							lastBrowsedDir;
	std::unique_ptr<juce::FileChooser>	directoryChooser;

	GUI_ultraView_Badge	badge;

	// The C64u's optional http password; the eye's second stage reveals the text
	GUI_SettingsBox	passwordBox { "password" };
		GUI_TextEditorEx	passwordField { "password-field", 0x2022 };
		GUI_SVG_Button		passwordEye { "password-eye", { "browser/password_hide", "browser/password_show" } };

	void applyPassword ();

	// juce::Component, re-resolves the placeholder color from the theme
	void lookAndFeelChanged () override;

	GUI_SettingsBox	background { "background" };
		GUI_Label			curPath { "", 15.0f, 500 };
		GUI_SVG_Button		changePath { "changePath", { "browser/path" } };
		GUI_SearchBar		searchBar;
		GUI_Label			info { "", 16.0f, 600 };
		GUI_ListBox			listbox;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Browser )
};
//-----------------------------------------------------------------------------
