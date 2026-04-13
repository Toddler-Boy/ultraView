#include "GUI_Overlay.h"

#include "Globals/constants.h"

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

	//
	// Page control
	//
	{
		addAndMakeVisible ( pageControl );
	}

	//
	// Setup C64u UDP receiver callback for live preview
	//
/*	c64uReceiver.setVideoBuffers (c64uBuffer[0].data.data (), c64uBuffer[1].data.data ());

	c64uReceiver.onVideoFrame = [ this ] ( int finishedBufferIndex, bool isNTSC )
	{
		if ( preferences->get<bool> ( "stream/enabled" ) )
			indexSourceTexture->fromImage ( c64uBuffer[ finishedBufferIndex ], false, false, true );
	};

	c64uReceiver.onStatusChange = [] ( bool receiving )
	{
		if ( receiving )
			juce::Logger::writeToLog ( "[I]C64u UDP receiver started receiving data (video)" );
		else
			lime::Logger::writeToLog ( "[W]C64u UDP receiver stopped receiving data (video)" );
	};
*/
}
//-----------------------------------------------------------------------------

/*void GUI_Overlay::newOpenGLContextCreated ()
{
	ShaderToyComponent::newOpenGLContextCreated ();

//	c64uReceiver.start ( "239.0.1.64", 11000 );
}
//-----------------------------------------------------------------------------

void GUI_Overlay::openGLContextClosing ()
{
//	c64uReceiver.stop ();

	ShaderToyComponent::openGLContextClosing ();
}
//-----------------------------------------------------------------------------
*/

void GUI_Overlay::resized ()
{
	CRTEmulation::resized ();

	kioskMode = dynamic_cast<juce::DocumentWindow*> ( getTopLevelComponent () )->isKioskMode ();

	stopTimer ();
	showCursor ();

	if ( shouldHideCursor () )
		startTimer ( 2000 );

	pageControl.updateLayout ();
}
//-----------------------------------------------------------------------------

bool GUI_Overlay::shouldHideCursor () const
{
	return ! openSettings.getStage () && isMouseOverOrDragging ();
}
//-----------------------------------------------------------------------------

void GUI_Overlay::showCursor ()
{
	//
	// Show cursor and settings button
	//
	setMouseCursor ( juce::MouseCursor::NormalCursor );
	openSettings.setAlpha ( 1.0f );
	pageControl.setAlpha ( 1.0f );
}
//-----------------------------------------------------------------------------

void GUI_Overlay::hideCursor ()
{
	if ( pageControl.isMouseOverOrDragging () )
		return;

	//
	// Hide the cursor and settings button
	//
	if ( kioskMode )
		setMouseCursor ( juce::MouseCursor::NoCursor );

	openSettings.setAlpha ( 0.0f );
	pageControl.setAlpha ( 0.0f );
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
