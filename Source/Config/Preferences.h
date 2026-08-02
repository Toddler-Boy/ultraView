#pragma once

#include <JuceHeader.h>

#include "ultra-shared/Config/SharedPreferences.h"
#include "ultra-shared/Config/YamlFile.h"

//-----------------------------------------------------------------------------

class Preferences final : public YamlFile
{
public:
	Preferences () : YamlFile ( getDefaultValues () )
	{
	}

	void setRoot ( const juce::File& _root )
	{
		if ( _root == juce::File () )
			return;

		Z_DLOG ( "loading preferences" );
		load ( _root.getChildFile ( "preferences.yml" ) );
	}

	struct range { int min = 0, max = 100; };

	// Valid ranges for the free-form number settings, shared by the UI editor
	// (rejects nonsense input) and the consumers (clamp hand-edited yml
	// values). ultraView has no number settings yet, so all keys get the
	// default
	[[ nodiscard ]] static range getRange ( const juce::String& )
	{
		return {};
	}

	[[ nodiscard ]] int getClamped ( const juce::String& key )
	{
		const auto	r = getRange ( key );

		return std::clamp ( get<int> ( key ), r.min, r.max );
	}

private:
	// No app-own settings: the theme is fixed and the screen never sleeps
	// (C64u sessions have no PC input). The CRT-emulation block (overlay, tv,
	// crt, webcam) is shared with ultraSID, keys and values identical by
	// construction
	[[ nodiscard ]] static std::vector<YamlFile::value> getDefaultValues ()
	{
		return sharedpreferences::getDefaultValues ();
	}
};
//-----------------------------------------------------------------------------
