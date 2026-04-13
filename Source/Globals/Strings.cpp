#include "Strings.h"
#include "constants.h"

//-----------------------------------------------------------------------------

Strings::Strings ()
{
	root = paths::getDataRoot ( "UI/strings" );
	load ();
}
//-----------------------------------------------------------------------------

void Strings::setLanguage ( const juce::String& _language )
{
	if ( language == _language )
		return;

	language = _language;
	load ();
}
//-----------------------------------------------------------------------------

void Strings::load ()
{
	file = root.getChildFile ( language + ".yml" );

	yamlFile::load ();
}
//-----------------------------------------------------------------------------

const juce::String& Strings::get ( const juce::String& name )
{
	if ( auto it = result.find ( name ); it != result.end () )
		return it->second;

	result.insert ( { name, name } );

	return result[ name ];
}
//-----------------------------------------------------------------------------
