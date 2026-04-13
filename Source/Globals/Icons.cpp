#include "Icons.h"
#include "constants.h"

//-----------------------------------------------------------------------------

Icons::Icons ()
{
	file = paths::getDataRoot ( "UI/icons.yml" );
	load ();
}
//-----------------------------------------------------------------------------

void Icons::load ()
{
	yamlFile::load ();

	// Replace missing icons with "notdef-solid-full" (a blank square)
	for ( auto& ent : result )
		if ( ! paths::getDataRoot ( "UI/svg/" + ent.second + ".svg" ).existsAsFile () )
			ent.second = "notdef-solid-full";
}
//-----------------------------------------------------------------------------

const juce::String& Icons::get ( const juce::String& name )
{
	if ( auto it = result.find ( name ); it != result.end () )
		return it->second;

	result.insert ( { name, "bug-solid-full" } );

	return result[ name ];
}
//-----------------------------------------------------------------------------
