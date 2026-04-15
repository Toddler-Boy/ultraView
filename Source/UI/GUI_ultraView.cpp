#include "GUI_ultraView.h"

#include "SID_LookAndFeel.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_ultraView::GUI_ultraView ()
	: mainScreen ( deviceManager )
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

	folderWatcher.coalesceEvents ( 50 );
	folderWatcher.addListener ( this );
	theme->setTargetLAF ( getLookAndFeel () );

	setDataRoot ();
	setUserRoot ();

	loadTheme ();

	curOutVol.set ( 1.0f );
	setAudioChannels ( 0, 2 );

	//
	// Setup C64u UDP receiver callback for live preview
	//
	c64uReceiver.setAudioBuffers ( c64uBuffer.getWritePointer ( 0 ), c64uBuffer.getWritePointer ( 1 ) );

	c64uReceiver.onAudioChunk = [ this ] ( int /*index*/ )
	{
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

	// Setup network
	{
		findC64OnNetwork ();
	}
}
//-----------------------------------------------------------------------------

GUI_ultraView::~GUI_ultraView ()
{
	c64uReceiver.stop ();

	shutdownAudio ();
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

	mainScreen.crt.setBackgroundColour ( bgCol );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::loadTheme ()
{
	auto	themeName = preferences->get<juce::String> ( "UI", "theme" );

	theme->load ( themeName );

	updateColors ();
	sendLookAndFeelChange ();
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

bool GUI_ultraView::isDataRootValid () const
{
	if ( ! dataRoot.isDirectory () )
	{
		Z_ERR ( "ultraView data folder is missing" );
		return false;
	}

	static const juce::StringArray	arr = {
		"CRTEmulation/Shaders/",

		"Themes/",

		"UI/",
		"UI/fonts/",
		"UI/layouts/",
		"UI/strings/",
		"UI/svg/",
		"UI/icons.yml",
	};

	const auto	dataValid = allPathsValid ( arr, dataRoot );

	if ( ! dataValid )
	{
		Z_ERR ( "ultraView data folder is incomplete (missing folders or files) " + dataRoot.getFullPathName () );
		jassertfalse; // This should never happen in a release build, but it's good to catch during development
	}

	return dataValid;
}
//-----------------------------------------------------------------------------

void GUI_ultraView::setDataRoot ()
{
	dataRoot = paths::getDataRoot ();

	if ( ! isDataRootValid () )
		return;

	folderWatcher.removeAllFolders ();

	theme->setRoot ( dataRoot.getChildFile ( "Themes/" ) );

	folderWatcher.addFolder ( dataRoot );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::setUserRoot ()
{
	folderWatcher.removeFolder ( userRoot );

	userRoot = settings->get<juce::String> ( "Paths", "user" );

	preferences->setRoot ( userRoot );

	folderWatcher.addFolder ( userRoot );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::saveState ()
{
}
//-----------------------------------------------------------------------------

void GUI_ultraView::restoreState ()
{
	juce::Desktop::getInstance ().setScreenSaverEnabled ( preferences->get<bool> ( "UI", "allow-screensaver" ) );
}
//-----------------------------------------------------------------------------

