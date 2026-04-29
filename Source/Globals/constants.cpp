#include <format>

#include <JuceHeader.h>

#include "constants.h"
#include "Icons.h"
#include "Preferences.h"
#include "Settings.h"
#include "Strings.h"

#include "UI/SID_LookAndFeel.h"

#include <regex>

//-----------------------------------------------------------------------------

namespace UI
{
	static	juce::ActionBroadcaster*	ab = nullptr;

	static	juce::Colour	startColor;
	static	juce::Colour	endColor;
}
//-----------------------------------------------------------------------------

void UI::setActionBroadCaster ( juce::ActionBroadcaster* _ab ) noexcept
{
	ab = _ab;
}
//-----------------------------------------------------------------------------

void UI::setShades ( const juce::Colour col1, const juce::Colour col2 ) noexcept
{
	startColor = col1;
	endColor = col2;
}
//-----------------------------------------------------------------------------

juce::Colour UI::getShade ( const float blend ) noexcept
{
	return startColor.interpolatedWith ( endColor, blend );
}
//-----------------------------------------------------------------------------

juce::Colour UI::getColorFromName ( const juce::String& name, const float brightness )
{
	auto	hash = float ( double ( name.hashCode64 () ) / double ( std::numeric_limits<int64_t>::max () ) );

	return juce::Colour::fromHSV ( hash, 0.5f, brightness, 1.0f );
}
//-----------------------------------------------------------------------------

juce::String UI::getHumanNumber ( int64_t number, const char thousand_separator )
{
	char	dst[ 64 ];
	auto	p = &dst[ sizeof ( dst ) - 1 ];	// points to terminating 0
	*p = 0;

	auto	cnt = 0;
	do
	{
		if ( cnt++ == 3 )
		{
			cnt = 1;
			*--p = thousand_separator;
		}

		*--p = '0' + char ( number % 10 );
		number /= 10;

	} while ( number != 0 );

	return juce::String ( p );
}
//-----------------------------------------------------------------------------

float UI::easeInOutQuad ( float t )
{
	const auto	sqr = t * t;
	return sqr / ( 2.0f * ( sqr - t ) + 1.0f );
}
//-----------------------------------------------------------------------------

juce::Colour UI::getColorWithPerceivedBrightness ( juce::Colour input, const float targetL ) noexcept
{
	auto	low = 0.0f;
	auto	high = 1.0f;
	auto	bestMatch = input;

	// 8 iterations gives ~0.0039 precision
	for ( auto i = 0; i < 8; ++i )
	{
		const auto	midV = ( low + high ) * 0.5f;
		const auto	testCol = input.withBrightness ( midV );
		const auto	currentL = testCol.getPerceivedBrightness ();

		if ( std::abs ( currentL - targetL ) <= 0.005f )
			return testCol;

		if ( currentL < targetL )
			low = midV;
		else
			high = midV;

		bestMatch = testCol;
	}

	// If the color is still too dark, try adjusting saturation instead of brightness
	if ( bestMatch.getPerceivedBrightness () < targetL - 0.005f )
	{
		auto	lowS = 0.0f;
		auto	highS = bestMatch.getSaturation ();

		for ( auto i = 0; i < 8; ++i )
		{
			const auto	midS = ( lowS + highS ) * 0.5f;
			const auto	testCol = bestMatch.withSaturation ( midS );
			const auto	currentL = testCol.getPerceivedBrightness ();

			if ( std::abs ( currentL - targetL ) <= 0.005f )
				return testCol;

			if ( currentL < targetL )
				highS = midS;
			else
				lowS = midS;

			bestMatch = testCol;
		}
	}

	return bestMatch;
}
//-----------------------------------------------------------------------------

juce::Font UI::font ( const float height, const int weight )
{
	auto&	laf = static_cast<SID_LookAndFeel&> ( juce::LookAndFeel::getDefaultLookAndFeel () );

	return laf.font ( height, weight );
}
//-----------------------------------------------------------------------------

juce::Font UI::monoFont ( const float height )
{
	auto&	laf = static_cast<SID_LookAndFeel&> ( juce::LookAndFeel::getDefaultLookAndFeel () );

	return laf.monoFontWithHeight ( height );
}
//-----------------------------------------------------------------------------

std::pair<std::unique_ptr<juce::Drawable>, int> UI::getSVG ( const juce::String& svgName )
{
	auto	svgFile = paths::getDataRoot ( "UI/svg/" + svgName + ".svg" );
	auto	svgStr = svgFile.loadFileAsString ();
	jassert ( svgStr.isNotEmpty () );

	auto	svg = juce::XmlDocument::parse ( svgStr );
	jassert ( svg );

	auto	viewBoxSize = 0;
	if ( auto vbStr = svg->getStringAttribute ( "viewBox" ); vbStr.isNotEmpty () )
	{
		auto	vb = juce::Rectangle<int>::fromString ( vbStr );
		viewBoxSize = std::max ( vb.getWidth (), vb.getHeight () );
	}
	else if ( auto w = svg->getIntAttribute ( "width" ), h = svg->getIntAttribute ( "height" ); w > 0 && h > 0 )
	{
		viewBoxSize = std::max ( w, h );
	}

	jassert ( viewBoxSize > 0 );

	return { juce::Drawable::createFromSVG ( *svg ), viewBoxSize };
}
//-----------------------------------------------------------------------------

juce::Path& UI::getScaledPath ( const juce::String& resourceName, juce::Rectangle<float> rect, juce::RectanglePlacement placement /*= 0*/, float padding /* = 0.0f */ )
{
	padding *= std::min ( rect.getWidth (), rect.getHeight () );
	rect.reduce ( padding, padding );

	//
	// Cache
	//
	struct cacheEntry
	{
		juce::Rectangle<float>		rect;
		juce::RectanglePlacement	placement;
		juce::Path					path;
	};

	static juce::HashMap<juce::String, cacheEntry>	paths;

	// Already in cache?
	if ( paths.contains ( resourceName ) )
	{
		auto&	p = paths.getReference ( resourceName );

		// Cache matches sizes?
		if ( p.rect == rect && p.placement == placement )
			return p.path;
	}

	auto	[ drawable, _ ] = getSVG ( resourceName );

	//
	// Convert SVG to scaled path
	//
	drawable->setTransformToFit ( rect, placement );
	auto	path = drawable->getOutlineAsPath ();

	//
	// Store in cache
	//
	paths.set ( resourceName, { rect, placement, path } );
	return paths.getReference ( resourceName ).path;
}
//-----------------------------------------------------------------------------

juce::Path& UI::getScaledPathWithSize ( const juce::String& resourceName, juce::Rectangle<float> rect, juce::RectanglePlacement placement /*= 0*/, float padding /* = 0.0f */ )
{
	padding *= std::min ( rect.getWidth (), rect.getHeight () );
	rect.reduce ( padding, padding );

	//
	// Cache
	//
	struct cacheEntry
	{
		juce::Rectangle<float>		rect;
		juce::RectanglePlacement	placement;
		juce::Path					path;
	};

	static juce::HashMap<juce::String, cacheEntry>	paths;

	// Already in cache?
	if ( paths.contains ( resourceName ) )
	{
		auto& p = paths.getReference ( resourceName );

		// Cache matches sizes?
		if ( p.rect == rect && p.placement == placement )
			return p.path;
	}

	auto [ drawable, orgSize ] = getSVG ( resourceName );

	//
	// Convert SVG to scaled path
	//
	{
		const auto	p = drawable->getOutlineAsPath ();
		const auto	b = p.getBounds ();
		const auto	newSize = std::max ( rect.getWidth (), rect.getHeight () ) / orgSize;

		auto	newX = b.getWidth () * 0.5f * newSize;
		auto	newY = b.getHeight () * 0.5f * newSize;

		if ( placement.testFlags ( juce::RectanglePlacement::xMid ) )
			newX = rect.getCentreX ();

		if ( placement.testFlags ( juce::RectanglePlacement::yMid ) )
			newY = rect.getCentreY ();

		auto	trans = juce::AffineTransform::translation ( b.getWidth () * -0.5f - b.getX (), b.getHeight () * -0.5f - b.getY () )
						.scaled ( newSize )
						.translated ( newX, newY );

		drawable->setTransform ( trans );
	}

	auto	path = drawable->getOutlineAsPath ();

	//
	// Store in cache
	//
	paths.set ( resourceName, { rect, placement, path } );
	return paths.getReference ( resourceName ).path;
}
//-----------------------------------------------------------------------------

juce::File paths::getDataRoot ( juce::String path )
{
	#if JUCE_MAC
		auto	ret = juce::File::getSpecialLocation ( juce::File::commonApplicationDataDirectory ).getChildFile ( "Application Support/ultraView" );
	#elif JUCE_WINDOWS
		auto	ret = juce::File::getSpecialLocation ( juce::File::commonApplicationDataDirectory ).getChildFile ( "ultraView" );
	#elif JUCE_LINUX
		auto	ret = juce::File::getSpecialLocation ( juce::File::commonApplicationDataDirectory ).getChildFile ( "ultraView" );
	#else
		// Unsupported platform
		jassertfalse;
		auto	ret = juce::File();
	#endif

	if ( path.isNotEmpty () )
		ret = ret.getChildFile ( path );

	return ret;
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

juce::File paths::getSearchtermsPath ()
{
	return getAppDataPath ( "searchterms.csv" );
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

juce::StringArray helpers::getFilteredStrings ( const juce::StringArray& arr, const juce::StringArray& ext )
{
	juce::StringArray	filteredResults;

	for ( const auto& item : arr )
		for ( const auto& suffix : ext )
			if ( item.endsWithIgnoreCase ( suffix ) )
				filteredResults.add ( item );

	return filteredResults;
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

void helpers::buildComponentMap ( std::unordered_map<juce::String, juce::Component*>& compMap, juce::Component* parent, const juce::String& pName )
{
	// Loop over all children recursivly and build a map of component-names
	for ( auto comp : parent->getChildren () )
	{
		auto	fullName = pName.isNotEmpty () ? pName + "/" + comp->getName () : comp->getName ();

		if ( fullName.endsWithChar ( '/' ) )
			continue;

		// Special handling for Viewports
		if ( auto vp = dynamic_cast<juce::Viewport*> ( comp ) )
		{
			compMap[ fullName ] = comp;

			comp = vp->getViewedComponent ();
			buildComponentMap ( compMap, vp->getViewedComponent (), pName.isNotEmpty () ? pName + "/" + vp->getName () + "/" + comp->getName () : vp->getName () + "/" + comp->getName () );
			continue;
		}

		jassert ( compMap.find ( fullName ) == compMap.end () ); // Duplicate component name!

		compMap[ fullName ] = comp;
		buildComponentMap ( compMap, comp, fullName);
	}
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
	auto	output = input;

	// Replace Roman numerals with their decimal equivalents
	{
		// Regex for Roman numerals as standalone words
		std::regex	romanregex ( R"((\s)([ivxlcdm]+)(?=$|[\s]))" );

		auto	words_begin = std::sregex_iterator ( input.begin (), input.end (), romanregex );
		auto	words_end = std::sregex_iterator ();

		auto	offset = 0;

		for ( auto i = words_begin; i != words_end; ++i )
		{
			const auto& match = *i;

			const auto	prefix = match[ 1 ].str ();
			const auto	romanPart = match[ 2 ].str ();

			if ( romanPart.empty () )
				continue;

			auto	decimal = std::to_string ( romanToInt ( romanPart ) );
			auto	replacement = prefix + decimal;

			output.replace ( match.position () + offset, match.length (), replacement );
			offset += int ( replacement.length () - match.length () );
		}
	}

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
