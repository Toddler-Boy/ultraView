#include "GUI_Overlay.h"

#include "Globals/constants.h"

#include "UI/Misc/VIC2_Render.h"

//-----------------------------------------------------------------------------

GUI_Overlay::GUI_Overlay ()
	: CRTEmulation ( true,
					 paths::getDataRoot ( "CRTEmulation" ),
					 resolutions {	VIC2_Render::outerUnscaledWidth, VIC2_Render::outerUnscaledHeight,
									VIC2_Render::outerUnscaledWidth * 4, VIC2_Render::outerUnscaledHeight * 4 } )
{
	setName ( "CRT" );

//	enableRenderTimeMeasurement ( true );

	//
	// Settings button
	//
	{
		auto setButtonProps = [ this ] ( GUI_SVG_Button& button )
		{
			button.margin = 14.0f;
			button.bckAlpha[ 1 ] = 0.1f;
			button.bckMargin = 6.0f;
			button.setSize ( 48, 48 );
			button.setWantsKeyboardFocus ( false );
			addAndMakeVisible ( button );
		};

		setButtonProps ( openBrowser );
		setButtonProps ( openSettings );
	}

	// Action buttons
	{
		auto addActionButton = [ this ] ( GUI_IconButton& button )
		{
			button.setWantsKeyboardFocus ( false );
			actionButtons.addAndMakeVisible ( button );

			button.onClick = [ name = button.getName () ]
			{
				UI::sendGlobalMessage ( "c64action {}", name );
			};
		};

		addActionButton ( actionMenu );
		addActionButton ( actionPause );
		addActionButton ( actionResume );
		addActionButton ( actionReboot );
		addActionButton ( actionPower );

		addAndMakeVisible ( actionButtons );
	}

	//
	// Setup C64u UDP receiver callback for live preview
	//
	c64uReceiver.setVideoBuffers ( c64uBuffer[ 0 ].getData (), c64uBuffer[ 1 ].getData () );

	c64uReceiver.onVideoFrame = [ this ] ( int finishedBufferIndex, bool isNTSC )
	{
		if ( isNTSC )
		{
			constexpr auto	w = VIC2_Render::outerUnscaledWidth;
			constexpr auto	h = 240;
			constexpr auto	ntscOverscanTop = 15;
			constexpr auto	ntscOverscanBottom = VIC2_Render::outerUnscaledHeight - h - ntscOverscanTop;
			constexpr auto	ntscBottomStart = VIC2_Render::outerUnscaledHeight - ntscOverscanBottom;

			// Move the visible 240 lines down so the buffer stays centered
			std::memmove ( c64uBuffer[ finishedBufferIndex ].getLinePointer ( ntscOverscanTop ), c64uBuffer[ finishedBufferIndex ].getData (), w * h );

			// Clear top and bottom borders (NTSC overscan area)
			std::memset ( c64uBuffer[ finishedBufferIndex ].getData (), 0, ntscOverscanTop * w );
			std::memset ( c64uBuffer[ finishedBufferIndex ].getLinePointer ( ntscBottomStart ), 0, ntscOverscanBottom * w );
		}

		isStreamNTSC.store ( isNTSC, std::memory_order_relaxed );

		setIndexTextureSource ( c64uBuffer[ finishedBufferIndex ] );
	};

	c64uReceiver.onStatusChange = [] ( bool receiving )
	{
		if ( receiving )
			juce::Logger::writeToLog ( "[I]C64u UDP receiver started receiving data (video)" );
		else
			lime::Logger::writeToLog ( "[W]C64u UDP receiver stopped receiving data (video)" );
	};
}
//-----------------------------------------------------------------------------

void GUI_Overlay::newOpenGLContextCreated ()
{
	CRTEmulation::newOpenGLContextCreated ();

	const juce::ScopedLock lock ( streamLock );
	startVideoStream ();
}
//-----------------------------------------------------------------------------

void GUI_Overlay::openGLContextClosing ()
{
	{
		const juce::ScopedLock lock ( streamLock );
		c64uReceiver.stop ();
	}

	CRTEmulation::openGLContextClosing ();
}
//-----------------------------------------------------------------------------

void GUI_Overlay::renderOpenGL ()
{
	CRTEmulation::renderOpenGL ();

	if ( const auto	load = getLastGpuTimeMS (); load > 0.0 )
	{
		Z_INFO ( "renderOpenGL: " + juce::String ( load ) );
	}
}
//-----------------------------------------------------------------------------

void GUI_Overlay::setStreamAddress ( const juce::String& address )
{
	const juce::ScopedLock lock ( streamLock );

	c64uReceiver.stop ();
	c64uStreamAddress = address;

	if ( isReady () )
		startVideoStream ();
}
//-----------------------------------------------------------------------------

void GUI_Overlay::startVideoStream ()
{
	if ( c64uStreamAddress.isNotEmpty () )
		UI::sendGlobalMessage ( "stream-status video {}", c64uReceiver.start ( c64uStreamAddress ).quoted () );
}
//-----------------------------------------------------------------------------

void GUI_Overlay::resized ()
{
	CRTEmulation::resized ();

	kioskMode = dynamic_cast<juce::DocumentWindow*> ( getTopLevelComponent () )->isKioskMode ();

	stopTimer ();
	showCursor ();

	if ( shouldHideCursor () )
		startTimer ( 2000 );
}
//-----------------------------------------------------------------------------

bool GUI_Overlay::shouldHideCursor () const
{
	return ! openSettings.getStage () && isMouseOverOrDragging () && kioskMode;
}
//-----------------------------------------------------------------------------

void GUI_Overlay::showCursor ()
{
	//
	// Show cursor and settings button
	//
	setMouseCursor ( juce::MouseCursor::NormalCursor );
	openSettings.setVisible ( true );
	actionButtons.setVisible ( true );
}
//-----------------------------------------------------------------------------

void GUI_Overlay::hideCursor ()
{
	//
	// Hide the cursor and settings button
	//
	if ( kioskMode )
		setMouseCursor ( juce::MouseCursor::NoCursor );

	openSettings.setVisible ( false );
	actionButtons.setVisible ( false );
}
//-----------------------------------------------------------------------------

void GUI_Overlay::mouseMove ( const juce::MouseEvent& evt )
{
	//
	// Prevent modifier keys (ctrl, shift, etc.) from triggering mouseMouse events
	//
	const auto	newMousePos = evt.getPosition ().toInt ();
	if ( newMousePos == oldMousePos )
		return;

	oldMousePos = newMousePos;

	showCursor ();

	if ( shouldHideCursor () )
		startTimer ( 2000 );
}
//-----------------------------------------------------------------------------

void GUI_Overlay::timerCallback ()
{
	stopTimer ();
	hideCursor ();
}
//-----------------------------------------------------------------------------
