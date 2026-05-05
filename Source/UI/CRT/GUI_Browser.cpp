#include "GUI_Browser.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_Browser::GUI_Browser ()
	: juce::Component ( "browser" )
	, juce::Thread ( "Browser loading thread" )
{
	addAndMakeVisible ( badge );

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

			const auto	query = helpers::normalizeGamesString ( searchString.toLowerCase ().toStdString () );

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

	curPath.setName ( "curPath" );
	changePath.bckAlpha[ 1 ] = 0.1f;
	changePath.margin = 6.0f;

	const auto	curPathStr = settings->get<juce::String> ( "Paths/apps" );
	if ( curPathStr.isEmpty () )
	{
		curPath.setText ( "No path set" );
		curPath.setColor ( UI::colors::textMuted );
	}
	else
	{
		curPath.setText ( curPathStr );
	}

	background.addAndMakeVisible ( curPath );
	background.addAndMakeVisible ( changePath );

	addAndMakeVisible ( background );

	listbox.addHeaderColumn ( UI::columnId::name, true );
	listbox.getHeader ().setSortColumnId ( UI::columnId::name, true );

	if ( curPathStr.isNotEmpty () )
	{
		info.setText ( "Scanning directory..." );
		startThread ( juce::Thread::Priority::low );
	}

	changePath.onClick = [ this ]
	{
		const auto	curPathStr = settings->get<juce::String> ( "Paths/apps" );
		lastBrowsedDir = juce::File ( curPathStr );

		directoryChooser = std::make_unique<juce::FileChooser> ( "Select a directory to scan for games", lastBrowsedDir, "*.crt;*.prg" );
		directoryChooser->launchAsync ( juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories, [ this ] ( const juce::FileChooser& chooser )
		{
			const auto	results = chooser.getResults ();
			if ( results.size () > 0 )
			{
				const auto	selectedDir = results[ 0 ];

				settings->set ( "Paths/apps", selectedDir.getFullPathName () );

				curPath.setText ( selectedDir.getFullPathName () );
				curPath.setColor ( UI::colors::text );

				stopThread ( -1 );
				startThread ( juce::Thread::Priority::low );
			}
		} );
	};
}
//-----------------------------------------------------------------------------

GUI_Browser::~GUI_Browser ()
{
	stopThread ( -1 );
}
//-----------------------------------------------------------------------------

void GUI_Browser::refreshBrowserEntries ()
{
	if ( isThreadRunning () )
		return;

	info.setText ( "Found " + juce::String ( browserEntryPtrs.size () ) + " entries" );
	listbox.setRowData ( browserEntryPtrs );
}
//-----------------------------------------------------------------------------

void GUI_Browser::run ()
{
	browserEntries.clear ();
	browserEntryPtrs.clear ();

	auto	files = juce::Array<juce::File> ();

	juce::Array<juce::File> directoriesToScan;

	const auto	curPathStr = settings->get<juce::String> ( "Paths/apps" );
	directoriesToScan.add ( juce::File ( curPathStr ) );

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
				const auto	name = file.getFileName ().toLowerCase ();
				if ( name == "images" || name == "docs" || name == "dumps" || name == "tapes" )
					continue;

				Z_INFO ( "Scanning directory: " + file.getFullPathName () );

				directoriesToScan.add ( file );
			}
			else
			{
				const auto	ext = file.getFileExtension ().toLowerCase ();

				if ( ext == ".crt" || ext == ".prg" )
				{
					auto	name = file.getFileNameWithoutExtension ();

					name = name.upToFirstOccurrenceOf ( "(by ", false, false ).trimEnd ();
					name = name.upToFirstOccurrenceOf ( "[", false, false ).trimEnd ();

					browserEntries.push_back (
						{
							ext == ".crt" ? 0 : 1,
							file.getParentDirectory ().getFileName ().containsIgnoreCase ( "official" ),
							name,
							file.getFullPathName (),
							helpers::normalizeGamesString ( name.toLowerCase ().toStdString () )
						} );

					if ( threadShouldExit () )
						return;
				}
			}
		}
	}

	if ( threadShouldExit () )
		return;

	browserEntryPtrs.reserve ( browserEntries.size () );
	for ( auto& entry : browserEntries )
		browserEntryPtrs.emplace_back (	&entry );

	UI::sendGlobalMessage ( "browser scan-finished" );
}
//-----------------------------------------------------------------------------
