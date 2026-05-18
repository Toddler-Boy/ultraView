#pragma once

#include <JuceHeader.h>

#include "UI/Components/BarelyML.h"
#include "UI/Components/GUI_SVG_Button.h"

//-----------------------------------------------------------------------------

class GUI_About final : public juce::Component, public BarelyMLDisplay::URLHandler
{
public:
	GUI_About ();

	// juce::Component
	void resized () override;

	// BarelyMLDisplay::URLHandler
	bool handleURL ( juce::String url ) override;

	// this
	void updateColors ();
	void loadContent ();

private:
	BarelyMLDisplay	mdDisplay;
	GUI_SVG_Button	closeAbout { "about", { "crt/about_close" } };

	gin::LayoutSupport	layout { *this };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_About )
};
//-----------------------------------------------------------------------------
