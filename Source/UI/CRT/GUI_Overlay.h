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
{
public:
	GUI_Overlay ();

	// lime::CRTEmulation
	void newOpenGLContextCreated () override;
	void openGLContextClosing () override;

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

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Overlay )
};
//-----------------------------------------------------------------------------
