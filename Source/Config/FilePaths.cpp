#include <JuceHeader.h>

#include "Config/Settings.h"
#include "FilePaths.h"

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
