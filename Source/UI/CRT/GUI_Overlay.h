#pragma once

#include <JuceHeader.h>

#include "UI/Components/GUI_IconButton.h"
#include "UI/Components/GUI_SVG_Button.h"

#include "Network/C64u_UDP_Receiver.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

class GUI_Overlay final
	: public lime::CRTEmulation
	, public juce::DragAndDropTarget
    , private juce::Timer
{
public:
	GUI_Overlay ();
	~GUI_Overlay () override;

	// lime::CRTEmulation
	void newOpenGLContextCreated () override;
	void openGLContextClosing () override;
	void renderOpenGL () override;

	// juce::DragAndDropTarget
	bool isInterestedInDragSource ( const SourceDetails& details ) override;
	void itemDropped ( const SourceDetails& details ) override;

	// this
	void setStreamAddress ( const juce::String& address );

	GUI_SVG_Button	openBrowser { "browser", { "crt/settings_close", "crt/settings_open" } };
	GUI_SVG_Button	openSettings { "open", { "crt/settings_close", "crt/settings_open" } };

	std::atomic<bool>	isStreamNTSC = false;

private:
	// C64u UDP Receiver
	juce::CriticalSection	streamLock;
	juce::String			c64uStreamAddress;
	lime::openGL_Image		c64uBuffer[ 2 ] = { { 1, 384, 272 }, { 1, 384, 272 } };
	C64u_UDP_Receiver		c64uReceiver { C64u_UDP_Receiver::streamType::video };

	void startVideoStream ();

	// Action buttons
	juce::Component		actionButtons { "action-buttons" };
		GUI_IconButton		actionMenu { "menu_button", UI::colors::actionOk };
		GUI_IconButton		actionPause { "pause", UI::colors::actionInfo };
		GUI_IconButton		actionResume { "resume", UI::colors::actionInfo };
		GUI_IconButton		actionReboot { "reboot", UI::colors::actionWarning, 200 };
		GUI_IconButton		actionPower { "poweroff", UI::colors::actionDanger, 500 };

//	void showCursor ();
//	void hideCursor ();

	// juce::Timer
	void timerCallback () override;
	void updateUI ( bool childrenVisible, bool cursorVisible );
	void handleGlobalMouseMove ( const juce::MouseEvent& e );
	void handleGlobalMouseUp ( const juce::MouseEvent& e );
	void processStateAt ( const juce::Point<int> screenPos );

	juce::Point<int>	lastMouseScreenPos = { -1000, -1000 };
	bool				curChildrenVisible = true;
	bool				curCursorVisible = true;

	// A simple helper to avoid the inheritance ambiguity
	struct HelperListener : public juce::MouseListener
	{
		std::function<void ( const juce::MouseEvent& )> onMouseMove;
		std::function<void ( const juce::MouseEvent& )> onMouseUp;

		void mouseMove ( const juce::MouseEvent& e ) override { if ( onMouseMove ) onMouseMove ( e ); }
		void mouseDrag ( const juce::MouseEvent& e ) override { if ( onMouseMove ) onMouseMove ( e ); }
		void mouseUp ( const juce::MouseEvent& e ) override { if ( onMouseUp ) onMouseUp ( e ); }
	} globalListener;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Overlay )
};
//-----------------------------------------------------------------------------
