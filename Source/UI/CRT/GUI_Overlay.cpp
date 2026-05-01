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
	// Use a separate listener object to avoid inheritance ambiguity
	globalListener.onMouseMove = [ this ] ( const juce::MouseEvent& e ) { handleGlobalMouseMove ( e ); };
	globalListener.onMouseUp = [ this ] ( const juce::MouseEvent& e ) { handleGlobalMouseUp ( e ); };

	// Register this separate object globally
	juce::Desktop::getInstance ().addGlobalMouseListener ( &globalListener );

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

GUI_Overlay::~GUI_Overlay ()
{
	juce::Desktop::getInstance ().removeGlobalMouseListener ( &globalListener );
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

void GUI_Overlay::handleGlobalMouseMove ( const juce::MouseEvent& e )
{
	const auto	screenPos = e.getScreenPosition ();

	if ( screenPos == lastMouseScreenPos )
		return;

	lastMouseScreenPos = screenPos;

	if ( e.mods.isAnyMouseButtonDown () )
		return;

	processStateAt ( screenPos );
}
//-----------------------------------------------------------------------------

void GUI_Overlay::handleGlobalMouseUp ( const juce::MouseEvent& e )
{
	auto	screenPos = e.getScreenPosition ();
	auto	isOverThis = getScreenBounds ().contains ( screenPos );

	if ( isOverThis )
		startTimer ( 2000 );
}
//-----------------------------------------------------------------------------

void GUI_Overlay::processStateAt ( const juce::Point<int> screenPos )
{
	const auto	localPos = getLocalPoint ( nullptr, screenPos );

	const auto*	hit = getComponentAt ( localPos );
	const auto	isOverThis = getLocalBounds ().contains ( localPos );
	const auto	isOverChild = ( hit != nullptr && hit != this );

	if ( isOverChild )
	{
		stopTimer ();
		updateUI ( true, true );
	}
	else if ( isOverThis )
	{
		updateUI ( true, true );
		startTimer ( 2000 );
	}
	else
	{
		if ( ! isTimerRunning () )
			startTimer ( 2000 );
	}
}
//-----------------------------------------------------------------------------

void GUI_Overlay::timerCallback ()
{
	stopTimer ();

	auto	isCurrentlyOverUs = getScreenBounds ().contains ( juce::Desktop::getMousePosition () );

	updateUI ( false, ! isCurrentlyOverUs );
}
//-----------------------------------------------------------------------------

void GUI_Overlay::updateUI ( bool childrenVisible, bool cursorVisible )
{
	if ( childrenVisible == curChildrenVisible && cursorVisible == curCursorVisible )
		return;

	curChildrenVisible = childrenVisible;
	curCursorVisible = cursorVisible;

	for ( auto* child : getChildren () )
		child->setVisible ( childrenVisible );

	setMouseCursor ( cursorVisible ? juce::MouseCursor::NormalCursor : juce::MouseCursor::NoCursor );
}
//-----------------------------------------------------------------------------

bool GUI_Overlay::isInterestedInDragSource ( const SourceDetails& /*details*/ )
{
	return true;
}
//-----------------------------------------------------------------------------

void GUI_Overlay::itemDropped ( const SourceDetails& details )
{
	UI::sendGlobalMessage ( "c64run {}", details.description.toString ().quoted () );
}
//-----------------------------------------------------------------------------
