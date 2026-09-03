#pragma once

#include <JuceHeader.h>

#include "GUI_ultraView_logo.h"
#include "ultra-shared/UI/Components/GUI_VersionPill.h"

//-----------------------------------------------------------------------------

class GUI_ultraView_Badge final : public juce::Component
{
public:
	GUI_ultraView_Badge ();

	GUI_VersionPill		version;

private:
	GUI_ultraView_logo	logoUltraView { "logo", "logos/ultraview" };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_ultraView_Badge )
};
//-----------------------------------------------------------------------------
