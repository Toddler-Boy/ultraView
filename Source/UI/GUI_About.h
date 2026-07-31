#pragma once

#include <JuceHeader.h>

#include "ultra-shared/UI/Components/GUI_Label.h"
#include "ultra-shared/UI/Components/GUI_SettingsBox.h"
#include "UI/Components/GUI_ScrollTextViewer.h"
#include "ultra-shared/UI/Components/GUI_SVG_Button.h"

#include "ultra-shared/Helpers/MipMap.h"

//-----------------------------------------------------------------------------

class GUI_MipMap : public juce::Component
{
public:
	GUI_MipMap () = default;

	void paint ( juce::Graphics& g ) override
	{
		mipMap.draw ( g, getLocalBounds ().toFloat () );
	}

	MipMap	mipMap;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_MipMap )
};
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
	GUI_SettingsBox			about { "about" };
		GUI_MipMap				icon;
		GUI_Label				title { ProjectInfo::projectName + juce::String ( " " ) + ProjectInfo::versionString, 20.0f, 700 };
		GUI_Label				copyright { u8"Copyright © 2026 Michael Hartmann (Toddler Boy)", 13.0f, 500 };
		juce::HyperlinkButton	link { "https://github.com/Toddler-Boy/ultraView", { "https://github.com/Toddler-Boy/ultraView"  } };

	GUI_ScrollTextViewer	scrollTextViewer;
	GUI_SVG_Button			closeAbout { "close", { "crt/about_close" } };

	gin::LayoutSupport	layout { *this };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_About )
};
//-----------------------------------------------------------------------------
