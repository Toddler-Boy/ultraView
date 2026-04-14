#pragma once

#include <JuceHeader.h>

#include "Globals/Preferences.h"

//-----------------------------------------------------------------------------

class GUI_XYPad final : public juce::Component, public juce::Timer
{
public:
	GUI_XYPad ( const juce::String& sectionId, const juce::String& compId, const juce::Colour col );

	// juce::Component
	void paint ( juce::Graphics& g ) override;

	void mouseEnter ( const juce::MouseEvent& evt ) override;
	void mouseExit ( const juce::MouseEvent& evt ) override;

	void mouseDown ( const juce::MouseEvent& evt ) override;
	void mouseDrag ( const juce::MouseEvent& evt ) override;

	// juce::Timer
	void timerCallback () override;

	std::function<void ()>	onValueChange;

private:
	const juce::Colour color;

	bool	hover = false;

	void updatePosition ( const juce::MouseEvent& evt );
	void valueChanged ();

	juce::SharedResourcePointer<Preferences>	preferences;

	juce::String	sectionId;
	juce::String	compId;
	std::pair<int, int>	value { 0, 0 };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_XYPad )
};
//-----------------------------------------------------------------------------
