#pragma once

#include <JuceHeader.h>

#include "UI/Components/GUI_Slider.h"
#include "UI/Components/GUI_SVG_Button.h"

#include "Globals/Preferences.h"
#include "Globals/Strings.h"

//-----------------------------------------------------------------------------

class GUI_CRTSliderIcon final : public juce::Component
{
public:
	GUI_CRTSliderIcon ( const juce::String& setSection, const juce::String& setName, const bool bidirectional = false );

	// juce::Component
	void resized () override;

	std::function<void ()>	onValueChange;

private:
	gin::LayoutSupport	layout { *this };

	juce::SharedResourcePointer<Preferences>	preferences;
	juce::SharedResourcePointer<Strings>		strings;

	juce::String	settingSection;
	juce::String	settingName;

	GUI_SVG_Button	icon;
	GUI_Slider		slider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_CRTSliderIcon )
};
//-----------------------------------------------------------------------------
