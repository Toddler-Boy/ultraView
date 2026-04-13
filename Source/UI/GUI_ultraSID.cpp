#include "GUI_ultraSID.h"

#include "SID_LookAndFeel.h"

#include "Globals/Tags.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_ultraSID::GUI_ultraSID ()
	: juce::Thread ( "Archive extraction thread" )
	, mainScreen ( deviceManager )
{
	UI::setActionBroadCaster ( this );

	setName ( "ultraSID" );

	downloader.setProgressInterval ( 1000 / 30 );

	mainScreen.layout.setConstant ( "fullscreen", 0 );
	mainScreen.layout.setConstant ( "windowed", 1 );

	setWantsKeyboardFocus ( true );

	tooltipWindow->setOpaque ( false );

	// Detect double-clicks on CRT images
	addMouseListener ( this, true );

	// A little trick to allow child components to send actions to this component
	addActionListener ( this );

	// Add listener to main menu
	mainScreen.sidebarLeft.mainMenu.addActionListener ( this );

	inputMeter[ 0 ].setName ( "inL" );
	inputMeter[ 1 ].setName ( "inR" );
	inputMeter[ 0 ].setColour ( gin::LevelMeter::lineColourId, juce::Colours::transparentBlack );
	inputMeter[ 1 ].setColour ( gin::LevelMeter::lineColourId, juce::Colours::transparentBlack );

	outputMeter[ 0 ].setName ( "outL" );
	outputMeter[ 1 ].setName ( "outR" );
	outputMeter[ 0 ].setColour ( gin::LevelMeter::lineColourId, juce::Colours::transparentBlack );
	outputMeter[ 1 ].setColour ( gin::LevelMeter::lineColourId, juce::Colours::transparentBlack );

	mainScreen.footer.volume.addChildComponent ( inputMeter[ 0 ] );
	mainScreen.footer.volume.addChildComponent ( inputMeter[ 1 ] );
	mainScreen.footer.volume.addChildComponent ( outputMeter[ 0 ] );
	mainScreen.footer.volume.addChildComponent ( outputMeter[ 1 ] );

	lastPage = "search";
	addAndMakeVisible ( mainScreen );
	addChildComponent ( onboardingScreen );
	addChildComponent ( updateHVSCScreen );

	updateTransportButtons ();

	folderWatcher.coalesceEvents ( 50 );
	folderWatcher.addListener ( this );
	theme->setTargetLAF ( getLookAndFeel () );

	// Handle transport seek
	mainScreen.footer.transport.seek = [ this ] ( int newPosition )
	{
		player.seek ( newPosition );
	};

	UI::sendGlobalMessage ( "download info" );

	setHVSCRoot ();
	setDataRoot ();
	setUserRoot ();

	loadTheme ();
	loadSIDPlayerProfilesAndOverrides ();

	// Show BASIC screen
	if ( areRootsValid () )
		mainScreen.pages.crtPage.loadGameArtwork ( "" );

	updateVolume ();

	setAudioChannels ( 0, 2 );

	//
	// Setup C64u UDP receiver callback for live preview
	//
/*	c64uReceiver.setAudioBuffers (c64uBuffer.getWritePointer (0), c64uBuffer.getWritePointer (1));

	c64uReceiver.onAudioChunk = [ this ] ( int index )
	{
		if ( ! preferences->get<bool> ( "stream/enabled" ) )
			return;

		if ( streamFifo.getFreeSpace () >= c64uBuffer.getNumSamples () )
			streamFifo.write ( c64uBuffer );
	};

	c64uReceiver.onStatusChange = [] ( bool receiving )
	{
		if ( receiving )
			juce::Logger::writeToLog ( "[I]C64u UDP receiver started receiving data (audio)" );
		else
			juce::Logger::writeToLog ( "[W]C64u UDP receiver stopped receiving data (audio)" );
	};

	c64uReceiver.start ( "239.0.1.65", 11001 );
*/
}
//-----------------------------------------------------------------------------

GUI_ultraSID::~GUI_ultraSID ()
{
//	c64uReceiver.stop ();

	shutdownAudio ();
	stopThread ( -1 );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::toggleFullscreen ()
{
	const auto	kioskMode = isFullscreen ();

//	showPage ( kioskMode ? lastPage : "crt" );

	kioskMode ? toWindowed () : toFullscreen ();
}
//-----------------------------------------------------------------------------

bool GUI_ultraSID::isFullscreen () const
{
	return static_cast<juce::DocumentWindow*> ( getParentComponent () )->isKioskMode ();
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::toFullscreen ()
{
	mainScreen.layout.setConstant ( "fullscreen", 1 );
	mainScreen.layout.setConstant ( "windowed", 0 );

	auto	parent = static_cast<juce::DocumentWindow*> ( getParentComponent () );

	// Only hide monitor/crt settings if they are open
	if ( ( settingsAreVisible = mainScreen.pages.crtPage.areSettingsVisible () ) )
		mainScreen.pages.crtPage.showSettings ( false );

	parent->parentHierarchyChanged ();

	juce::Desktop::getInstance ().setKioskModeComponent ( parent, false );

	wasFullscreen = true;
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::toWindowed ()
{
	mainScreen.layout.setConstant ( "fullscreen", 0 );
	mainScreen.layout.setConstant ( "windowed", 1 );

	juce::Desktop::getInstance ().setKioskModeComponent ( nullptr, false );

	// Only show monitor/crt settings if they where open when toggling to fullscreen
	if ( settingsAreVisible )
		mainScreen.pages.crtPage.showSettings ( true );

	auto	parent = dynamic_cast<juce::DocumentWindow*> ( getTopLevelComponent () );
	parent->parentHierarchyChanged ();

	wasFullscreen = false;
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::moved ()
{
	mainScreen.footer.volume.updateQualityPosition ();
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::resized ()
{
	layout.setLayout ( { paths::getDataRoot ( "UI/layouts/screens.json" ) } );

	const auto	kioskMode = isFullscreen ();

	if ( wasFullscreen && ! kioskMode )
		toWindowed ();

	if ( ! kioskMode )
		mainScreen.footer.volume.updateQualityPosition ();
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::mouseDoubleClick ( const juce::MouseEvent& evt )
{
	// Double click on "CRT" toggles fullscreen
	if ( evt.eventComponent->getName () == "CRT" )
		toggleFullscreen ();
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::mouseDown ( const juce::MouseEvent& evt )
{
	//
	// Click outside quality-selector hides it
	//

	// Check if quality-selector is even on desktop
	if ( !mainScreen.footer.volume.qualitySelector.isOnDesktop () )
		return;

	// Check if click happened on the quality button
	if ( evt.originalComponent == &mainScreen.footer.volume.quality )
		return;

	// Check if click happened inside the quality-selector bounds
	{
		auto	ne = evt.getEventRelativeTo ( this ).getScreenPosition ();
		auto	b = mainScreen.footer.volume.qualitySelector.getScreenBounds ();
		if ( b.contains ( ne ) )
			return;
	}

	mainScreen.footer.volume.qualitySelector.removeFromDesktop ();
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::parentHierarchyChanged ()
{
	if ( ! findParentComponentOfClass<juce::ResizableWindow> () )
		return;

	std::call_once ( firstStart, [ this ]
	{
		updateFooterThumbnail ( "", false );

		mainScreen.pages.updateGrid ();
		mainScreen.sidebarLeft.miniPlaylists.updateContent ();
	} );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::update ( double time )
{
	// Calculate how much time has passed since last V-blank
	const auto	diff = std::min ( 1.0f, float ( time - lastTimer ) );
	lastTimer = time;

	//
	// Call various update functions
	//
	if ( mainScreen.pages.isShowing () )
	{
		mainScreen.pages.timerUpdate ( diff, player.getCPUCycles () );

		if ( mainScreen.sidebarRight.isShowing () )
		{
			mainScreen.sidebarRight.stil.timerUpdate ( diff );
			mainScreen.sidebarRight.fftLeft.update ();
			mainScreen.sidebarRight.fftRight.update ();
			updateVoices ();
		}
	}

	if ( mainScreen.footer.isShowing () )
		mainScreen.footer.transport.setTime ( player.getTimeMS (), player.getRenderProgressMS () );

	updatePlaylistPosition ();
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::updateVoices ()
{
	if ( ! player.lockDigiBuffers () )
		return;

	const auto	numChips = player.getNumChips ();

	for ( auto chipIndex = 0; chipIndex < numChips; ++chipIndex )
 	{
 		const auto	[ regsPtr, regsIndex ] = player.getSidStatus ( chipIndex );

		mainScreen.sidebarRight.chips.getChipState ( chipIndex ).updateState ( regsPtr, regsIndex );
		mainScreen.sidebarRight.freqLines.updateState ( chipIndex, regsPtr, regsIndex );

		mainScreen.sidebarRight.chips.setDigiData ( chipIndex, player.getDigiBuffer ( chipIndex ) );
	}

	player.unlockDigiBuffers ();
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::updateVolume ()
{
	const auto	volState = mainScreen.footer.volume.getState ();

	// FX
	dspEffects.setFXMode ( std::get<int> ( volState.at ( "quality" ) ) );

	// Volume
	dspEffects.setVolume ( std::get<float> ( volState.at ( "volume" ) ) * 0.01f );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::updateFooterThumbnail ( const std::string& tunename, const bool isNTSC )
{
	mainScreen.footer.info.thumbnail.setMipMap ( thumbnailCache->getThumbnail ( tunename, isNTSC ) );

	updateColors ();
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::updateColors ()
{
	// Set average color as window background
	//const auto	img = footer.info.thumbnail.getImage ();
	//const auto	col = UI::getAverageColor ( img, 0.03f, 1.0f, 0.1f );

	const auto	bgCol = findColour ( UI::colors::window );
	const auto	textCol = findColour ( UI::colors::text );
	UI::setShades ( bgCol, textCol );

	const auto	darkCol = UI::getShade ( 0.1f );
	const auto	bento = bgCol;// UI::getShade ( 1.0f / 16.0f );
	auto&	laf = getLookAndFeel ();

	laf.setColour ( juce::ResizableWindow::backgroundColourId, bgCol );
	laf.setColour ( juce::TextEditor::backgroundColourId, darkCol );
	laf.setColour ( juce::TextButton::buttonColourId, juce::Colours::orangered );

	laf.setColour ( UI::colors::bento, bento );
	laf.setColour ( UI::colors::textMuted, UI::getShade ( 0.5f ) );
	laf.setColour ( UI::colors::accentBright, UI::getColorWithPerceivedBrightness ( findColour ( UI::colors::accent ), 0.6f ) );

	// Set some JUCE colors
	{
		laf.setColour ( juce::TooltipWindow::backgroundColourId, darkCol );

		laf.setColour ( juce::ScrollBar::backgroundColourId, UI::getShade ( 1.0f / 32.0f ) );
		laf.setColour ( juce::ScrollBar::thumbColourId, UI::getShade ( 0.25f ) );
		laf.setColour ( juce::ScrollBar::trackColourId, UI::getShade ( 0.5f ) );

		const auto	comboCol = UI::getShade ( 0.2f );

		laf.setColour ( juce::ComboBox::backgroundColourId, comboCol );
		laf.setColour ( juce::ComboBox::buttonColourId, comboCol );
		laf.setColour ( juce::ComboBox::textColourId, textCol );
		laf.setColour ( juce::ComboBox::arrowColourId, textCol );

		laf.setColour ( juce::PopupMenu::backgroundColourId, darkCol );
		laf.setColour ( juce::PopupMenu::textColourId, textCol );
		laf.setColour ( juce::PopupMenu::highlightedBackgroundColourId, UI::getShade ( 0.2f ) );

		if ( auto p = findParentComponentOfClass<juce::ResizableWindow> () )
			p->setBackgroundColour ( bgCol );
	}

	mainScreen.pages.crtPage.setBackgroundColour ( bgCol );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::showPage ( const std::string& name )
{
	mainScreen.sidebarLeft.mainMenu.updateState ( name );

	// Switch main page
	{
		const static juce::StringArray	mainPages {	"search", "playlists", "history", "crt", "export", "settings" };

		const auto	isOnMainScreen = mainPages.contains ( name );

		mainScreen.setVisible ( isOnMainScreen );
		if ( isOnMainScreen )
		{
			mainScreen.pages.setPage ( name );
			mainScreen.resized ();
		}
	}

	onboardingScreen.setVisible ( name == "onboarding" );
	updateHVSCScreen.setVisible ( name == "updateHVSC" );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::showPlaylist ( const std::string& name )
{
	mainScreen.pages.showPlaylist ( name );
	mainScreen.sidebarLeft.miniPlaylists.selectPlaylist ( name );
	lastPage = "playlists";
	showPage ( "playlists" );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::loadTheme ()
{
	auto	themeName = preferences->get<juce::String> ( "UI", "theme" );

	theme->load ( themeName );
	dynamic_cast<SID_LookAndFeel&> ( getLookAndFeel () ).updateProgressColors ();

	updateColors ();
	sendLookAndFeelChange ();
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::loadSIDPlayerProfilesAndOverrides ()
{
	//
	// Load chip profiles
	//
	if ( auto fcFile = dataRoot.getChildFile ( "Data/chip-profiles.csv" ); fcFile.existsAsFile () )
		player.setChipProfiles ( fcFile.loadFileAsString ().toStdString () );

	//
	// Load audio profiles
	//
	if ( auto fsFile = dataRoot.getChildFile ( "Data/audio-profiles.csv" ); fsFile.existsAsFile () )
		player.setAudioProfiles ( fsFile.loadFileAsString ().toStdString () );

	//
	// Load overrides
	//
	if ( auto fsFile = dataRoot.getChildFile ( "Data/tune-overrides.csv" ); fsFile.existsAsFile () )
	{
		player.setTuneOverrides ( fsFile.loadFileAsString ().toStdString () );
		database->applyOverrides ( player.getAllTuneOverrides () );

		mainScreen.pages.search.repaint ();
	}
}
//-----------------------------------------------------------------------------

namespace
{
	bool allPathsValid ( const juce::StringArray& arr, const juce::File& root )
	{
		for ( const auto& f : arr )
		{
			if ( f.endsWithChar ( '/' ) )
			{
				if ( ! root.getChildFile ( f ).isDirectory () )
					return false;
			}
			else
			{
				if ( ! root.getChildFile ( f ).existsAsFile () )
					return false;
			}
		}

		return true;
	}
}
//-----------------------------------------------------------------------------

bool GUI_ultraSID::isHVSCRootValid () const
{
	// Does folder even exist?
	if ( ! hvscRoot.isDirectory () )
	{
		Z_ERR ( "HVSC root not set or invalid path" );
		return false;
	}

	// Is it complete?
	static const juce::StringArray	arr = {
		"DEMOS/",
		"DOCUMENTS/",
		"GAMES/",
		"MUSICIANS/",
		"DOCUMENTS/Songlengths.md5",
		"DOCUMENTS/STIL.txt",
		"DOCUMENTS/BUGlist.txt",
	};

	const auto	hvscValid = allPathsValid ( arr, hvscRoot );
	if ( ! hvscValid )
		Z_ERR ( "HVSC incomplete (missing folders or files)" );

	return hvscValid;
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::setHVSCRoot ()
{
	// No installed at all
	installState->hvsc.versionInstalled = -1;
	checkHVSCStatus ();

	hvscRoot = settings->get<juce::String> ( "Paths", "hvsc" );
	if ( ! isHVSCRootValid () )
		return;

	// Installed but not loaded yet
	installState->hvsc.versionInstalled = 0;
	checkHVSCStatus ();

	hvscDatabase->setRoot ( hvscRoot.getFullPathName ().toRawUTF8 () );
	installState->hvsc.versionInstalled = hvscDatabase->getHVSCVersion ();

	if ( installState->hvsc.versionInstalled < 0 )
		return;

	//
	// Load HVSC databases (STIL & Bugs) in a background thread
	//
	hvscDatabase->load ( [ this ]
	{
		const auto	hvscError = juce::String ( hvscDatabase->getErrorString () );

		mainScreen.pages.setError ( hvscError );

		checkHVSCStatus ();

		UI::sendGlobalMessage ( "hvscCheck" );
	} );
}
//-----------------------------------------------------------------------------

bool GUI_ultraSID::isDataRootValid () const
{
	if ( ! dataRoot.isDirectory () )
	{
		Z_ERR ( "ultraSID data folder is missing" );
		return false;
	}

	static const juce::StringArray	arr = {
		"CRTEmulation/Shaders/",

		"Data/",
		"Roms/",
		"Tags/",
		"Themes/",

		"UI/",
		"UI/fonts/",
		"UI/layouts/",
		"UI/png/",
		"UI/strings/",
		"UI/svg/",
		"UI/icons.yml",

		"sidid.cfg",
		"sidid.nfo",
		"ultraSID.db",
	};

	const auto	dataValid = allPathsValid ( arr, dataRoot );

	if ( ! dataValid )
	{
		Z_ERR ( "ultraSID data folder is incomplete (missing folders or files) " + dataRoot.getFullPathName () );
		jassertfalse; // This should never happen in a release build, but it's good to catch during development
	}

	return dataValid;
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::setDataRoot ()
{
	dataRoot = paths::getDataRoot ();

	mainScreen.pages.setFileDatabase ( {} );

	if ( ! isDataRootValid () )
		return;

	folderWatcher.removeAllFolders ();

	tags->setRoot ( dataRoot.getChildFile ( "Tags/" ) );

	mainScreen.pages.crtPage.setRoot ( dataRoot );
	screenshots->setRoot ( dataRoot );
	thumbnailCache->setRoot ( dataRoot.getChildFile ( "Screenshots/" ) );
	mainScreen.sidebarRight.stil.setImageRoot ( dataRoot.getChildFile ( "Portraits/" ) );
	theme->setRoot ( dataRoot.getChildFile ( "Themes/" ) );

	loadROMs ();
	loadDatabase ();

	player.loadSidIDConfig ( dataRoot.getChildFile ( "sidid.cfg" ).getFullPathName ().toRawUTF8 () );

	folderWatcher.addFolder ( dataRoot );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::setUserRoot ()
{
	folderWatcher.removeFolder ( userRoot );

	mainScreen.pages.setUserDatabase ( {} );

	userRoot = settings->get<juce::String> ( "Paths", "user" );

	preferences->setRoot ( userRoot );
	mainScreen.footer.volume.restorePreferences ();
	mainScreen.pages.settingsPage.restorePreferences ();
	mainScreen.sidebarRight.stil.restorePreferences ();

	likes->setRoot ( userRoot );

	userDatabase->scanUserTunes ();

	mainScreen.pages.setUserDatabase ( userDatabase->getAllEntries () );

	playlists->findPlaylists ();

	mainScreen.pages.setPlaylists ();

	{
		const auto	playlistNames = juce::SharedResourcePointer<Playlists> ()->getPlaylistNames ();

		mainScreen.sidebarLeft.miniPlaylists.setPlaylists ( playlistNames );
	}

	mainScreen.pages.loadHistory ();
	mainScreen.pages.updateGrid ();

	folderWatcher.addFolder ( userRoot );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::loadDatabase ()
{
	database->load ( dataRoot.getFullPathName ().toRawUTF8 () );
	database->applyOverrides ( player.getAllTuneOverrides () );
	std::tie ( installState->database.versionInstalled, installState->database.versionInstalled_min ) = database->getVersion ();
	checkDatabaseStatus ();

	mainScreen.pages.setFileDatabase ( database->getAllEntries () );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::loadROMs ()
{
	//
	// Load the three ROMs (Kernal, Basic, and Character)
	//
	auto loadRom = [ this ] ( const juce::String& name )
	{
		juce::MemoryBlock	mb;

		if ( auto file = dataRoot.getChildFile ( "Roms/" + name ); file.existsAsFile () )
			file.loadFileAsData ( mb );

		return mb;
	};

	auto	romKernal = loadRom ( "kernal.bin" );
	auto	romBasic = loadRom ( "basic.bin" );
	auto	romCharacter = loadRom ( "character.bin" );

	player.setRoms ( romKernal.getData (), romBasic.getData (), romCharacter.getData () );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::saveState ()
{
	//
	// Save last page
	//
	settings->set ( "UI", "page", juce::String ( lastPage ) );

	//
	// Last viewed CRT image
	//
	settings->set ( "UI", "crt-image", mainScreen.pages.crtPage.getCRTPage () );

	//
	// Playlists
	//
	{
		if ( auto visiblePlaylist = mainScreen.pages.getCurrentVisiblePlaylist () )
		{
			settings->set ( "UI", "playlist", visiblePlaylist->getName () );
			settings->set ( "UI", "playlist-selected", visiblePlaylist->getSelectedRow () );
			settings->set ( "UI", "playlist-pos", visiblePlaylist->getVerticalPosition () );
		}
		else
		{
			settings->set ( "UI", "playlist", std::string {} );
			settings->set ( "UI", "playlist-selected", 0 );
			settings->set ( "UI", "playlist-pos", 0.0 );
		}
	}

	//
	// Search
	//
	{
		auto&	results = mainScreen.pages.getSearchResults ();

		settings->set ( "UI", "search-str", mainScreen.pages.getSearchString () );
		settings->set ( "UI", "search-selected", results.getSelectedRow () );
		settings->set ( "UI", "search-pos", results.getVerticalPosition () );

		auto&	header = results.getHeader ();

		settings->set ( "UI", "search-col", header.getSortColumnId () );
		settings->set ( "UI", "search-forwards", header.isSortedForwards () );
	}
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::restoreState ()
{
	//
	// Restore playlist state
	//
	if ( auto playlistName = settings->get<juce::String> ( "UI", "playlist" ); playlistName.isNotEmpty () )
	{
		showPlaylist ( playlistName.toStdString () );
		if ( auto curPls = mainScreen.pages.getCurrentVisiblePlaylist () )
		{
			curPls->selectRow ( settings->get<int> ( "UI", "playlist-selected" ) );
			curPls->setVerticalPosition ( settings->get<double> ( "UI", "playlist-pos" ) );
		}
	}

	//
	// Restore search state
	//
	{
		auto	searchStr = settings->get<juce::String> ( "UI", "search-str" );
		mainScreen.pages.setSearch ( searchStr, false );

		auto&	results = mainScreen.pages.getSearchResults ();

		auto&	header = results.getHeader ();

		header.setSortColumnId ( settings->get<int> ( "UI", "search-col" ),
								 settings->get<bool> ( "UI", "search-forwards" ) );

		results.selectRow ( settings->get<int> ( "UI", "search-selected" ) );
		results.setVerticalPosition ( settings->get<double> ( "UI", "search-pos" ) );
	}

	//
	// Restore page
	//
	{
		auto	pageStr = settings->get<std::string> ( "UI", "page" );

		if ( ! isHVSCRootValid () )
		{
			pageStr = "onboarding";
			onboardingScreen.startOver ();
		}

		lastPage = pageStr;
		showPage ( pageStr );
	}

	mainScreen.pages.crtPage.setCRTPage ( settings->get<int> ( "UI", "crt-image" ) );

	juce::Desktop::getInstance ().setScreenSaverEnabled ( preferences->get<bool> ( "UI", "allow-screensaver" ) );

	updateFX ();
}
//-----------------------------------------------------------------------------

namespace
{
void executeCommand ( const juce::String& command, const juce::String& root, const juce::StringArray& files )
{
	if ( command == "mv" && files[ 0 ] == files[ 1 ] )
		return;

	if ( command == "rm" && files[ 0 ].isEmpty () )
		return;

	// Find git repository root
	{
		auto	cwd = juce::File::getCurrentWorkingDirectory ();
		while ( ! cwd.getChildFile ( ".git" ).isDirectory () )
		{
			cwd = cwd.getParentDirectory ();
			if ( ! cwd.isDirectory () )
				return;
		}

		cwd.setAsCurrentWorkingDirectory ();
	}

	// Build command
	auto	cmd = "git " + command;

	for ( const auto& f : files )
		cmd += " " + ( "Data" + f.replaceCharacter ('\\', '/').fromFirstOccurrenceOf ( root, true, false) ).quoted ();

	// Execute command
	{
		auto	cp = juce::ChildProcess ();
		cp.start ( cmd );
		cp.waitForProcessToFinish ( -1 );

		if ( cp.getExitCode () )
			Z_ERR ( "Command failed: " + cmd + "\nOutput: " + cp.readAllProcessOutput () )
	}
}
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::addScreenshots ( const juce::StringArray& filenames )
{
	if ( ! UI::isDeveloperMode () || filenames.isEmpty () || lastFilename.empty () )
		return;

	// Use oxipng to optimize screenshots
	for ( const auto& f : filenames )
	{
		const auto	cmd = "oxipng -o max -Z -s " + f.quoted ();

		auto	cp = juce::ChildProcess ();
		cp.start ( cmd );
		cp.waitForProcessToFinish ( -1 );
	}

	const auto	dstDir = juce::String ( lastFilename ).fromFirstOccurrenceOf ( "/", false, false ).upToLastOccurrenceOf ( "/", true, false );
	const auto	dstName = juce::String ( lastFilename ).fromLastOccurrenceOf ( "/", false, false ).upToLastOccurrenceOf ( ".", false, false ).toLowerCase ();

	auto	dst = dataRoot.getChildFile ( "Screenshots/" + dstDir );
	dst.createDirectory ();

	for ( const auto& f : filenames )
 	{
		auto	srcFile = juce::File ( f );
		if ( ! srcFile.hasFileExtension ( ".png" ) || ! srcFile.existsAsFile () )
			continue;

		const auto	srcNumber = srcFile.getFileName ().fromLastOccurrenceOf ( "_", true, false );

		// Expecting an underscore, 2-digit number, and then ".png" = 7 characters
		if ( srcNumber.length () != 7 )
			continue;

		auto	dstFile = dst.getChildFile ( dstName + srcNumber );

 		srcFile.moveFileTo ( dstFile );

		// Use git to add new file to repository
		executeCommand ( "add", "/Screenshots/", { dstFile.getFullPathName () });
	}
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::addSidTunes ( const juce::StringArray& filenames )
{
	if ( filenames.isEmpty () )
		return;

	auto	dstPath = paths::getUserTunesPath ();
	if ( dstPath == juce::File () || ! dstPath.isDirectory () )
		return;

	// Move files
	for ( const auto& f : filenames )
	{
		auto	srcFile = juce::File ( f );
		if ( ! srcFile.hasFileExtension ( ".sid" ) || ! srcFile.existsAsFile () )
			continue;

		auto	dstFile = dstPath.getChildFile ( srcFile.getFileName () );
		srcFile.moveFileTo ( dstFile );
	}
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::assignBorderColor ( const int index )
{
	auto	file = mainScreen.pages.crtPage.getLastLoadedFile ();

	auto	newName = file.getFullPathName ();

	auto	hint = helpers::hintFromFilename ( newName );
	hint.borderColor = int8_t ( index );
	newName = helpers::filenameFromHint ( hint );

	executeCommand ( "mv", "/Screenshots/", { file.getFullPathName (), newName } );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::toggleFirstLuma ()
{
	auto	file = mainScreen.pages.crtPage.getLastLoadedFile ();

	auto	newName = file.getFullPathName ();

	auto	hint = helpers::hintFromFilename ( newName );
	hint.firstLuma = ! hint.firstLuma;
	newName = helpers::filenameFromHint ( hint );

	executeCommand ( "mv", "/Screenshots/", { file.getFullPathName (), newName } );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::toggleFirstLumaAll ()
{
	auto	tuneArtwork = screenshots->getScreenshots ( lastFilename );

	for ( const auto& art : tuneArtwork )
	{
		auto	file = dataRoot.getChildFile ( "Screenshots/" ).getChildFile ( art );
		auto	newName = file.getFullPathName ();

		auto	hint = helpers::hintFromFilename ( newName );
		hint.firstLuma = ! hint.firstLuma;
		newName = helpers::filenameFromHint ( hint );

		executeCommand ( "mv", "/Screenshots/", { file.getFullPathName (), newName } );
	}
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::toggleThumbnail ()
{
	auto	file = mainScreen.pages.crtPage.getLastLoadedFile ();

	auto	newName = file.getFullPathName ();

	auto	hint = helpers::hintFromFilename ( newName );
	hint.isThumbnail = ! hint.isThumbnail;
	newName = helpers::filenameFromHint ( hint );

	executeCommand ( "mv", "/Screenshots/", { file.getFullPathName (), newName } );
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::deleteImage ()
{
	auto	file = mainScreen.pages.crtPage.getLastLoadedFile ();

	executeCommand ( "rm", "/Screenshots/", { file.getFullPathName () } );
}
//-----------------------------------------------------------------------------
