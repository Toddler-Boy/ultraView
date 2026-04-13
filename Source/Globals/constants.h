#pragma once

#include <format>
#include <iostream>
#include <string>
#include <utility>

#include "Database/Database.h"
#include "Database/HVSCDatabase.h"

template <>
struct std::formatter<juce::String> : std::formatter<std::string_view> {
	auto format ( const juce::String& s, std::format_context& ctx ) const {
		return std::formatter<string_view>::format ( std::string ( s.toStdString () ), ctx );
	}
};

namespace browser
{
	constexpr auto	maxHistoryItems = 100;
}

namespace SID
{
	constexpr auto	numVoices = 3;
	constexpr auto	maxChips = 3;

	juce::String convertTimeToString ( int timeMS );
	juce::String convertToLongTimeString ( int timeMS );
	uint32_t getTuneLength ( const std::string& tuneName, int subTune );

	constexpr auto	stingerMs = 5 * 1'000 + 500;
	constexpr auto	songMs = 20 * 1'000 - 500;

	inline bool isFX ( const int lengthMs ) 		{	return lengthMs < stingerMs;	}
	inline bool isStinger ( const int lengthMs )	{	return lengthMs < songMs;		}
	inline bool isSong ( const int lengthMs )		{	return lengthMs >= songMs;		}

	std::tuple<uint32_t, uint32_t, float, bool> getRenderInfo ( const std::string& tuneName, const int subtune );
}

using GUI_STIL_blocks = std::list<std::pair<juce::String, juce::String>>;

enum vic2 : uint8_t
{
	black,
	white,
	red,
	cyan,
	purple,
	green,
	blue,
	yellow,
	orange,
	brown,
	light_red,
	dark_grey,
	grey,
	light_green,
	light_blue,
	light_grey,

	num_colors
};

namespace VIC2
{
	constexpr auto	truePalX = 0.937f;
}

namespace UI
{
	enum colors
	{
		// Basic
		window = 0x1008000,
		text,

		accent,
		accent2,

		// Tags
		tagLiked,
		tagGoldenAge,
		tagGem,

		// STIL
		stilToggleStingers,
		stilToggleSTIL,

		stilBoxTitle,
		stilBoxTitleIcon1,
		stilBoxTitleIcon2,
		stilBoxQuote,
		stilBoxBug,

		// Chips
		chipDivot,
		chipText,

		// Voice colors for the chip visualization
		voiceOff,
		voiceOn,
		filterOn,
		voiceMuted,
		digi,

		// FX mode
		fxReal,
		fxPure,
		fxMagic,

		// Status display
		statusOk,
		statusWarning,
		statusError,

		// End of stuff
		count,

		bento,
		textMuted,
		accentBright,
	};

	namespace shades
	{
		constexpr auto	hover = 1.0f / 12.0f;
		constexpr auto	selected = 1.0f / 6.0f;
	}

	enum columnId : int8_t
	{
		number = 1,
		animation,
		name,
		release,
		information,
		length,

		liked,

		// History
		historyDate,

		// STIL list specific
		tuneNo,
		author,

		// Export
		exportProgress,
	};

	extern juce::ActionBroadcaster*	ab;
	void setActionBroadCaster ( juce::ActionBroadcaster* _ab ) noexcept;

	template<typename... Args>
	void sendGlobalMessage ( std::string_view fmt_str, Args&&... args )
	{
		auto	format_args = std::make_format_args ( args... );
		auto	formatted = std::vformat ( fmt_str, format_args );

		jassert ( ab );
		ab->sendActionMessage ( formatted );
	}

	void setShades ( const juce::Colour col1, const juce::Colour col2 ) noexcept;
	[[ nodiscard ]] juce::Colour getShade ( const float blend ) noexcept;
	[[ nodiscard ]] juce::Colour getAverageColor ( const juce::Image& img, const float bright, const float satMul, const float satDiv );
	[[ nodiscard ]] juce::Colour getColorFromName ( const juce::String& name, const float brightness = 0.25f );
	[[ nodiscard ]] juce::String getHumanNumber ( int64_t number, const char thousand_separator = ',' );
	[[ nodiscard ]] float easeInOutQuad ( float t );
	[[ nodiscard ]] juce::Colour getColorWithPerceivedBrightness ( const juce::Colour col, const float perceivedBrightness ) noexcept;

	[[ nodiscard ]] juce::Font font ( const float height, const int weight = 400 );
	[[ nodiscard ]] juce::Font monoFont ( const float height );

	[[ nodiscard ]] std::unique_ptr<juce::Drawable> getMenuIcon ( const juce::String& name );
	[[ nodiscard ]] juce::PopupMenu::Item newMenuItem ( const juce::String& name, const juce::String& icon, std::function<void ()> func );
	[[ nodiscard ]] juce::PopupMenu::Item newDangerousMenuItem ( const juce::String& name, const juce::String& icon, std::function<void ()> func );

	[[ nodiscard ]] std::pair<std::unique_ptr<juce::Drawable>, int> getSVG ( const juce::String& svgName );
	[[ nodiscard ]] juce::Path& getScaledPath ( const juce::String& resourceName, juce::Rectangle<float> rect, juce::RectanglePlacement placement = 0, float padding = 0.0f );
	[[ nodiscard ]] juce::Path& getScaledPathWithSize ( const juce::String& resourceName, juce::Rectangle<float> rect, juce::RectanglePlacement placement = 0, float padding = 0.0f );

	[[ nodiscard ]] const Database::entry* findDatabaseEntry ( const std::string& filename );

	void repaintCell ( juce::TableListBox* tlb, const int rowNumber, const int columnId );
	void repaintColumn ( juce::TableListBox* tlb, const int columnId );

	void menu_ToggleTag ( juce::PopupMenu& m, const juce::String& tuneStr );

	void menu_AddToPlaylist ( juce::PopupMenu& m, const juce::String& tuneStr );
	void menu_RemoveFromPlaylist ( juce::PopupMenu& m, const juce::String& plName, const juce::SparseSet<int>& rows );
	void menu_MoveItems ( juce::PopupMenu& m, const juce::String& plName, const juce::SparseSet<int>& rows );

	void menu_GoToFolder ( juce::PopupMenu& m, const juce::String& folder );
	void menu_ExportTrack ( juce::PopupMenu& m, const juce::String& tuneStr );

	void menu_DeleteCover ( juce::PopupMenu& m, const juce::String& plName );
	void menu_DeletePlaylist ( juce::PopupMenu& m, const juce::String& plName );

	bool isDeveloperMode ();

	constexpr auto	bentoRadius = 8.0f;
	constexpr auto	bentoGap = 8;

	constexpr auto	disabledAlpha = 0.35f;

	extern juce::Colour	startColor;
	extern juce::Colour	endColor;

	namespace chip
	{
		constexpr auto	numHistory = 64;
		constexpr auto	backBlack = 0.8f;

		constexpr auto	lineScale = 0.7f;

		constexpr auto	SID_REGISTER_VOICE_DELTA = 7;

		constexpr auto	PAL_CLOCK	=  985248.0f / 16777216.0f;
		constexpr auto	NTSC_CLOCK	= 1022730.0f / 16777216.0f;
	}

	namespace fft
	{
		constexpr auto	numHistory = 16;
		constexpr auto	glow = 8.0f;

		const float	freqStartLog = std::log10 ( 20.0f );
		const float	freqRangeLog = std::log10 ( 10'000.0f ) - freqStartLog;

		inline float freqToNormalized ( const float freq )
		{
			return ( std::log10 ( freq ) - freqStartLog ) / freqRangeLog;
		}

		inline float pow2 ( const float a ) { return a * a; }
	}

	namespace strings
	{
		constexpr auto	searchPrompt = "What do you want to listen to?";
		constexpr auto	searchNoResults = "No results";
		constexpr auto	searchResults = " results";

		constexpr auto	addToPlaylist = "Add to playlist";
		constexpr auto	removeFromPlaylist = "Remove from this playlist";
		constexpr auto	moveToTop = "Move to top";
		constexpr auto	moveToBottom = "Move to bottom";
		constexpr auto	removeFromExport = "Cancel export of this tune";
		constexpr auto	goToFolder = "Go to folder";
		constexpr auto	newPlaylist = "New playlist";
		constexpr auto	newPlaylistShort = "My Playlist #1";
		constexpr auto	myPlaylists = "My playlists";
		constexpr auto	playlistDelete = "Delete playlist";
		constexpr auto	playlistDeleteCover = "Delete cover-image";
		constexpr auto	history = "History";
		constexpr auto	exportQueue = "Export queue";
		constexpr auto	exportTrack = "Export tune";

		constexpr auto	toggleTag = "Toggle tag ";

		constexpr auto	hexDigits = "0123456789ABCDEF";

		constexpr auto	hvsc = "High Voltage SID Collection";
		constexpr auto	labelUserPath = "User data (playlists, likes, preferences, non HVSC tunes)";
		constexpr auto	labelStorage = "Storage";

		constexpr auto	headerWelcome = "Welcome to ultraSID";
		constexpr auto	headerInstallingHVSC = "Installing HVSC";
		constexpr auto	statusCanceled = "Canceling...";

		constexpr auto	headerUpdateHVSC = "HVSC is outdated";
		constexpr auto	headerUpdatingHVSC = "Updating HVSC";
	}

	namespace mainButton
	{
		constexpr auto	fontSize = 17.0f;
	}

	namespace list
	{
		constexpr auto	fontSize = 17.0f;
	}

	namespace STIL
	{
		constexpr auto	fontSize = 17.0f;
	}
}

namespace helpers
{
	std::pair<juce::String, juce::StringArray> parseActionMessage ( const juce::String& message );
	std::string createActionMessage ( const juce::String& command, const juce::StringArray& args );
	juce::StringArray getFilteredStrings ( const juce::StringArray& arr, const juce::StringArray& ext );
	std::pair<std::string, int> parseTuneName ( const std::string& tuneName );

	inline std::string strToLower ( std::string str )
	{
		std::ranges::transform ( str, str.begin (), [] ( unsigned char c ) { return std::tolower ( c ); } );
		return str;
	}

	int strnatcmp ( const char* const a, const char* const b );

	void buildComponentMap ( std::unordered_map<juce::String, juce::Component*>& compMap, juce::Component* parent, const juce::String& name = "" );

	template<typename T>
	void getChildrenOfClass ( juce::Component* parent, std::vector<T*>& comps )
	{
		// Loop over all children recursivly and build a vector of components of class T
		for ( auto comp : parent->getChildren () )
		{
			if ( auto tComp = dynamic_cast<T*> ( comp ) )
				comps.push_back ( tComp );

			getChildrenOfClass<T> ( comp, comps );
		}
	}

	template<typename T>
	T* findComponent ( const juce::String& name, const std::unordered_map<juce::String, juce::Component*>& compMap )
	{
		auto	it = compMap.find ( name );

		// Component not found
		jassert ( it != compMap.end () );

		return dynamic_cast<T*> ( it->second );
	}

	struct imageHint
	{
		juce::String	name;
		juce::String	extension;
		int8_t			borderColor = -1;
		bool			firstLuma = false;
		bool			isThumbnail = false;
	};

	imageHint hintFromFilename ( const juce::String& in );
	juce::String filenameFromHint ( const imageHint& hint );
}

namespace paths
{
	juce::File getDataRoot ( juce::String path = "" );
	juce::File getSearchtermsPath ();
	juce::File getPlaylistsPath ();
	juce::File getHistoryPath ();
	juce::File getUserTunesPath ();
}

namespace url
{
	constexpr auto	updateInfo = "https://ultrasid.com/updates.json";
}
