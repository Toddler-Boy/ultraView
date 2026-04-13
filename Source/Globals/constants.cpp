#include <format>

#include <JuceHeader.h>

#include "constants.h"
#include "Icons.h"
#include "Playlists.h"
#include "Preferences.h"
#include "Settings.h"
#include "Strings.h"
#include "Tags.h"

#include "UI/SID_LookAndFeel.h"

//-----------------------------------------------------------------------------

juce::String SID::convertTimeToString ( int timeMS )
{
	if ( timeMS < 0 )
		return {};

	// Round up by half a second
	timeMS += 500;

	// Convert millisecond counter to human readable time
	const auto	hours = timeMS / ( 1000 * 60 * 60 );		timeMS -= hours * 1000 * 60 * 60;
	const auto	min = timeMS / ( 1000 * 60 );				timeMS -= min * 1000 * 60;
	const auto	sec = timeMS / 1000;

	if ( hours )	return std::format ( "{}:{:02}:{:02}", hours, min, sec );

	return std::format ( "{}:{:02}", min, sec );
}
//-----------------------------------------------------------------------------

juce::String SID::convertToLongTimeString ( int timeMS )
{
	if ( timeMS < 0 )
		return {};

	// Round up by half a second
	timeMS += 500;

	// Convert millisecond counter to human readable time
	const auto	days = timeMS / ( 1000 * 60 * 60 * 24 );	timeMS -= days * 1000 * 60 * 60 * 24;
	const auto	hours = timeMS / ( 1000 * 60 * 60 );		timeMS -= hours * 1000 * 60 * 60;
	const auto	min = timeMS / ( 1000 * 60 );				timeMS -= min * 1000 * 60;
	const auto	sec = timeMS / 1000;

	auto	ret = juce::String ();

	if ( days )		ret += juce::String ( days ) + " days ";
	if ( hours )	ret += juce::String ( hours ) + " hr ";
	if ( min )		ret += juce::String ( min ) + " min ";
	if ( ! hours )	ret += juce::String ( sec ) + " sec";

	return ret.trim ();
}
//-----------------------------------------------------------------------------

uint32_t SID::getTuneLength ( const std::string& tuneName, int subTune )
{
	const juce::SharedResourcePointer<HVSC_database>	hvscDB;

	if ( const auto	lengthMs = hvscDB->getLengthMs ( tuneName, subTune ) )
		return lengthMs;

	const juce::SharedResourcePointer<Preferences>	settings;

	return settings->get<int> ( "Songs", "unknown" ) * 60u * 1000u;
}
//-----------------------------------------------------------------------------

std::tuple<uint32_t, uint32_t, float, bool> SID::getRenderInfo ( const std::string& tuneName, const int subtune )
{
	const juce::SharedResourcePointer<HVSC_database>	hvscDB;
	const juce::SharedResourcePointer<Database>			database;

	const auto	lufs = database->getSongLoudness ( tuneName, subtune );
	auto		len = SID::getTuneLength ( tuneName, subtune );
	const auto	filterUsed = database->getSongFilterUsed ( tuneName, subtune );

	const juce::SharedResourcePointer<Preferences>	settings;

	// Extend song-render-length considering minimum-length, repetition count, and fade-out
	{
		auto		songMinimum = settings->get<int> ( "Songs", "minimum" ) * 1000u;
		const auto	songLoops = settings->get<int> ( "Songs", "max-loops" ) * len;

		if ( songLoops )
			songMinimum = std::min ( songLoops, songMinimum );

		len = std::max ( len, songMinimum );
	}
	const auto	songFade = settings->get<int> ( "Songs", "fade-out" ) * 1000u;

	return { len + songFade, songFade, lufs, filterUsed };
}
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

juce::Colour UI::getAverageColor ( const juce::Image& img, const float bright, const float satMul, const float satDiv )
{
	auto	r = 0.0f;
	auto	g = 0.0f;
	auto	b = 0.0f;

	// Get average color, weighted by saturation
	{
		auto	src = juce::Image::BitmapData ( img, juce::Image::BitmapData::ReadWriteMode::readOnly );
		auto	countedPixels = 0.0f;

		for ( auto y = 0; y < img.getHeight (); ++y )
		{
			for ( auto x = 0; x < img.getWidth (); ++x )
			{
				const auto	col = src.getPixelColour ( x, y ).withMultipliedSaturation ( satMul );
				const auto	sat = col.getSaturation ();

				r += col.getFloatRed () * sat;
				g += col.getFloatGreen () * sat;
				b += col.getFloatBlue () * sat;

				countedPixels += sat;
			}
		}

		r /= countedPixels;
		g /= countedPixels;
		b /= countedPixels;
	}

	auto	col = juce::Colour::fromFloatRGBA ( r, g, b, 1.0f ).withMultipliedSaturation ( satDiv );

	if ( bright <= 0.0f )
		return col;

	return UI::getColorWithPerceivedBrightness ( col, bright );
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

std::unique_ptr<juce::Drawable> UI::getMenuIcon ( const juce::String& name )
{
	auto	[ iconImage, _ ] = UI::getSVG ( name );

	iconImage->replaceColour ( juce::Colours::black, endColor );

	return std::move ( iconImage );
}
//-----------------------------------------------------------------------------

juce::PopupMenu::Item UI::newMenuItem ( const juce::String& name, const juce::String& icon, std::function<void ()> func )
{
	juce::PopupMenu::Item	item ( name );

	item.setAction ( std::move ( func ) );
	item.setImage ( getMenuIcon ( icon ) );

	return item;
}
//-----------------------------------------------------------------------------

juce::PopupMenu::Item UI::newDangerousMenuItem ( const juce::String& name, const juce::String& icon, std::function<void ()> func )
{
	auto	item = newMenuItem ( name, icon, std::move ( func ) );

	return item.setColour ( juce::Colours::red );
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

const Database::entry* UI::findDatabaseEntry ( const std::string& filename )
{
	const juce::SharedResourcePointer<Database>	database;

	if ( auto ent = database->findEntry ( filename ) )
		return ent;

	const juce::SharedResourcePointer<UserDatabase>	userDatabase;
	return userDatabase->findEntry ( filename );
}
//-----------------------------------------------------------------------------

void UI::repaintCell ( juce::TableListBox* tlb, const int rowNumber, const int columnId )
{
	if ( tlb->getHeader ().getIndexOfColumnId ( columnId, true ) < 0 )
		return;

	auto	cp = tlb->getCellPosition ( columnId, rowNumber, true );
	if ( cp.isEmpty () )
		return;

	tlb->repaint ( cp );
}
//-----------------------------------------------------------------------------

void UI::repaintColumn ( juce::TableListBox* tlb, const int columnId )
{
	auto&	header = tlb->getHeader ();
	auto	headerCell = header.getColumnPosition ( header.getIndexOfColumnId ( columnId, true ) );

	headerCell.translate ( header.getX (), 0 );

	tlb->repaint ( headerCell.withY ( 0 ).withHeight ( tlb->getHeight () ) );
}
//-----------------------------------------------------------------------------

void UI::menu_AddToPlaylist ( juce::PopupMenu& m, const juce::String& tuneStr )
{
	juce::PopupMenu	submenu;

	// Add to new playlist
	submenu.addItem ( UI::newMenuItem ( UI::strings::newPlaylist, "plus-solid-full", [ tuneStr ]
	{
		const juce::SharedResourcePointer<Playlists>	playlists;

		auto	newName = playlists->addToPlaylist ( "", tuneStr );
		sendGlobalMessage ( "playlist new \"{}\"", newName );
	} ) );

	submenu.addSeparator ();

	// Add to existing playlist
	{
		const juce::SharedResourcePointer<Playlists>	playlists;

		for ( auto cnt = 0; const auto& name : playlists->getPlaylistNames () )
		{
			submenu.addItem ( UI::newMenuItem ( name, "list-solid-full", [ name, tuneStr ]
			{
				const juce::SharedResourcePointer<Playlists>	playlists;

				playlists->addToPlaylist ( name, tuneStr );
				sendGlobalMessage ( "playlist addTo \"{}\"", name );
			} ) );

			if ( ++cnt % 10 == 0 )
				submenu.addColumnBreak ();
		}
	}

	m.addSubMenu ( UI::strings::addToPlaylist, submenu, true, UI::getMenuIcon ( "plus-solid-full" ) );
}
//-----------------------------------------------------------------------------

void UI::menu_RemoveFromPlaylist ( juce::PopupMenu& m, const juce::String& plName, const juce::SparseSet<int>& rows )
{
	const juce::SharedResourcePointer<Playlists>	playlists;

	auto	plItems = playlists->getPlaylistItems ( plName.toStdString () );

	// Remove items from playlist
	m.addItem ( UI::newDangerousMenuItem ( UI::strings::removeFromPlaylist, "trash-can-regular-full", [ plItems, rows ]
	{
		if ( ! plItems )
			return;

		for ( auto i = rows.size () - 1; i >= 0; --i )
			plItems->removeItem ( rows[ i ], true );

		plItems->createShuffle ();
		plItems->save ();

		sendGlobalMessage ( "playlist update \"{}\"", plItems->getName () );

	} ).setEnabled ( plItems && plItems->hasWriteAccess () ) );
}
//-----------------------------------------------------------------------------

void UI::menu_MoveItems ( juce::PopupMenu& m, const juce::String& plName, const juce::SparseSet<int>& rows )
{
	const juce::SharedResourcePointer<Playlists>	playlists;

	auto	plItems = playlists->getPlaylistItems ( plName.toStdString () );

	// Move to top
	m.addItem ( UI::newMenuItem ( UI::strings::moveToTop, "arrow-up-solid-full", [ plItems, rows ]
	{
		plItems->moveItems ( rows, 0 );
		plItems->createShuffle ();
		plItems->save ();

		sendGlobalMessage ( "playlist update \"{}\"", plItems->getName () );

	} ).setEnabled ( plItems&& plItems->hasWriteAccess () ) );

	// Move to bottom
	m.addItem ( UI::newMenuItem ( UI::strings::moveToBottom, "arrow-down-solid-full", [ plItems, rows ]
	{
		plItems->moveItems ( rows, plItems->getNumItems () );
		plItems->createShuffle ();
		plItems->save ();

		sendGlobalMessage ( "playlist update \"{}\"", plItems->getName () );

	} ).setEnabled ( plItems && plItems->hasWriteAccess () ) );
}
//-----------------------------------------------------------------------------

void UI::menu_GoToFolder ( juce::PopupMenu& m, const juce::String& folder )
{
	// TODO: re-implment this differently
	auto	authorStr = juce::String ( UI::strings::goToFolder );

	if ( folder.isEmpty () )
		authorStr += " <multiple>";
	else
		authorStr += " " + folder;

	m.addItem ( UI::newMenuItem ( authorStr, "folder-open-solid-full", [ folder ]
	{
		sendGlobalMessage ( "goToFolder \"{}\"", folder );

	} ).setEnabled ( folder.isNotEmpty () ) );
}
//-----------------------------------------------------------------------------

void UI::menu_ExportTrack ( juce::PopupMenu& m, const juce::String& tuneStr )
{
	m.addItem ( UI::newMenuItem ( UI::strings::exportTrack, "arrow-up-right-from-square-solid-full", [ tuneStr ]
	{
		sendGlobalMessage ( "exportTune {}", tuneStr );
	} ) );
}
//-----------------------------------------------------------------------------

void UI::menu_DeleteCover ( juce::PopupMenu& m, const juce::String& plName )
{
	const juce::SharedResourcePointer<Playlists>	playlists;

	auto	plItems = playlists->getPlaylistItems ( plName.toStdString () );

	m.addItem ( UI::newDangerousMenuItem ( UI::strings::playlistDeleteCover, "trash-can-regular-full", [ plName ]
	{
		const juce::SharedResourcePointer<Playlists>	playlists;

		playlists->deleteCover ( plName.toStdString () );

		sendGlobalMessage ( "playlist updateInfo \"{}\"", plName );

	} ).setEnabled ( plItems->hasCover () ) );
}
//-----------------------------------------------------------------------------

void UI::menu_DeletePlaylist ( juce::PopupMenu& m, const juce::String& plName )
{
	const juce::SharedResourcePointer<Playlists>	playlists;

	auto	plItems = playlists->getPlaylistItems ( plName.toStdString () );

	m.addItem ( UI::newDangerousMenuItem ( UI::strings::playlistDelete, "trash-can-regular-full", [ plName ]
	{
		const juce::SharedResourcePointer<Playlists>	playlists;

		sendGlobalMessage ( "playlist deleteList \"{}\"", plName );

	} ).setEnabled ( plItems ) );
}
//-----------------------------------------------------------------------------

void UI::menu_ToggleTag ( juce::PopupMenu& m, const juce::String& tuneStr )
{
	const juce::SharedResourcePointer<Tags>		tags;
	const juce::SharedResourcePointer<Strings>	strings;
	const juce::SharedResourcePointer<Icons>	icons;

	for ( const auto& tag : tags->getTagEntries () )
	{
		m.addItem ( UI::newMenuItem ( UI::strings::toggleTag + strings->get ( tag.name ), icons->get ( tag.name ), [ name = tag.name, tuneStr, tags ]
		{
			tags->toggleTags ( name, tuneStr.toStdString () );
			sendGlobalMessage ( "tagsToggled" );
		} ) );
	}
}
//-----------------------------------------------------------------------------

bool UI::isDeveloperMode ()
{
	return juce::SystemStats::getEnvironmentVariable ( "ULTRASID_DEVELOPER_MODE", "" ).equalsIgnoreCase ( "enabled" );
}
//-----------------------------------------------------------------------------

juce::File paths::getDataRoot ( juce::String path )
{
	#if JUCE_MAC
		auto	ret = juce::File::getSpecialLocation ( juce::File::commonApplicationDataDirectory ).getChildFile ( "Application Support/ultraSID" );
	#elif JUCE_WINDOWS
		auto	ret = juce::File::getSpecialLocation ( juce::File::commonApplicationDataDirectory ).getChildFile ( "ultraSID" );
	#elif JUCE_LINUX
		auto	ret = juce::File::getSpecialLocation ( juce::File::commonApplicationDataDirectory ).getChildFile ( "ultraSID" );
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

static juce::File getAppDataPath ( const juce::String& file )
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

juce::File paths::getHistoryPath ()
{
	return getAppDataPath ( "history.csv" );
}
//-----------------------------------------------------------------------------

static juce::File getUserPath ( const juce::String& folder )
{
	const juce::SharedResourcePointer<Settings>	settings;

	auto	path = settings->get<juce::String> ( "Paths", "user" );
	if ( path.isEmpty () )
		return {};

	auto	subFolder = juce::File ( path ).getChildFile ( folder );
	subFolder.createDirectory ();

	return subFolder;
}
//-----------------------------------------------------------------------------

juce::File paths::getPlaylistsPath ()
{
	return getUserPath ( "Playlists" );
}
//-----------------------------------------------------------------------------

juce::File paths::getUserTunesPath ()
{
	return getUserPath ( "Tunes" );
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

std::pair<std::string, int> helpers::parseTuneName ( const std::string& tuneName )
{
	const auto	commaOffset = tuneName.find_last_of ( ',' );
	auto	subTune = 0;

	// If this entry uses a subtune, remember it
	if ( commaOffset != std::string::npos )
		subTune = std::atoi ( tuneName.substr ( commaOffset + 1 ).c_str () );

	return { tuneName.substr ( 0, commaOffset ), subTune };
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

helpers::imageHint helpers::hintFromFilename ( const juce::String& in )
{
	if ( in.isEmpty () )
		return {};

	auto	arr = juce::StringArray::fromTokens ( in, "#.", "" );

	jassert ( arr.size () >= 2 );

	// No hints, add empty string
	if ( arr.size () == 2 )
		arr.insert ( 1, "" );

	imageHint	hint;

	hint.name = arr[ 0 ];
	hint.extension = "." + arr[ 2 ];

	auto	hintStr = arr[ 1 ].toUpperCase ();

	hint.firstLuma = hintStr.containsChar ( 'L' );
	hint.isThumbnail = hintStr.containsChar ( 'T' );

	hintStr = hintStr.removeCharacters ( "LT" );

	// Last parameter left over is always border color
	if ( hintStr.isNotEmpty () )
		hint.borderColor = int8_t ( juce::String ( UI::strings::hexDigits ).indexOfChar ( hintStr[ 0 ] ) );

	return hint;
}
//-----------------------------------------------------------------------------

juce::String helpers::filenameFromHint ( const helpers::imageHint& hint )
{
	if ( hint.name.isEmpty () )
		return {};

	auto	hintStr = juce::String ();

	if ( hint.isThumbnail )		hintStr += "T";
	if ( hint.firstLuma )		hintStr += "L";
	if ( hint.borderColor >= 0 && hint.borderColor < 16 )
		hintStr += UI::strings::hexDigits[ hint.borderColor ];

	if ( hintStr.isNotEmpty () )
		hintStr = "#" + hintStr;

	return hint.name + hintStr + hint.extension;
}
//-----------------------------------------------------------------------------
