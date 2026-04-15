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

	//
	// Settings button
	//
	{
		openSettings.margin = 14.0f;
		openSettings.bckAlpha[ 1 ] = 0.1f;
		openSettings.bckMargin = 6.0f;
		openSettings.setSize ( 48, 48 );
		openSettings.setWantsKeyboardFocus ( false );

		addAndMakeVisible ( openSettings );
	}

	// Action buttons
	{
		auto addActionButton = [ this ] ( GUI_IconButton& button )
		{
			button.setWantsKeyboardFocus ( false );
			actionButtons.addAndMakeVisible ( button );
			button.onClick = [ this, name = button.getName () ]
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
	c64uReceiver.setVideoBuffers ( c64uBuffer[ 0 ].data.data (), c64uBuffer[ 1 ].data.data () );

	c64uReceiver.onVideoFrame = [ this ] ( int finishedBufferIndex, bool /*isNTSC*/ )
	{
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
	ShaderToyComponent::newOpenGLContextCreated ();

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

	ShaderToyComponent::openGLContextClosing ();
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
