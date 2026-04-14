#pragma once

#include <JuceHeader.h>

#include "UI/Components/GUI_Label.h"
#include "UI/Components/GUI_Toggle.h"

#include "Globals/Preferences.h"
#include "Globals/Strings.h"

//-----------------------------------------------------------------------------

class GUI_SettingsToggle final : public juce::Component
{
public:
	GUI_SettingsToggle ( const juce::String& setSection, const juce::String& setName );

	// juce::Component
	void resized () override;

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
	GUI_Toggle			toggle;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_SettingsToggle )
};
//-----------------------------------------------------------------------------
