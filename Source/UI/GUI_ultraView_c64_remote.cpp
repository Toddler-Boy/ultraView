#include "GUI_ultraView.h"

//-----------------------------------------------------------------------------

void GUI_ultraView::c64_reboot ()
{
	network.put ( "v1/machine:reboot" );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::c64_run ( const juce::String& type, const juce::MemoryBlock& crtData, const juce::String& filename )
{
	// Find game in our database, so that we can configure the C64 and CRT-emulation before running it
	const auto	entry =	findGameEntry ( filename );

	c64_forceSystemMode ( entry.isNTSC ? "NTSC" : "PAL" );
	c64_forceJoystickSwapper ( entry.firstJoyport ? "Swapped" : "Normal" );
	mainScreen.crt.setFirstLuma ( entry.firstLuma );

	network.post ( "v1/runners:run_" + type, crtData, [ filename ] ( const juce::var& response, const int statusCode )
	{
		if ( statusCode != 200 )
		{
			Z_ERR ( "Failed to upload file: " << filename.quoted () << "\n" << response[ "errors" ].toString () );
		}
	} );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::c64_forceSystemMode ( const juce::String& mode )
{
	network.put ( "v1/configs/U64 Specific Settings/System Mode", { "value", mode } );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::c64_forceJoystickSwapper ( const juce::String& mode )
{
	network.put ( "v1/configs/U64 Specific Settings/Joystick Swapper", { "value", mode } );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::loadGamesDatabase ()
{
	gamesDatabase.clear ();

	const auto	gamesFile = paths::getDataRoot ( "Data/Games.csv" );
	const auto	csv = gamesFile.loadFileAsString ().toLowerCase ();
	const auto	lines = juce::StringArray::fromLines ( csv );

	const auto	columns = juce::StringArray::fromTokens ( lines[ 0 ], ",", "" );
	for ( auto i = 1; i < lines.size (); ++i )
	{
		auto	line = lines[ i ];
		auto	values = juce::StringArray::fromTokens ( line, ",", "" );
		if ( values.size () != columns.size () )
			continue;

		GameEntry	entry;

		for ( auto j = 0; j < columns.size (); ++j )
		{
			const auto&	column = columns[ j ].trim ();
			const auto&	value = values[ j ].trim ();

			if ( column == "name" )			entry.name			= value.toStdString ();
			else if ( column == "sys" )		entry.isNTSC		= value == "ntsc";
			else if ( column == "vic" )		entry.firstLuma		= value == "first";
			else if ( column == "joy" )		entry.firstJoyport	= value == "1";
		}

		gamesDatabase.push_back ( std::move ( entry ) );
	}
}
//-----------------------------------------------------------------------------

GUI_ultraView::GameEntry GUI_ultraView::findGameEntry ( juce::String filename ) const
{
	filename = filename.toLowerCase ();
	filename = filename.replaceCharacter ( '\\', '/' );
	filename = filename.fromLastOccurrenceOf ( "/", false, false );
	filename = filename.upToLastOccurrenceOf ( ".", false, false );
	filename = filename.trim ();

	const auto	hasJ1 = filename.containsIgnoreCase ( "(j1)" );
	filename = filename.replace ( " (J1)", "" );

	// Early out for games with () in them
	{
		const auto	normalizedName = filename.toStdString ();

		auto	it = std::ranges::find_if ( gamesDatabase, [ &normalizedName ] ( const GameEntry& entry ) {
			return normalizedName == entry.name;
		} );

		if ( it != gamesDatabase.end () )
			return *it;
	}

	// Not found, try some more aggressive filename cleanup and search again
	filename = filename.upToFirstOccurrenceOf ( "(", false, false );
	filename = filename.upToFirstOccurrenceOf ( "[", false, false );
	filename = filename.trim ();

	filename = filename.retainCharacters ( "abcdefghijklmnopqrstuvwxyz0123456789 " );
	while ( filename.contains ( "  " ) )
		filename = filename.replace ( "  ", " " );

	if ( filename.startsWith ( "the " ) )	filename = filename.substring ( 4 );
	if ( filename.endsWith ( " the" ) )	filename = filename.dropLastCharacters ( 4 );

	const auto	normalizedName = helpers::normalizeGamesString ( filename.toStdString () );

	auto	it = std::ranges::find_if ( gamesDatabase, [ &normalizedName ] ( const GameEntry& entry ) {
		return normalizedName == entry.name;
	} );

	if ( it != gamesDatabase.end () )
		return *it;

	// Game is not in list, use filename to make a best effort guess for joystick swap
	return { .firstJoyport = hasJ1 };
}
//-----------------------------------------------------------------------------
