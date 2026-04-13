#include "GUI_ultraSID.h"

#include "UI/SidebarRight/GUI_SidebarRight.h"

//-----------------------------------------------------------------------------

void GUI_ultraSID::actionListenerCallback ( const juce::String& message )
{
	if ( message.isEmpty () )
		return;

	const auto	[ cmd, params ] = helpers::parseActionMessage ( message );

	if ( cmd == "setLocation" )
	{
		if ( params[ 0 ] == "hvsc" )
			setHVSCRoot ();
		else if ( params[ 0 ] == "user" )
			setUserRoot ();

		const auto	isReady = areRootsValid ();

		mainScreen.sidebarLeft.mainMenu.enableMenus ( isReady );
		if ( ! onboardingScreen.isVisible () && ! isReady )
			showPage ( "settings" );
	}
	else if ( cmd == "mainMenu" )
	{
		lastPage = params[ 0 ].toStdString ();
		showPage ( lastPage );
		saveState ();
	}
	else if ( cmd == "showPage" )
	{
		showPage ( params[ 0 ].toStdString () );
	}
	else if ( cmd == "showPlaylist" )
	{
		showPlaylist ( params[ 0 ].toStdString () );
	}
	else if ( cmd == "showSearch" )
	{
		showPage ( "search" );
		unfocusAllComponents ();
	}
	else if ( cmd == "volumeChanged" )
	{
		updateVolume ();
	}
	else if ( cmd == "loadTune" )
	{
		loadTune ( params[ 0 ], params[ 1 ].getIntValue (), params[ 2 ], params[ 3 ].getIntValue () );
	}
	else if ( cmd == "playSubtune" )
	{
		playSubtune ( params[ 0 ].getIntValue () );
	}
	else if ( cmd == "transport" )
	{
		if ( params[ 0 ] == "play" )
		{
			togglePause ();
			updateTransportButtons ();
		}
		else if ( params[ 0 ] == "prev" || params[ 0 ] == "next" )
		{
			nextPreviousPlaylistItem ( params[ 0 ] == "prev" ? -1 : 1 );
			updateTransportButtons ();
			auto& pl = *mainScreen.pages.getCurrentPlaylist ();
			pl.selectRow ( playlistPlayPosition );
		}
	}
	else if ( cmd == "setCRTpage" )
	{
		mainScreen.pages.crtPage.loadGameArtwork ( params[ 0 ].getIntValue () );
	}
	else if ( cmd == "settingChanged" )
	{
		if ( params[ 0 ] == "ui" && params[ 1 ] == "allow-screensaver" )
			juce::Desktop::setScreenSaverEnabled ( preferences->get<bool> ( params[ 0 ], params[ 1 ] ) );
		else if ( params[ 0 ] == "fx" )
			updateFX ();
		else if ( params[ 0 ] == "player" && params[ 1 ] == "normalize" )
			player.setReplayGain ( preferences->get<bool> ( params[ 0 ], params[ 1 ] ) );
	}
	else if ( cmd == "playPlaylist" )
	{
		auto	newPlaylist = mainScreen.pages.getPlaylistItems ( params[ 0 ] );
		if ( ! newPlaylist )
			return;

		mainScreen.pages.setCurrentPlaylist ( newPlaylist );

		auto	item = newPlaylist->getItem ( 0 );
		const auto	subTune = newPlaylist->getSubtune ( 0 );

		loadTune ( item->file, subTune, "playlist", 0 );
	}
	else if ( cmd == "exportTune" )
	{
		for ( auto& item : juce::StringArray::fromTokens ( params[ 0 ], "|", "" ) )
			mainScreen.pages.exportPage.addItem ( getFullFilename ( item.upToLastOccurrenceOf ( ",", false, false ) ), item.toStdString () );
	}
	else if ( cmd == "goToFolder" )
	{
		mainScreen.pages.setSearch ( params[ 0 ], true );
	}
	else if ( cmd == "tagsToggled" )
	{
		mainScreen.pages.repaint ();
	}
	else if ( cmd == "restoreState" )
	{
		restoreState ();
	}
	else if ( cmd == "assignBorderColor" )
	{
		assignBorderColor ( params[ 0 ].getIntValue () );
	}
	else if ( cmd == "removeBorderColor" )
	{
		assignBorderColor ( -1 );
	}
	else if ( cmd == "toggleFirstLumaAll" )
	{
		toggleFirstLumaAll ();
	}
	else if ( cmd == "toggleFirstLuma" )
	{
		toggleFirstLuma ();
	}
	else if ( cmd == "toggleThumbnail" )
	{
		toggleThumbnail ();
	}
	else if ( cmd == "deleteImage" )
	{
		deleteImage ();
	}
	else if ( cmd == "likeChanged" )
	{
		mainScreen.sidebarRight.likeChanged ();
	}
	else if ( cmd == "hvscCheck" )
	{
		if ( installState->hvsc.versionAvailable <= 0 )
			return;

		if ( updateHVSCScreen.isUpdating () )
		{
			if ( installState->hvsc.needsUpdate () )
			{
				downloadHVSC_update ();
				return;
			}

			sendActionMessage ( "databaseCheck" );
			return;
		}

		if ( installState->hvsc.needsFullInstall () )
		{
			downloadHVSC_full ();
			return;
		}

		// HVSC installed version is up-to-date, check database version
		if ( ! installState->hvsc.needsUpdate () )
		{
			sendActionMessage ( "databaseCheck" );
			return;
		}
	}
	else if ( cmd == "databaseCheck" )
	{
		if ( installState->database.versionAvailable <= 0 )
			return;

		// Database installed version is up-to-date
		if ( ! installState->database.needsUpdate () )
		{
			if ( isOnboardingOrUpdating () )
				showPage ( "search" );

			return;
		}

		downloadDatabase_full ();
	}
	else if ( cmd == "download" )
	{
		if ( params[ 0 ] == "info" )
		{
			if ( installState->database.versionAvailable <= 0 )
				downloadInfo ();
			else
				checkHVSCStatus ();
		}
		else if ( params[ 0 ] == "HVSC" )
		{
			if		( params[ 1 ] == "update" )			downloadHVSC_update ();
			else if ( params[ 1 ] == "full" )			downloadHVSC_full ();
			else if ( params[ 1 ] == "cancel" )			stopFullHVSCInstall ();
			else if ( params[ 1 ] == "cancelUpdate" )	stopHVSCupdate ();
		}
		else
		{
			Z_ERR ( "Unknown download: " << message );
		}
	}
	else if ( cmd == "addScreenshots" )
		addScreenshots ( params );
	else if ( cmd == "downloadScreenshot" )
		downloadScreenshot ( params[ 0 ].trim () );
	else if ( cmd == "downloadCover" )
		downloadCoverImage ( params[ 0 ], params[ 1 ] );
	else if ( cmd == "playlist" )
	{
		if ( params[ 0 ] == "update" || params[ 0 ] == "addTo" )
		{
			mainScreen.pages.playlist.updatePlaylist ( params[ 1 ] );
			mainScreen.pages.playlistGrid.selectPlaylist ( params[ 1 ] );

			mainScreen.sidebarLeft.miniPlaylists.updateGridItemByName ( params[ 1 ] );
			mainScreen.sidebarLeft.miniPlaylists.selectPlaylist ( params[ 1 ] );
		}
		else if ( params[ 0 ] == "new" )
		{
			mainScreen.pages.playlist.addPlaylist ( params[ 1 ] );

			mainScreen.pages.playlistGrid.addPlaylist ( params[ 1 ] );
			mainScreen.pages.playlistGrid.selectPlaylist ( params[ 1 ] );

			mainScreen.sidebarLeft.miniPlaylists.addPlaylist ( params[ 1 ] );
			mainScreen.sidebarLeft.miniPlaylists.selectPlaylist ( params[ 1 ] );
		}
		else if ( params[ 0 ] == "deleteList" )
		{
			mainScreen.pages.playlist.deletePlaylist ( params[ 1 ] );

			mainScreen.pages.playlistGrid.removePlaylist ( params[ 1 ] );
			mainScreen.sidebarLeft.miniPlaylists.removePlaylist ( params[ 1 ] );
		}
		else if ( params[ 0 ] == "renamed" )
		{
			mainScreen.sidebarLeft.miniPlaylists.removePlaylist ( params[ 1 ] );
			mainScreen.sidebarLeft.miniPlaylists.addPlaylist ( params[ 2 ] );
			mainScreen.sidebarLeft.miniPlaylists.updateContent ();
			mainScreen.sidebarLeft.miniPlaylists.selectPlaylist ( params[ 2 ] );
		}
		else if ( params[ 0 ] == "updateInfo" )
		{
			mainScreen.pages.playlist.updateInfo ();
			mainScreen.pages.playlistGrid.updateGridItemByName ( params[ 1 ] );
			mainScreen.sidebarLeft.miniPlaylists.updateGridItemByName ( params[ 1 ] );
		}
	}
	else
	{
		Z_ERR ( "Unknown action: " << message );
	}
}
//-----------------------------------------------------------------------------
