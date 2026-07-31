#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

// All factory data comes through here. The backing store is either Data.pak
// (the only layout users get) or the naked repo Data folder (developer mode),
// resolved once on first use; consumers only ever see data-relative paths.
//
// The lime shader stack reads its CRT data through lime::content, installed
// in pak mode to route the nominal files under getCRTRoot () into the pak.

namespace datasource
{
	// A Data.pak next to the exe beats everything, including developer mode:
	// dropping one beside a dev build tests the release path
	[[ nodiscard ]] bool isPak ();

	// Pak: the central directory parses and holds the CRT shaders.
	// Naked: the folder passes the content spot-check
	[[ nodiscard ]] bool isValid ();

	// The pak file or naked folder in use, for the startup log
	[[ nodiscard ]] juce::String describe ();

	[[ nodiscard ]] bool exists ( const juce::String& path );

	[[ nodiscard ]] juce::String loadText ( const juce::String& path );
	[[ nodiscard ]] juce::MemoryBlock loadData ( const juce::String& path );
	[[ nodiscard ]] juce::Image loadImage ( const juce::String& path );
	[[ nodiscard ]] std::unique_ptr<juce::InputStream> openStream ( const juce::String& path );

	// Paths relative to prefix; non-recursive stops at the next '/'
	[[ nodiscard ]] juce::StringArray listFiles ( const juce::String& prefix, const bool recursive = false, const juce::String& wildcard = {} );
	[[ nodiscard ]] juce::StringArray listFolders ( const juce::String& prefix );

	// The CRT emulation folder: a real directory in naked mode, the nominal
	// root the lime content hook maps onto pak entries in pak mode
	[[ nodiscard ]] juce::File getCRTRoot ();

	// The real file behind a path, for developer tooling that writes factory
	// data and for the factory folder watcher. In pak mode there is no such
	// file: Z_ERR and an invalid File
	[[ nodiscard ]] juce::File getDevFile ( const juce::String& path = {} );

	// The user's explicit $USER$ picks from the CRT drop-downs: when factory
	// and user content share a name, the lime content loader serves the user
	// variant only for these; unique user names always resolve to the user
	// folder. Empty = the factory variant is selected
	void setActiveUserOverlay ( const juce::String& name );
	void setActiveUserCRTMask ( const juce::String& name );	// stem, no ".png"
}
//-----------------------------------------------------------------------------
