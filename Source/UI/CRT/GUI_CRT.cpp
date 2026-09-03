#include <JuceHeader.h>

#include "GUI_CRT.h"

#include "ultra-shared/Config/DataSource.h"
#include "Globals/constants.h"
#include "ultra-shared/Helpers/ComponentUtils.h"

//-----------------------------------------------------------------------------

GUI_CRT::GUI_CRT ()
{
	setName ( "crt" );
	addAndMakeVisible ( overlay );

	//
	// Browser
	//
	{
		overlay.openBrowser.onClick = [ this ]
		{
			showBrowser ( overlay.openBrowser.getStage () );
		};

		addChildComponent ( browser );
	}

	//
	// CRT settings
	//
	{
		overlay.openSettings.onClick = [ this ]
		{
			showSettings ( overlay.openSettings.getStage () );
		};

		addChildComponent ( settingsPanel );

		settingsPanel.onSettingsChanged = [ this ]	{	updateOverlayCRTSettings ();	};
		settingsPanel.onOverlayChanged = [ this ]	{	overlay.updateOverlay ();		};
		settingsPanel.onZoomChanged = [ this ]		{	overlay.updateZoom ();			};

		settingsPanel.autoSystem = [ this ]			{	return juce::String ( streamIsNTSC ? "NTSC" : "PAL" );	};
		settingsPanel.autoFirstLuma = [ this ]		{	return lastFirstLuma;	};
	}

	startTimer ( 'NTSC', 100 );

	addMouseListener ( this, true );
}
//-----------------------------------------------------------------------------

void GUI_CRT::timerCallback ( int timerID )
{
	switch ( timerID )
	{
		case 'NTSC':
			if ( overlay.isStreamNTSC != streamIsNTSC )
			{
				streamIsNTSC.store ( overlay.isStreamNTSC.load () );
				updateOverlayCRTSettings ();
				settingsPanel.updateCRTsettingsUI ();
			}
			break;
	}
}
//-----------------------------------------------------------------------------

void GUI_CRT::paintIntoSnapshot ( juce::Image& snapshot, juce::Component& top )
{
	if ( ! isShowing () )
		return;

	const auto	frame = overlay.grabFrame ();

	if ( ! frame.isValid () )
		return;

	juce::Graphics	g ( snapshot );
	g.drawImage ( frame, top.getLocalArea ( &overlay, overlay.getLocalBounds () ).toFloat () );
}
//-----------------------------------------------------------------------------

void GUI_CRT::resized ()
{
	const auto	kioskMode = dynamic_cast<juce::DocumentWindow*> ( getTopLevelComponent () )->isKioskMode ();

	crtLayout.setConstant ( "fullscreen", kioskMode ? 1 : 0 );
	crtLayout.setConstant ( "windowed", kioskMode ? 0 : 1 );
	crtLayout.setConstant ( "showBrowser", browserVisible ? 1 : 0 );
	crtLayout.setConstant ( "showSettings", settingsVisible ? 1 : 0 );

	UI::setLayout ( crtLayout, {	"UI/layouts/constants.json",
								"UI/layouts/pages/crt.json" } );
}
//-----------------------------------------------------------------------------

void GUI_CRT::mouseWheelMove ( const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel )
{
	if ( event.eventComponent != &overlay )
		return;

	const auto	delta = wheel.deltaY * ( event.mods.isShiftDown () ? 20.0f : 50.0f );

	if ( event.mods.isCommandDown () )
	{
		auto	overscan = componentutils::findComponent<juce::Slider> ( "tv/overscan/slider", settingsPanel.componentMap () );

		const auto	newOverscan = std::clamp ( float ( overscan->getValue () ) + delta, 0.0f, 100.0f );
		overscan->setValue ( newOverscan, juce::dontSendNotification );
		preferences->set ( "tv/overscan", newOverscan );
	}
	else
	{
		if ( ! preferences->get<bool> ( "overlay/enabled" ) )
			return;

 		auto	zoom = componentutils::findComponent<juce::Slider> ( "overlay/disabler/zoom/slider", settingsPanel.componentMap () );

 		const auto	newZoom = std::clamp ( zoom->getValue () + delta, 0.0, 100.0 );
		zoom->setValue ( newZoom, juce::dontSendNotification );
 		preferences->set ( "overlay/zoom", int ( newZoom ) );
	}

	updateOverlayCRTSettings ();
	overlay.updateZoom ();
}
//-----------------------------------------------------------------------------

void GUI_CRT::update ( const float /*secondsPassed*/ )
{
	if ( ! isShowing () )
		return;

	// Update OpenGL iFrame & iTime
	overlay.setFrameAndTime ( 0, float ( juce::Time::highResolutionTicksToSeconds ( juce::Time::getHighResolutionTicks () ) ) );
}
//-----------------------------------------------------------------------------

void GUI_CRT::showBrowser ( const bool visible )
{
	browser.setVisible ( visible );
	browserVisible = visible;

	overlay.openBrowser.setStage ( browserVisible ? 1 : 0 );

	resized ();
}
//-----------------------------------------------------------------------------

void GUI_CRT::refreshBrowserEntries ()
{
	browser.refreshBrowserEntries ();
}
//-----------------------------------------------------------------------------

void GUI_CRT::showSettings ( const bool visible )
{
	// Cameras hot-plug (and OBS virtual ones come and go), refresh per open
	if ( visible )
		settingsPanel.refreshWebcamDevices ();

	settingsPanel.setVisible ( visible );
	settingsVisible = visible;

	overlay.openSettings.setStage ( settingsVisible ? 1 : 0	);

	resized ();
}
//-----------------------------------------------------------------------------

void GUI_CRT::setBackgroundColour ( const juce::Colour& bckCol )
{
	overlay.setBackgroundColor ( bckCol );
}
//-----------------------------------------------------------------------------

void GUI_CRT::setFirstLuma ( const bool isFirstLuma )
{
	lastFirstLuma = isFirstLuma;
	renderCRT ();
}
//-----------------------------------------------------------------------------

void GUI_CRT::showRasterTime ( const bool show )
{
	overlay.enableRenderTimeMeasurement ( show );
	overlay.enableRenderTimeDisplay ( show );
}
//-----------------------------------------------------------------------------

void GUI_CRT::renderCRT ()
{
	auto	vic2Settings = settingsPanel.getVIC2SettingsFromPreferences ();
	auto	settings = overlay.getSettings ();
	settings.isNTSC = vic2Settings.standard == VIC2_Render::settings::NTSC;

	updateCRTPalette ( vic2Settings );

	overlay.setSettings ( settings );

	settingsPanel.updateCRTsettingsUI ();
}
//-----------------------------------------------------------------------------

void GUI_CRT::updateOverlayCRTSettings ()
{
	auto	settings = settingsPanel.getCRTEmulationSettingsFromPreferences ();

	// VIC2 settings that affect CRT emulation
	{
		const auto	vic2Settings = settingsPanel.getVIC2SettingsFromPreferences ();

		settings.isNTSC = vic2Settings.standard == VIC2_Render::settings::NTSC;
		settings.crtEmulation = ! vic2Settings.raw;

		updateCRTPalette ( vic2Settings );
	}

	overlay.setSettings ( settings );
}
//-----------------------------------------------------------------------------

void GUI_CRT::updateCRTPalette ( const VIC2_Render::settings& vic2Settings )
{
	if (	vic2Settings.needsNewPalette ( curVicSettings )
		 ||	yuvE_yuvO_yiq.empty () )
	{
		yuvE_yuvO_yiq = colo.generateYUV_YIQ ( vic2Settings.firstLuma, vic2Settings.warmth );
		overlay.setLumaChromaPalette ( yuvE_yuvO_yiq );

		curVicSettings = vic2Settings;
	}
}
//-----------------------------------------------------------------------------

void GUI_CRT::userCRTContentChanged ( const juce::String& relPath, const gin::FileSystemWatcher::FileSystemEvent event )
{
	// The same file addressed through the factory tree, which is the only way
	// lime knows it; the content loader routes it back to the user file
	const auto	nominal = datasource::getCRTRoot ().getChildFile ( relPath );

	// Live tweak of an existing file: feed lime's own hot-reload path (profile
	// yml re-parse, texture reloads with dependency tracking)
	if ( event == gin::FileSystemWatcher::fileUpdated )
	{
		overlay.fileChanged ( nominal, event );
		return;
	}

	// Files appeared or disappeared: the pick lists change, and the files
	// behind the active overlay/mask may have come or gone - re-pull them
	// through the loader
	overlay.rescanOverlays ();
	settingsPanel.refreshCRTPickLists ();

	updateOverlayCRTSettings ();
	overlay.loadOverlayProfile ( settingsPanel.currentOverlayName () );
	overlay.updateOverlay ();
}
//-----------------------------------------------------------------------------
