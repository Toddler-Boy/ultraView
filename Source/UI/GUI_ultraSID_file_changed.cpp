#include "GUI_ultraSID.h"

//-----------------------------------------------------------------------------

void GUI_ultraSID::fileChanged ( const juce::File& file, gin::FileSystemWatcher::FileSystemEvent event )
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

		// Screenshots
		if ( parent.startsWithIgnoreCase ( "Screenshots/" ) )
		{
			if ( ! file.hasFileExtension ( ".png" ) )
				return;

			auto	filename = parent.fromFirstOccurrenceOf ( "/", false, false ).toStdString ();
			auto	updateCRT = true;

			if ( event == gin::FileSystemWatcher::fileCreated || event == gin::FileSystemWatcher::fileRenamedNewName )
			{
				screenshots->addScreenshot ( filename );
			}
			else if ( event == gin::FileSystemWatcher::fileDeleted || event == gin::FileSystemWatcher::fileRenamedOldName )
			{
				screenshots->removeScreenshot ( filename );
				filename = "";

				if ( event == gin::FileSystemWatcher::fileRenamedOldName )
					updateCRT = false;
			}
			else if ( event == gin::FileSystemWatcher::fileUpdated )
			{
				screenshots->removeScreenshot ( filename );
				screenshots->addScreenshot ( filename );
			}

			// In case the artwork is currently visible in the browser
			mainScreen.pages.repaint ();

			// Update footer thumbnail
			thumbnailCache->removeCacheEntry ( lastFilename );
			updateFooterThumbnail ( lastFilename, player.isNTSC () );

			// Set CRT page to new artwork
			if ( updateCRT )
				mainScreen.pages.crtPage.loadGameArtwork ( lastFilename, filename );

			return;
		}

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
			if ( parent.endsWithIgnoreCase ( ".ini" ) )
				mainScreen.pages.crtPage.reloadOverlayProfile ();

			return;
		}

		// UI strings
		if ( parent.startsWithIgnoreCase ( "UI/strings/" ) )
		{
			if ( event != gin::FileSystemWatcher::fileUpdated )
				return;

			strings->load ();
			repaint ();
			mainScreen.footer.volume.qualitySelector.repaint ();
		}

		// UI icons
		if ( parent.equalsIgnoreCase ( "UI/icons.yml" ) )
		{
			if ( event != gin::FileSystemWatcher::fileUpdated )
				return;

			icons->load ();
			repaint ();
			mainScreen.footer.volume.qualitySelector.repaint ();
		}

		// Data files
		if ( parent.startsWithIgnoreCase ( "Data/" ) )
		{
			if ( event != gin::FileSystemWatcher::fileUpdated )
				return;

			if ( file.hasFileExtension ( ".csv" ) )
				loadSIDPlayerProfilesAndOverrides ();
			else if ( file.getFileName () == "STIL-overrides.txt" )
				setHVSCRoot ();

			return;
		}

		return;
	}

	//
	// Change happened to user-data folder
	//
	if ( file.isAChildOf ( userRoot ) )
	{
		auto	parent = file.getRelativePathFrom ( userRoot ).replaceCharacter ( '\\', '/' );

		// Tunes
		if ( parent.startsWithIgnoreCase ( "Tunes/" ) )
		{
			if ( ! file.hasFileExtension ( ".sid" ) )
				return;

			if ( event == gin::FileSystemWatcher::fileCreated || event == gin::FileSystemWatcher::fileRenamedNewName )
			{
				userDatabase->addUserTune ( file );
			}
			else if ( event == gin::FileSystemWatcher::fileDeleted || event == gin::FileSystemWatcher::fileRenamedOldName )
			{
				userDatabase->removeUserTune ( file );
			}
			else if ( event == gin::FileSystemWatcher::fileUpdated )
			{
				userDatabase->removeUserTune ( file );
				userDatabase->addUserTune ( file );
			}

			mainScreen.pages.setUserDatabase ( userDatabase->getAllEntries () );
			mainScreen.pages.repaint ();
			return;
		}

		return;
	}
}
//-----------------------------------------------------------------------------
