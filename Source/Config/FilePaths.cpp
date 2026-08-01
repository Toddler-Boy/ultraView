#include <JuceHeader.h>

#include "Config/Settings.h"
#include "FilePaths.h"

//-----------------------------------------------------------------------------

bool filepaths::isDeveloperMode ()
{
	static const bool enabled = []
	{
		const auto value = juce::SystemStats::getEnvironmentVariable ( "ULTRASID_DEVELOPER_MODE", "" );

		return juce::MD5 ( value.toUTF8 () ).toHexString () == "085663f5bcbd242d43e8eaad5dcbfe80";
	} ();

	return enabled;
}
//-----------------------------------------------------------------------------

bool filepaths::allPathsValid ( const juce::StringArray& arr, const juce::File& root )
{
	for ( const auto& f : arr )
	{
		if ( f.endsWithChar ( '/' ) )
		{
			if ( ! root.getChildFile ( f ).isDirectory () )
				return false;
		}
		else
		{
			if ( ! root.getChildFile ( f ).existsAsFile () )
				return false;
		}
	}

	return true;
}
//-----------------------------------------------------------------------------

bool filepaths::hasDataContent ( const juce::File& folder )
{
	static const juce::StringArray	arr = {
		"CRTEmulation/Shaders/",

		"Data/Games.csv",

		// Every file under Data/UI, enumerated at configure time
		#include "ui-manifest.h"
	};

	return folder.isDirectory () && allPathsValid ( arr, folder );
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
