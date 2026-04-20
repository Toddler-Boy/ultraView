#pragma once

#include <JuceHeader.h>

#include "Globals/Icons.h"
#include "Globals/Strings.h"

#include <chrono>

//-----------------------------------------------------------------------------

class GUI_IconButton final : public juce::Button, private juce::Timer
{
public:
	GUI_IconButton ( const juce::String& name, const int colId, const int timeoutMs = 0 );

	// juce::Component
	void enablementChanged () override;

	void mouseDown ( const juce::MouseEvent& evt ) override;
	void mouseUp ( const juce::MouseEvent& evt ) override;

	juce::MouseCursor getMouseCursor () override { return juce::MouseCursor::PointingHandCursor; }

	// juce::Button

	void paintButton ( juce::Graphics& g, bool hover, bool down ) override;

private:
	juce::SharedResourcePointer<Icons>		icons;
	juce::SharedResourcePointer<Strings>	strings;

	juce::String	icon;
	int				colId;
	int				timeoutMs;
	float			progress = 0.0f;

	std::chrono::steady_clock::time_point	startTime;
	std::chrono::steady_clock::time_point	targetTime;

	void timerCallback () override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_IconButton )
};
//-----------------------------------------------------------------------------
