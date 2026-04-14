#pragma once

#include <JuceHeader.h>

#include "GUI_Label.h"
#include "GUI_Slider.h"

#include "Globals/Preferences.h"
#include "Globals/Strings.h"

//-----------------------------------------------------------------------------

class GUI_CRTSliderLabel final : public juce::Component
{
public:
	GUI_CRTSliderLabel ( const juce::String& setSection, const juce::String& setName, const bool bidirectional = false );

	// juce::Component
	void resized () override;

	std::function<void()>	onValueChange;

private:
	gin::LayoutSupport	layout { *this };

	juce::SharedResourcePointer<Preferences>	preferences;
	juce::SharedResourcePointer<Strings>		strings;

	juce::String	settingSection;
	juce::String	settingName;

	GUI_DynamicLabel	label;
	GUI_Slider			slider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_CRTSliderLabel )
};
//-----------------------------------------------------------------------------
