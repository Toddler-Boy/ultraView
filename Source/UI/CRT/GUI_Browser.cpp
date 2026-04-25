#include "GUI_Browser.h"

#include "Globals/constants.h"

#include <regex>

//-----------------------------------------------------------------------------

GUI_Browser::GUI_Browser ()
	: juce::Component ( "browser" )
	, juce::Thread ( "Browser loading thread" )
{
	searchBar.enableSearchHistory ( false );

	searchBar.onTextChange = [ this ]
	{
		searchString = searchBar.getTextEditor ().getText ().toLowerCase ();
		if ( searchString.isEmpty () )
		{
			listbox.setRowData ( browserEntryPtrs );

			info.setText ( "Showing all " + juce::String ( browserEntryPtrs.size () ) + " entries." );
		}
		else
		{
			std::vector<browserEntry*>	filteredEntries;
			filteredEntries.reserve ( browserEntryPtrs.size () );

			const auto	query = normalizeString ( searchString.toLowerCase ().toStdString () );

			auto containsAllWords = [ &query ] ( const std::string& target )
			{
				std::stringstream	ss ( query );
				std::string			word;

				while ( ss >> word )
					if ( ! target.contains ( word ) )
						return false;

				return true;
			};

			for ( auto& entry : browserEntryPtrs )
				if ( containsAllWords ( entry->normalized ) )
					filteredEntries.emplace_back ( entry );

			listbox.setRowData ( filteredEntries );

			if ( filteredEntries.empty () )
				info.setText ( "No matches found." );
			else
				info.setText ( "Found " + juce::String ( filteredEntries.size () ) + " of " + juce::String ( browserEntryPtrs.size () ) + " matches." );
		}
	};

	info.setName ( "info" );
	background.addAndMakeVisible ( searchBar );
	background.addAndMakeVisible ( info );
	background.addAndMakeVisible ( listbox );

	addAndMakeVisible ( background );

	listbox.addHeaderColumn ( UI::columnId::name, true );
	listbox.getHeader ().setSortColumnId ( UI::columnId::name, true );

	startThread ( juce::Thread::Priority::low );
}
//-----------------------------------------------------------------------------

GUI_Browser::~GUI_Browser ()
{
	stopThread ( -1 );
}
//-----------------------------------------------------------------------------

namespace
{

int romanToInt ( std::string s )
{
	std::map<char, int>	m = { {'i',1},{'v',5},{'x',10},{'l',50},{'c',100},{'d',500},{'m',1000} };
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

}
//-----------------------------------------------------------------------------

std::string GUI_Browser::normalizeString ( const std::string& input )
{
	auto	output = input;

	// Replace Roman numerals with their decimal equivalents
	{
		// Regex for Roman numerals as standalone words
		std::regex	romanregex ( R"((\s)([ivxlcdm]+)(?=$|[\s]))" );

		auto words_begin = std::sregex_iterator ( input.begin (), input.end (), romanregex );
		auto words_end = std::sregex_iterator ();

		auto	offset = 0;

		for ( auto i = words_begin; i != words_end; ++i )
		{
			const auto&	match = *i;

			auto	prefix = match[ 1 ].str ();
			auto	romanPart = match[ 2 ].str ();

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

void GUI_Browser::run ()
{
	browserEntries.clear ();
	browserEntries.reserve ( 10'000 );
	browserEntryPtrs.clear ();
	browserEntryPtrs.reserve ( browserEntries.capacity () );

	auto	files = juce::Array<juce::File> ();

	juce::Array<juce::File> directoriesToScan;
	directoriesToScan.add ( juce::File ( "Z:/Data/C64 games" ) );

	auto sendToListBox = [ this ]
	{
		if ( browserEntries.empty () )
			return;

		browserEntryPtrs.resize ( browserEntries.size () );

		for ( auto i = 0; auto& entry : browserEntries )
			browserEntryPtrs[ i++ ] = &entry;

		juce::MessageManager::callSync ( [ this ]
		{
			listbox.setRowData ( browserEntryPtrs );
			info.setText ( juce::String ( listbox.getNumRows () ) + " entries loaded" );
		} );
	};

	while ( directoriesToScan.size () > 0 )
	{
		if ( threadShouldExit () )
			return;

		auto	currentDir = directoriesToScan.removeAndReturn ( 0 );

		for ( const auto& entry : juce::RangedDirectoryIterator ( currentDir, false, "*", juce::File::findFilesAndDirectories, juce::File::FollowSymlinks::noCycles ) )
		{
			if ( threadShouldExit () )
				return;

			const auto	file = entry.getFile ();

			if ( entry.isDirectory () )
			{
				Z_LOG ( "Scanning directory: " + file.getFullPathName () );

				const auto	name = file.getFileName ().toLowerCase ();
				if ( name == "images" || name == "docs" || name == "dumps" || name == "tapes" )
					continue;

				directoriesToScan.add ( file );
			}
			else
			{
				const auto	ext = file.getFileExtension ().toLowerCase ();

				if ( ext == ".crt" || ext == ".prg" )
				{
					auto	name = file.getFileNameWithoutExtension ();
					const auto	hasJ1 = name.containsIgnoreCase ( "(j1)" );

					name = name.upToFirstOccurrenceOf ( "(", false, false ).trimEnd ();
					name = name.upToFirstOccurrenceOf ( "[", false, false ).trimEnd ();

					browserEntries.push_back (
						{
							ext == ".crt" ? 0 : 1,
							name + ( hasJ1 ? " (J1)" : "" ),
							file.getFullPathName (),
							normalizeString ( name.toLowerCase ().toStdString () )
						} );

					if ( threadShouldExit () )
						return;

					sendToListBox ();
				}
			}
		}
	}

	if ( threadShouldExit () )
		return;

	sendToListBox ();
}
//-----------------------------------------------------------------------------
