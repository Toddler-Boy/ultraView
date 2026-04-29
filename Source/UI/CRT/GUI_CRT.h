#pragma once

#include <JuceHeader.h>

#include "UI/Misc/VIC2_Render.h"
#include "UI/Misc/colodore.h"

#include "GUI_Browser.h"
#include "GUI_Overlay.h"

#include "UI/Components/GUI_Toggle.h"

#include "Globals/Preferences.h"

#include "UI/Misc/ComponentFactory.h"

//-----------------------------------------------------------------------------

class GUI_CRT final
	: public juce::Component
	, private juce::MultiTimer
{
public:
	GUI_CRT ();

	// juce::Component
	void resized () override;
	void mouseWheelMove ( const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel ) override;

	// this
	void update ( const float secondsPassed );

	void reloadOverlayProfile ()	{	overlay.reloadOverlayProfile ();	}

	bool isBrowserVisible () const { return browserVisible; }
	void showBrowser ( const bool visible );

	bool areSettingsVisible () const	{ return settingsVisible;	}
	void showSettings ( const bool visible );
	void setBackgroundColour ( const juce::Colour& bckCol );

	void setStreamAddress ( const juce::String& address ) { overlay.setStreamAddress ( address ); }
	void setFirstLuma ( const bool isFirstLuma );

private:
	// V-blank stuff
	juce::VBlankAttachment	vBlankAttachment { this, [ this ] ( double time ) {	update ( time );	} };

	// juce::MultiTimer
	void timerCallback ( int timerID ) override;

	// this
	void renderCRT ();
	VIC2_Render::settings getVIC2SettingsFromPreferences () const;
	lime::CRTEmulation::settings getCRTEmulationSettingsFromPreferences () const;

	void updateOverlayCRTSettings ();

	void updateCRTsettingsUI ();
	void updateCRTPalette ( const VIC2_Render::settings& vic2Settings );

	void connectComponents ();

	const colodore				colo;
	colodore::shaderPalette		yuvE_yuvO_yiq;
	VIC2_Render::settings		curVicSettings;

	std::atomic<bool>	streamIsNTSC = false;
	bool	lastFirstLuma = false;
	bool	cursorVisible = true;

	juce::SharedResourcePointer<Preferences>		preferences;

	GUI_Overlay		overlay;
	float			timePassed = 0.0f;

	// Show hide/browser
	bool	browserVisible = false;

	// Settings
	GUI_Browser		browser;

	// Show hide/settings
	bool	settingsVisible = false;

	// Settings
	juce::Component	settingsWrapper { "settings" };
		juce::Viewport	settingsViewport { "viewport" };
			juce::Component	settingsContent { "content" };

	// CRT layout
	std::unordered_map<juce::String, juce::Component*> crtSettingsComponentMap;

	gin::LayoutSupport	crtLayout { *this, [] ( const juce::String& typeName ) { return componentFactory ( typeName ); } };

	void updateDisablers ();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_CRT )
};
//-----------------------------------------------------------------------------
