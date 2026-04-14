#pragma once

#include <JuceHeader.h>

#include <chrono>

#include "UI/Components/GUI_SVG_Button.h"

#include "UI/Misc/colodore.h"
#include "UI/Misc/VIC2_Render.h"

#include "Globals/Preferences.h"

#include "Network/C64u_UDP_Receiver.h"

//-----------------------------------------------------------------------------

class GUI_Overlay final : public lime::CRTEmulation, private juce::Timer
{
public:
	GUI_Overlay ();

	// juce::Component
	void resized () override;
	void mouseMove ( const juce::MouseEvent& evt ) override;

	// lime::CRTEmulation
	void newOpenGLContextCreated () override;
	void openGLContextClosing () override;

	// this
	void setStreamAddress ( const juce::String& address );

	GUI_SVG_Button	openSettings { "open", { "crt/settings_close", "crt/settings_open" } };

private:
	// juce::Timer
	void timerCallback () override;

	// C64u UDP Receiver
	juce::CriticalSection	streamLock;
	juce::String		c64uStreamAddress;
	C64u_UDP_Receiver	c64uReceiver { C64u_UDP_Receiver::streamType::video };
	lime::openGL_Image	c64uBuffer[ 2 ] = { { 1, 384, 272 }, { 1, 384, 272 } };

	void startVideoStream ();

	//
	// Hide mouse-cursor helpers
	//
	bool				kioskMode = false;
	juce::Point<int>	oldMousePos = { -1000, -1000 };

	bool shouldHideCursor () const;
	void showCursor ();
	void hideCursor ();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Overlay )
};
//-----------------------------------------------------------------------------
