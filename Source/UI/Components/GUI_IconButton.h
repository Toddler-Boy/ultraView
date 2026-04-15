#pragma once

#include <JuceHeader.h>

#include "Globals/Icons.h"
#include "Globals/Strings.h"

//-----------------------------------------------------------------------------

class GUI_IconButton final : public juce::Button
{
public:
	GUI_IconButton ( const juce::String& name, const int colId );

	// juce::Component
	void enablementChanged () override;
	juce::MouseCursor getMouseCursor () override { return juce::MouseCursor::PointingHandCursor; }

	// juce::Button
	void paintButton ( juce::Graphics& g, bool hover, bool down ) override;

private:
	juce::SharedResourcePointer<Icons>		icons;
	juce::SharedResourcePointer<Strings>	strings;

	juce::String	icon;
	int				colId;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_IconButton )
};
//-----------------------------------------------------------------------------
