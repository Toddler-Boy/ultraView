#include "GUI_ultraView.h"

#include "ultra-shared/Config/DataSource.h"

//-----------------------------------------------------------------------------

void GUI_ultraView::fileChanged ( const juce::File& file, gin::FileSystemWatcher::FileSystemEvent event )
{
	if ( file.isDirectory () )
		return;

	//
	// Change happened inside the naked factory data, under either root (only
	// watched in developer mode; in pak mode both roots stay invalid and
	// nothing matches)
	//
	const auto	factoryRoot = file.isAChildOf ( dataRoot ) ? dataRoot : sharedDataRoot;

	if ( file.isAChildOf ( factoryRoot ) )
	{
		auto	parent = file.getRelativePathFrom ( factoryRoot ).replaceCharacter ( '\\', '/' );

		// Factory theme
		if ( parent.startsWithIgnoreCase ( "UI/themes/" ) )
		{
			if ( event != gin::FileSystemWatcher::fileUpdated )
				return;

			if ( file == theme->resolve ( "$DATA$/default" ) )
				loadTheme ();

			return;
		}

		// Overlay profile tweaks (textures and shaders reload through lime's
		// own watcher in the naked layout)
		if ( parent.startsWithIgnoreCase ( "CRTEmulation/" ) )
		{
			if ( event == gin::FileSystemWatcher::fileUpdated && parent.endsWithIgnoreCase ( ".yml" ) )
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
			return;
		}

		// UI icons
		if ( parent.equalsIgnoreCase ( "UI/icons.yml" ) )
		{
			if ( event != gin::FileSystemWatcher::fileUpdated )
				return;

			icons->load ();
			sendLookAndFeelChange ();
			return;
		}

		// About content
		if ( parent.equalsIgnoreCase ( "UI/about.txt" ) )
		{
			if ( event != gin::FileSystemWatcher::fileUpdated )
				return;

			aboutScreen.loadContent ();
			return;
		}

		// Games database
		if ( parent.equalsIgnoreCase ( "Data/Games.csv" ) )
		{
			if ( event != gin::FileSystemWatcher::fileUpdated )
				return;

			loadGamesDatabase ();
			return;
		}

		return;
	}

	//
	// Change happened inside the user data folder
	//
	if ( file.isAChildOf ( userRoot ) )
	{
		auto	parent = file.getRelativePathFrom ( userRoot ).replaceCharacter ( '\\', '/' );

		// User themes: any event reloads, so deleting the active user theme
		// reverts to the code defaults live
		if ( parent.startsWithIgnoreCase ( "Themes/" ) )
		{
			loadTheme ();
			return;
		}

		// User CRT presets are plain files read directly, no pak/lime coupling
		if ( parent.startsWithIgnoreCase ( "CRT Presets/" ) )
		{
			mainScreen.crt.userCRTPresetsChanged ();
			return;
		}

		// User CRT content merges over the factory set through the content
		// loader; the CRT page maps the change back to its nominal path
		if ( parent.startsWithIgnoreCase ( "Overlays/" ) || parent.startsWithIgnoreCase ( "CRT Masks/" ) )
		{
			mainScreen.crt.userCRTContentChanged ( parent, event );
			return;
		}

		return;
	}
}
//-----------------------------------------------------------------------------
