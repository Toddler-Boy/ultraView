#pragma once

#include <JuceHeader.h>

#include "ultra-shared/Video/VIC2_Render.h"
#include "ultra-shared/Video/colodore.h"

#include "GUI_Browser.h"
#include "GUI_Overlay.h"

#include "Config/Preferences.h"
#include "ultra-shared/UI/GUI_CRTSettings.h"

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

	// A file under the user Overlays / CRT Masks folders changed (relPath is
	// relative to the user root); hot-reload live tweaks or rescan pick lists
	void userCRTContentChanged ( const juce::String& relPath, gin::FileSystemWatcher::FileSystemEvent event );

	bool isBrowserVisible () const { return browserVisible; }
	void showBrowser ( const bool visible );
	void refreshBrowserEntries ();

	bool areSettingsVisible () const	{ return settingsVisible;	}
	void showSettings ( const bool visible );
	void setBackgroundColour ( const juce::Colour& bckCol );

	void setStreamAddress ( const juce::String& address ) { overlay.setStreamAddress ( address ); }
	void setFirstLuma ( const bool isFirstLuma );

	void showRasterTime ( const bool show );

private:
	// V-blank stuff
	juce::VBlankAttachment	vBlankAttachment { this, [ this ] ( double time ) {	update ( time );	} };

	// juce::MultiTimer
	void timerCallback ( int timerID ) override;

	// this
	void renderCRT ();

	// The panel's settings plus the VIC2-derived fields, pushed into the emulation
	void updateOverlayCRTSettings ();

	void updateCRTPalette ( const VIC2_Render::settings& vic2Settings );

	const colodore				colo;
	colodore::shaderPalette		yuvE_yuvO_yiq;
	VIC2_Render::settings		curVicSettings;

	std::atomic<bool>	streamIsNTSC = false;
	bool	lastFirstLuma = false;

	juce::SharedResourcePointer<Preferences>		preferences;

	GUI_Overlay		overlay;

	// Show hide/browser
	bool	browserVisible = false;

	// Settings
	GUI_Browser		browser;

	// Show hide/settings
	bool	settingsVisible = false;

	// The shared settings panel; the page layout positions it by its
	// component name "settings"
	GUI_CRTSettings	settingsPanel;

	gin::LayoutSupport	crtLayout { *this, [] ( const juce::String& typeName ) { return componentFactory ( typeName ); } };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_CRT )
};
//-----------------------------------------------------------------------------
