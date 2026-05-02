#include "GUI_Overlay.h"

#include "Globals/constants.h"

#include "UI/Misc/VIC2_Render.h"

//-----------------------------------------------------------------------------

GUI_Overlay::GUI_Overlay ()
	: CRTEmulation ( true, 2000,
					 paths::getDataRoot ( "CRTEmulation" ),
					 resolutions {	VIC2_Render::outerUnscaledWidth, VIC2_Render::outerUnscaledHeight,
									VIC2_Render::outerUnscaledWidth * 4, VIC2_Render::outerUnscaledHeight * 4 } )
{
	setName ( "CRT" );

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
