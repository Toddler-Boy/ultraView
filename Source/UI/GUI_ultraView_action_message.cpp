#include "GUI_ultraView.h"

//-----------------------------------------------------------------------------

void GUI_ultraView::actionListenerCallback ( const juce::String& message )
{
	if ( message.isEmpty () )
		return;

	const auto	[ cmd, params ] = helpers::parseActionMessage ( message );

	if ( cmd == "volumeChanged" )
	{
//		updateVolume ();
	}
	else if ( cmd == "settingChanged" )
	{
		// The screen never sleeps (Main.cpp, a C64u session has no PC input)
		// and the theme is fixed, so no ui setting needs a reaction anymore
	}
	else if ( cmd == "restoreState" )
	{
		restoreState ();
	}
	else if ( cmd == "passwordChanged" )
	{
		network.setC64uPassword ( settings->get<juce::String> ( "network/password" ) );

		// Retry right away instead of waiting out the tick
		probeFailures = 0;
		reconnectTick ();
	}
	else if ( cmd == "stream-status" )
	{
		Z_INFO ( "Stream status " << params[ 0 ] << " " << params[ 1 ] );
	}
	else if ( cmd == "c64run" )
	{
		Z_INFO ( "Upload " << params[ 0 ].quoted () );

		juce::MemoryBlock	mb;
		if ( ! juce::File ( params[ 0 ] ).loadFileAsData ( mb ) )
		{
			Z_ERR ( "Failed to load file: " << params[ 0 ].quoted () );
			return;
		}

		const auto	extension = params[ 0 ].fromLastOccurrenceOf ( ".", false, false ).toLowerCase ();
		if ( extension == "crt" )
		{
			const auto	wasEasyFlash = loadedEasyFlash;

			const auto	cartType = juce::ByteOrder::bigEndianShort ( static_cast<const char*>( mb.getData () ) + 22 );
			loadedEasyFlash = cartType == 32;

			if ( wasEasyFlash )
			{
				c64_reboot ();

				juce::Timer::callAfterDelay ( 2000, [ this, mb, filename = params[ 0 ] ] {	c64_run ( "crt", mb, filename );	} );
				return;
			}
		}

		c64_run ( extension, mb, params[ 0 ] );
	}
	else if ( cmd == "c64action" )
	{
		network.put ( "v1/machine:" + params[ 0 ], {} );

		if ( params[ 0 ] == "reboot" || params[ 0 ] == "poweroff" )
			loadedEasyFlash = false;

		if ( params[ 0 ] == "pause" )
			machinePaused = true;
		else if ( params[ 0 ] == "resume" || params[ 0 ] == "reboot" || params[ 0 ] == "poweroff" )
			machinePaused = false;
	}
	else if ( cmd == "browser" )
	{
		if ( params[ 0 ] == "scan-finished" )
			mainScreen.crt.refreshBrowserEntries ();
	}
	else if ( cmd == "showAbout" )
	{
		auto*	top = getTopLevelComponent ();
		auto	snapshot = top->createComponentSnapshot ( top->getLocalBounds () );

		mainScreen.crt.paintIntoSnapshot ( snapshot, *top );
		aboutScreen.setBackground ( std::move ( snapshot ) );

		mainScreen.setVisible ( false );
		aboutScreen.setVisible ( true );
	}
	else if ( cmd == "closeAbout" )
	{
		aboutScreen.setVisible ( false );
		aboutScreen.setBackground ( nullptr );

		mainScreen.setVisible ( true );
	}
	else if ( cmd == "showShortcuts" )
	{
		// The shortcut key toggles: showing while shown closes
		if ( shortcutsScreen.isVisible () )
			return actionListenerCallback ( "closeShortcuts" );

		auto*	top = getTopLevelComponent ();
		auto	snapshot = top->createComponentSnapshot ( top->getLocalBounds () );

		mainScreen.crt.paintIntoSnapshot ( snapshot, *top );
		shortcutsScreen.setBackground ( std::move ( snapshot ) );

		mainScreen.setVisible ( false );
		shortcutsScreen.setVisible ( true );
	}
	else if ( cmd == "closeShortcuts" )
	{
		shortcutsScreen.setVisible ( false );
		shortcutsScreen.setBackground ( nullptr );

		mainScreen.setVisible ( true );
	}
	// The keyboard verbs (Data/UI/shortcuts.csv binds keys to these)
	else if ( cmd == "toggleFullscreen" )
	{
		if ( mainScreen.crt.isVisible () )
			toggleFullscreen ();
	}
	else if ( cmd == "toggleRasterTime" )
	{
		showRasterTime = ! showRasterTime;
		mainScreen.crt.showRasterTime ( showRasterTime );
	}
	else if ( cmd == "focusSearch" )
	{
		mainScreen.crt.focusSearch ();
	}
	else if ( cmd == "toggleBrowser" )
	{
		mainScreen.crt.showBrowser ( ! mainScreen.crt.isBrowserVisible () );
	}
	else if ( cmd == "toggleSettings" )
	{
		mainScreen.crt.showSettings ( ! mainScreen.crt.areSettingsVisible () );
	}
	else if ( cmd == "togglePause" )
	{
		actionListenerCallback ( machinePaused ? "c64action resume" : "c64action pause" );
	}
	else if ( cmd == "resetMachine" )
	{
		actionListenerCallback ( "c64action reboot" );
	}
	else if ( cmd == "toggleMenu" )
	{
		actionListenerCallback ( "c64action menu_button" );
	}
	else
	{
		Z_ERR ( "Unknown action: " << message );
	}
}
//-----------------------------------------------------------------------------
