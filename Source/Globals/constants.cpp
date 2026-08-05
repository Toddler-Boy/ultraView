#include <JuceHeader.h>

#include "constants.h"

#include "ultra-shared/Helpers/Regex.h"
#include "ultra-shared/UI/GUI_LookAndFeel.h"

//-----------------------------------------------------------------------------

juce::String helpers::appTitle ()
{
	auto	title = juce::String ( ProjectInfo::projectName ) + " " + ProjectInfo::versionString;

	#if ULTRA_DEVELOPMENT
		title += " - development";
	#elif JUCE_DEBUG
		title += " - debug";
	#endif

	return title;
}
//-----------------------------------------------------------------------------

juce::Font UI::font ( const float height, const int weight )
{
	auto&	laf = static_cast<GUI_LookAndFeel&> ( juce::LookAndFeel::getDefaultLookAndFeel () );

	return laf.font ( height, weight );
}
//-----------------------------------------------------------------------------

juce::Font UI::monoFont ( const float height )
{
	auto&	laf = static_cast<GUI_LookAndFeel&> ( juce::LookAndFeel::getDefaultLookAndFeel () );

	return laf.monoFontWithHeight ( height );
}
//-----------------------------------------------------------------------------

juce::File paths::getAppDataPath ( const juce::String& file )
{
	auto	path = juce::File::getSpecialLocation ( juce::File::SpecialLocationType::userApplicationDataDirectory ).getChildFile ( ProjectInfo::projectName );

	path.createDirectory ();
	path = path.getChildFile ( file );

	return path;
}
//-----------------------------------------------------------------------------

std::pair<juce::String, juce::StringArray> helpers::parseActionMessage ( const juce::String& message )
{
	auto	parts = juce::StringArray::fromTokens ( message, " ", "\"" );

	for ( auto& part : parts )
		part = part.unquoted ();

	auto	cmd = parts[ 0 ];
	parts.remove ( 0 );
	return { cmd, parts };
}
//-----------------------------------------------------------------------------

std::string helpers::createActionMessage ( const juce::String& command, const juce::StringArray& args )
{
	if ( args.isEmpty () )
		return {};

	juce::String	str;

	for ( const auto& a : args )
		str += a.quoted () + " ";

	return ( command + " " + str.dropLastCharacters ( 1 ) ).toStdString ();
}
//-----------------------------------------------------------------------------

int helpers::strnatcmp ( const char* const a, const char* const b )
{
	auto isspace = [] ( const unsigned char c ) { return c && c <= 32; };
	auto isdigit = [] ( const unsigned char c ) { return c >= '0' && c <= '9'; };
	auto compare_right = [ &isdigit ] ( const char* a, const char* b ) -> int
	{
		auto	bias = 0;

		// The longest run of digits wins. That aside, the greatest
		// value wins, but we can't know that it will until we've scanned
		// both numbers to know that they have the same magnitude, so we
		// remember it in BIAS.
		for ( ;; a++, b++ )
		{
			if ( ! isdigit ( *a ) && ! isdigit ( *b ) )	return bias;
			if ( ! isdigit ( *a ) )						return -1;
			if ( ! isdigit ( *b ) )						return +1;

			if ( *a < *b )
			{
				if ( !bias )
					bias = -1;
			}
			else if ( *a > *b )
			{
				if ( ! bias )
					bias = +1;
			}
			else if ( ! *a && ! *b )
				return bias;
		}

		std::unreachable ();
	};
	auto compare_left = [ &isdigit ] ( const char* a, const char* b ) -> int
	{
		// Compare two left-aligned numbers: the first to have a different value wins
		for ( ;; a++, b++ )
		{
			if ( ! isdigit ( *a ) && ! isdigit ( *b ) )		return 0;
			if ( ! isdigit ( *a ) )							return -1;
			if ( ! isdigit ( *b ) )							return +1;
			if ( *a < *b )									return -1;
			if ( *a > *b )									return +1;
		}

		std::unreachable ();
	};

	auto	ai = 0;
	auto	bi = 0;

	while ( 1 )
	{
		auto	ca = a[ ai ];
		auto	cb = b[ bi ];

		// skip over leading spaces or zeros
		while ( isspace ( ca ) )	ca = a[ ++ai ];
		while ( isspace ( cb ) )	cb = b[ ++bi ];

		// process run of digits
		if ( isdigit ( ca ) && isdigit ( cb ) )
		{
			const auto	fractional = ( ca == '0' || cb == '0' );

			if ( fractional )
			{
				if ( auto result = compare_left ( a + ai, b + bi ); result != 0 )
					return result;
			}
			else
			{
				if ( auto result = compare_right ( a + ai, b + bi ); result != 0 )
					return result;
			}
		}

		if ( ! ca && ! cb )		return std::strcmp ( a, b );
		if ( ca < cb )			return -1;
		if ( ca > cb )			return +1;

		++ai;
		++bi;
	}

	std::unreachable ();
}
//-----------------------------------------------------------------------------

int helpers::romanToInt ( std::string s )
{
	std::map<char, int>	m = { { 'i',1 }, { 'v',5 }, { 'x',10 }, { 'l',50 }, { 'c',100 }, { 'd',500 }, { 'm',1000 } };
	auto	res = 0;

	for ( auto i = 0; i < s.length (); ++i )
	{
		if ( i + 1 < s.length () && m[ s[ i ] ] < m[ s[ i + 1 ] ] )
			res -= m[ s[ i ] ];
		else
			res += m[ s[ i ] ];
	}
	return res;
}
//-----------------------------------------------------------------------------

std::string helpers::normalizeGamesString ( const std::string& input )
{
	// Replace Roman numerals, standalone words only, with their decimal equivalents
	const regex::Pattern	romanRegex ( R"((\s)([ivxlcdm]+)(?=$|[\s]))" );

	auto	output = romanRegex.replaceAll ( input, [] ( const std::vector<std::string>& groups )
	{
		return groups[ 1 ] + std::to_string ( romanToInt ( groups[ 2 ] ) );
	} );

	// Remove dots in name and replace underscores with spaces
	{
		std::erase ( output, '.' );
		std::erase ( output, '\'' );
		std::ranges::replace ( output, '_', ' ' );
		std::ranges::replace ( output, '-', ' ' );
	}

	return output;
}
//-----------------------------------------------------------------------------
