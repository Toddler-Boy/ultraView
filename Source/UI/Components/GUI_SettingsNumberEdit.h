#pragma once

#include <JuceHeader.h>

#include "Globals/Preferences.h"
#include "Globals/Strings.h"

#include "UI/Components/GUI_Label.h"

//-----------------------------------------------------------------------------

class GUI_SettingsNumberEdit final : public juce::Component
{
public:
	GUI_SettingsNumberEdit ( const juce::String& setSection, const juce::String& setName );

	// juce::Component
	void resized () override;
	void lookAndFeelChanged () override;

	// this
	void restorePreference ();

private:
	gin::LayoutSupport	layout { *this };

	juce::SharedResourcePointer<Preferences>	preferences;
	juce::SharedResourcePointer<Strings>		strings;

	juce::String	settingSection;
	juce::String	settingName;

	GUI_DynamicLabel	label;
	GUI_DynamicLabel	help;
	juce::TextEditor	number { "number" };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_SettingsNumberEdit )
};
//-----------------------------------------------------------------------------
