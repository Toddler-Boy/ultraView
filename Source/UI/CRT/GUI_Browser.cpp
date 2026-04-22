#include "GUI_Browser.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_Browser::GUI_Browser ()
	: juce::Component ( "browser" )
	, juce::Thread ( "Browser loading thread" )
{
	listbox.setName ( "listbox" );
	background.addAndMakeVisible ( listbox );

	addAndMakeVisible ( background );

	listbox.addHeaderColumn ( UI::columnId::name, true );

	startThread ( juce::Thread::Priority::low );
}
//-----------------------------------------------------------------------------

GUI_Browser::~GUI_Browser ()
{
	stopThread ( -1 );
}
//-----------------------------------------------------------------------------

void GUI_Browser::run ()
{
	browserEntries.clear ();

	auto	files = juce::Array<juce::File> ();

	auto	fileCount = 0;

	juce::Array<juce::File> directoriesToScan;
	directoriesToScan.add ( juce::File ( "Z:/Data/C64 games" ) );

	while ( directoriesToScan.size () > 0 )
	{
		if ( threadShouldExit () )
			return;

		auto	currentDir = directoriesToScan.removeAndReturn ( 0 );
		auto	iter = juce::DirectoryIterator ( currentDir, false, "*", juce::File::findFilesAndDirectories, juce::File::FollowSymlinks::noCycles );

		while ( iter.next () )
		{
			if ( threadShouldExit () )
				return;

			if ( auto file = iter.getFile (); file.isDirectory () )
			{
				const auto	name = file.getFileName ().toLowerCase ();
				if ( name == "images" || name == "docs" || name == "dumps" )
					continue;

				directoriesToScan.add ( file );
			}
			else
			{
				const auto	ext = file.getFileExtension ().toLowerCase ();

				if ( ext == ".crt" || ext == ".prg" )
				{
					browserEntries.push_back (
						{
							ext == ".crt" ? 0 : 1,
							file.getFileNameWithoutExtension (),
							helpers::strToLower ( file.getFileNameWithoutExtension ().toStdString () ),
							file.getFullPathName ()
						} );

					if ( ++fileCount % 100 == 0 )
					{
						if ( threadShouldExit () )
							return;

						juce::MessageManager::callAsync ( [ this ]
						{
							auto	data = browserEntries;
							listbox.setRowData ( data );
						} );
					}
				}
			}
		}
	}

	if ( fileCount % 100 )
	{
		if ( threadShouldExit () )
			return;

		juce::MessageManager::callAsync ( [ this ]
		{
			auto	data = browserEntries;
			listbox.setRowData ( data );
		} );
	}
}
//-----------------------------------------------------------------------------
