#pragma once

#include <JuceHeader.h>

#include "UI/Components/GUI_ScrollTextViewer.h"
#include "UI/Components/GUI_SVG_Button.h"

//-----------------------------------------------------------------------------

class GUI_About final : public juce::Component
{
public:
	GUI_About ();

	// juce::Component
	void resized () override;

	// this
	void updateColors ();
	void loadContent ();

private:
	GUI_ScrollTextViewer	scrollTextViewer;
	GUI_SVG_Button			closeAbout { "about", { "crt/about_close" } };

	gin::LayoutSupport	layout { *this };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_About )
};
//-----------------------------------------------------------------------------
