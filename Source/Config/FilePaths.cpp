#include <JuceHeader.h>

#include "Config/Settings.h"
#include "FilePaths.h"

//-----------------------------------------------------------------------------

bool filepaths::isDeveloperMode ()
{
#if ULTRA_DEVELOPMENT
	return true;
#else
	return false;
#endif
}
//-----------------------------------------------------------------------------

bool filepaths::allPathsValid ( const juce::StringArray& arr, const juce::File& root, const juce::File& fallback )
{
	for ( const auto& f : arr )
	{
		auto present = [ &f ] ( const juce::File& base )
		{
			if ( base == juce::File () )
				return false;

			const auto	child = base.getChildFile ( f );

			return f.endsWithChar ( '/' ) ? child.isDirectory () : child.existsAsFile ();
		};

		if ( ! present ( root ) && ! present ( fallback ) )
			return false;
	}

	return true;
}
//-----------------------------------------------------------------------------

bool filepaths::hasDataContent ( const juce::File& folder )
{
	static const juce::StringArray	arr = {
		"CRTEmulation/Shaders/",

		"Data/Games.csv",

		// Every file under Data/UI (app tree + ultra-shared), enumerated at
		// configure time
		#include "ui-manifest.h"
	};

	// The shared entries live in the ultra-shared Data tree, the second naked
	// root datasource falls back to
	#ifdef ULTRA_SHARED_DATA_DIR
		const juce::File	shared ( ULTRA_SHARED_DATA_DIR );
	#else
		const juce::File	shared;
	#endif

	return folder.isDirectory () && allPathsValid ( arr, folder, shared );
}
//-----------------------------------------------------------------------------

static juce::File getUserPath ( const juce::String& folder )
{
	const juce::SharedResourcePointer<Settings>	settings;

	auto	path = settings->get<juce::String> ( "paths/user" );
	if ( path.isEmpty () )
		return {};

	auto	subFolder = juce::File ( path ).getChildFile ( folder );
	subFolder.createDirectory ();

	return subFolder;
}
//-----------------------------------------------------------------------------

juce::File filepaths::getUserThemesPath ()
{
	return getUserPath ( "Themes" );
}
//-----------------------------------------------------------------------------

juce::File filepaths::getUserOverlaysPath ()
{
	return getUserPath ( "Overlays" );
}
//-----------------------------------------------------------------------------

juce::File filepaths::getUserCRTMasksPath ()
{
	return getUserPath ( "CRT Masks" );
}
//-----------------------------------------------------------------------------

juce::File filepaths::getUserCRTPresetsPath ()
{
	return getUserPath ( "CRT Presets" );
}
//-----------------------------------------------------------------------------

juce::String filepaths::markerFor ( const root which )
{
	switch ( which )
	{
		case root::user:	return "$USER$";

		default:			return "$DATA$";
	}
}
//-----------------------------------------------------------------------------
