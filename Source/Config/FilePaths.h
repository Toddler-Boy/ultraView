#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

namespace filepaths
{
	// Marked pick-list names record which root a name lives under,
	// e.g. "$USER$/neon" vs "$DATA$/default"
	enum class root { data, user };

	[[ nodiscard ]] juce::String markerFor ( const root which );

	// User content: real folders merged over the pak-backed factory set (user
	// overlays shadow whole folders, user masks shadow single files); themes
	// list as the $USER$ group of the theme selector, presets are plain files
	// read directly
	[[ nodiscard ]] juce::File getUserThemesPath ();
	[[ nodiscard ]] juce::File getUserOverlaysPath ();
	[[ nodiscard ]] juce::File getUserCRTMasksPath ();
	[[ nodiscard ]] juce::File getUserCRTPresetsPath ();
}
//-----------------------------------------------------------------------------
