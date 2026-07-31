#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

namespace filepaths
{
	// Marked pick-list names record which root a name lives under,
	// e.g. "$USER$/neon" vs "$DATA$/default"
	enum class root { data, user };

	[[ nodiscard ]] juce::String markerFor ( const root which );

	// The ULTRASID_DEVELOPER_MODE environment variable (MD5 of its value must
	// match), gating developer features and the naked data-root probe; one
	// switch covers both apps
	[[ nodiscard ]] bool isDeveloperMode ();

	// Every entry exists under root: trailing '/' = directory, otherwise file
	[[ nodiscard ]] bool allPathsValid ( const juce::StringArray& arr, const juce::File& root );

	// Does this folder hold a complete naked ultraView data set? A spot check of
	// the installed structure, not an exhaustive inventory
	[[ nodiscard ]] bool hasDataContent ( const juce::File& folder );

	// User content: real folders merged over the pak-backed factory set (user
	// overlays shadow whole folders, user masks shadow single files); themes
	// list as the $USER$ group of the theme selector
	[[ nodiscard ]] juce::File getUserThemesPath ();
	[[ nodiscard ]] juce::File getUserOverlaysPath ();
	[[ nodiscard ]] juce::File getUserCRTMasksPath ();
}
//-----------------------------------------------------------------------------
