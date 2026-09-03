#include "GUI_ultraView.h"

#include "ultra-shared/UI/GUI_LookAndFeel.h"

#include "ultra-shared/Config/DataSource.h"
#include "ultra-shared/Helpers/PlatformHelper.h"
#include "Config/FilePaths.h"
#include "Globals/constants.h"

//-----------------------------------------------------------------------------

// AudioAppComponent only stores the reference here
GUI_ultraView::GUI_ultraView ()
	: juce::AudioAppComponent ( ownedDeviceManager )
	, mainScreen ( deviceManager )
{
	UI::setActionBroadCaster ( this );

	setName ( "ultraView" );

	mainScreen.layout.setConstant ( "fullscreen", 0 );
	mainScreen.layout.setConstant ( "windowed", 1 );

	setWantsKeyboardFocus ( true );

	tooltipWindow->setOpaque ( false );

	// Detect double-clicks on CRT
	addMouseListener ( this, true );

	// A little trick to allow child components to send actions to this component
	addActionListener ( this );

	// Add listener to main menu
	addAndMakeVisible ( mainScreen );
	addChildComponent ( aboutScreen );

	folderWatcher.coalesceEvents ( 50 );
	folderWatcher.addListener ( this );
	theme->setTargetLAF ( getLookAndFeel () );

	setDataRoot ();
	setUserRoot ();

	appUpdater.onStateChanged = [ this ] ( const AppUpdater::State state )
	{
		mainScreen.crt.versionPill ().setState ( state );
	};

	appUpdater.onProgress = [ this ] ( const float progress )
	{
		mainScreen.crt.versionPill ().setProgress ( progress );
	};

	// The regular quit path, the swapped-in program relaunches after the exit
	appUpdater.onInstalled = [ this ]
	{
		static_cast<juce::DocumentWindow*> ( getParentComponent () )->closeButtonPressed ();
	};

	mainScreen.crt.versionPill ().onClick = [ this ]
	{
		if ( AppUpdater::canInstall && appUpdater.updatePending () )
			appUpdater.install ();
		else
			appUpdater.checkNow ();
	};
	mainScreen.crt.versionPill ().setState ( appUpdater.state () );

	// Needs the loaded preferences
	appUpdater.check ();

	loadTheme ();

	curOutVol.set ( 1.0f );

	initAudio ();

	//
	// Setup C64u UDP receiver callback for live preview
	//
	c64uReceiver.setAudioBuffers ( c64uBuffer.getWritePointer ( 0 ), c64uBuffer.getWritePointer ( 1 ) );

	c64uReceiver.onAudioChunk = [ this ] ( int /*index*/ )
	{
		if ( streamFifo.getFreeSpace () >= c64uBuffer.getNumSamples () )
			streamFifo.write ( c64uBuffer );
	};

	c64uReceiver.onStatusChange = [ this ] ( bool receiving )
	{
		netReceiving = receiving;

		if ( receiving )
		{
			const juce::ScopedLock	sl ( titleLock );
			setTitleStatus ( connectedTitle );
		}
		else
		{
			setTitleStatus ( strings->get ( "network/waiting" ) );
		}
	};

	// Setup network; priming the password first also logs its state right at
	// startup, where support logs need it
	{
		network.setC64uPassword ( settings->get<juce::String> ( "network/password" ) );

		// A cancelled firewall prompt from an earlier run left a block rule
		// pinned to this exe: no stream data can ever arrive, say so right away
		if ( firewallBlocksThisApp () )
			showFirewallNotice ();

		findC64OnNetwork ();
		startTimer ( 5000 );
	}
}
//-----------------------------------------------------------------------------

GUI_ultraView::~GUI_ultraView ()
{
	stopTimer ();
	healWatchdog.stopTimer ();

	// The C64u has no way to notice the listener is gone and would stream at
	// this machine forever; wait briefly so the stops leave before teardown
	{
		juce::WaitableEvent	sent;

		network.put ( "v1/streams/video:stop" );
		network.put ( "v1/streams/audio:stop", {}, [ &sent ] ( const juce::var&, const int ) { sent.signal (); } );

		sent.wait ( 1000 );
	}

	c64uReceiver.stop ();

	// shutdownAudio () only detaches the callback from our own manager
	shutdownAudio ();
	deviceManager.closeAudioDevice ();
}
//-----------------------------------------------------------------------------

void GUI_ultraView::toggleFullscreen ()
{
	const auto	kioskMode = isFullscreen ();

	kioskMode ? toWindowed () : toFullscreen ();
}
//-----------------------------------------------------------------------------

bool GUI_ultraView::isFullscreen () const
{
	return static_cast<juce::DocumentWindow*> ( getParentComponent () )->isKioskMode ();
}
//-----------------------------------------------------------------------------

void GUI_ultraView::toFullscreen ()
{
	mainScreen.layout.setConstant ( "fullscreen", 1 );
	mainScreen.layout.setConstant ( "windowed", 0 );

	auto	parent = static_cast<juce::DocumentWindow*> ( getParentComponent () );

	// Only hide browser if it is open
	if ( ( browserIsVisible = mainScreen.crt.isBrowserVisible () ) )
		mainScreen.crt.showBrowser ( false );

	// Only hide monitor/crt settings if they are open
	if ( ( settingsAreVisible = mainScreen.crt.areSettingsVisible () ) )
		mainScreen.crt.showSettings ( false );

	parent->parentHierarchyChanged ();

	juce::Desktop::getInstance ().setKioskModeComponent ( parent, false );

	wasFullscreen = true;
}
//-----------------------------------------------------------------------------

void GUI_ultraView::toWindowed ()
{
	mainScreen.layout.setConstant ( "fullscreen", 0 );
	mainScreen.layout.setConstant ( "windowed", 1 );

	juce::Desktop::getInstance ().setKioskModeComponent ( nullptr, false );

	// Only show browser if it was open when toggling to fullscreen
	if ( browserIsVisible )
		mainScreen.crt.showBrowser ( true );

	// Only show monitor/crt settings if they where open when toggling to fullscreen
	if ( settingsAreVisible )
		mainScreen.crt.showSettings ( true );

	auto	parent = dynamic_cast<juce::DocumentWindow*> ( getTopLevelComponent () );
	parent->parentHierarchyChanged ();

	wasFullscreen = false;
}
//-----------------------------------------------------------------------------

void GUI_ultraView::resized ()
{
	mainScreen.setBounds ( getLocalBounds () );
	aboutScreen.setBounds ( getLocalBounds () );

	const auto	kioskMode = isFullscreen ();

	if ( wasFullscreen && ! kioskMode )
		toWindowed ();
}
//-----------------------------------------------------------------------------

void GUI_ultraView::mouseDoubleClick ( const juce::MouseEvent& evt )
{
	// Double click on "CRT" toggles fullscreen
	if ( evt.eventComponent->getName () == "CRT" )
		toggleFullscreen ();
}
//-----------------------------------------------------------------------------

void GUI_ultraView::updateColors ()
{
	const auto	bgCol = findColour ( UI::colors::window );
	const auto	textCol = findColour ( UI::colors::text );
	UI::setShades ( bgCol, textCol );

	const auto	darkCol = UI::getShade ( 0.1f );
	const auto	bento = bgCol;
	auto&	laf = getLookAndFeel ();

	laf.setColour ( juce::ResizableWindow::backgroundColourId, bgCol );
	laf.setColour ( juce::TextEditor::backgroundColourId, UI::getShade ( 0.02f ) );
	laf.setColour ( juce::TextEditor::textColourId, textCol );
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

		laf.setColour ( juce::HyperlinkButton::textColourId, laf.findColour ( UI::colors::accentBright ) );

		if ( auto p = findParentComponentOfClass<juce::ResizableWindow> () )
			p->setBackgroundColour ( bgCol );
	}

	mainScreen.crt.setBackgroundColour ( bgCol );
	aboutScreen.updateColors ();
}
//-----------------------------------------------------------------------------

void GUI_ultraView::loadTheme ()
{
	// ultraView has no theme selector: the default theme is the theme
	theme->load ( "$DATA$/default" );

	updateColors ();
	sendLookAndFeelChange ();
}
//-----------------------------------------------------------------------------

void GUI_ultraView::setDataRoot ()
{
	Z_INFO ( "Data: " << datasource::describe () );

	folderWatcher.removeAllFolders ();

	// Factory data only exists as watchable files in the naked developer
	// layout; the ultra-shared Data tree is the second factory root
	if ( ! datasource::isPak () )
	{
		dataRoot = datasource::getDevFile ();
		sharedDataRoot = datasource::getSharedDevRoot ();

		folderWatcher.addFolder ( dataRoot );
		folderWatcher.addFolder ( sharedDataRoot );
	}

	loadGamesDatabase ();
}
//-----------------------------------------------------------------------------

void GUI_ultraView::setUserRoot ()
{
	folderWatcher.removeFolder ( userRoot );

	userRoot = settings->get<juce::String> ( "paths/user" );
	userRoot.createDirectory ();

	preferences->setRoot ( userRoot );
	theme->setUserRoot ( filepaths::getUserThemesPath () );

	// Materialize the user content folders so the watcher covers them
	std::ignore = filepaths::getUserOverlaysPath ();
	std::ignore = filepaths::getUserCRTMasksPath ();

	folderWatcher.addFolder ( userRoot );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::saveState ()
{
}
//-----------------------------------------------------------------------------

void GUI_ultraView::restoreState ()
{
}
//-----------------------------------------------------------------------------
