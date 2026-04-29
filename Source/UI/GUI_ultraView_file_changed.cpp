#include "GUI_ultraView.h"

//-----------------------------------------------------------------------------

void GUI_ultraView::fileChanged ( const juce::File& file, gin::FileSystemWatcher::FileSystemEvent event )
{
	//	Z_INFO ( file.getFullPathName () );
	//	Z_INFO ( juce::String ( int ( event ) ) );

	if ( file.isDirectory () )
		return;

	//
	// Change happened inside our data-folder
	//
	if ( file.isAChildOf ( dataRoot ) )
	{
		auto	parent = file.getRelativePathFrom ( dataRoot ).replaceCharacter ( '\\', '/' );

		// Theme
		if ( parent.startsWithIgnoreCase ( "Themes/" ) )
		{
			if ( event != gin::FileSystemWatcher::fileUpdated )
				return;

			// Reload theme
			auto	themeName = preferences->get<juce::String> ( "UI", "theme" );
			auto	themeFile = theme->getRoot ().getChildFile ( themeName + ".ini" );

			if ( file == themeFile )
				loadTheme ();

			return;
		}

		// Overlays
		if ( parent.startsWithIgnoreCase ( "Overlays/" ) )
		{
			if ( event != gin::FileSystemWatcher::fileUpdated )
				return;

			// Reload monitor profile
			if ( parent.endsWithIgnoreCase ( ".yml" ) )
				mainScreen.crt.reloadOverlayProfile ();

			return;
		}

		// UI strings
		if ( parent.startsWithIgnoreCase ( "UI/strings/" ) )
		{
			if ( event != gin::FileSystemWatcher::fileUpdated )
				return;

			strings->load ();
			sendLookAndFeelChange ();
		}

		// UI icons
		if ( parent.equalsIgnoreCase ( "UI/icons.yml" ) )
		{
			if ( event != gin::FileSystemWatcher::fileUpdated )
				return;

			icons->load ();
			sendLookAndFeelChange ();
		}

		// Games database
		if ( parent.equalsIgnoreCase ( "Data/Games.csv" ) )
		{
			if ( event != gin::FileSystemWatcher::fileUpdated )
				return;

			loadGamesDatabase ();
		}

		return;
	}
}
//-----------------------------------------------------------------------------
