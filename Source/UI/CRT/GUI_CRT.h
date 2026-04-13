#pragma once

#include <JuceHeader.h>

#include "libSidplayEZ/src/EZ/SidTuneInfoEZ.h"

#include "UI/Misc/VIC2_Render.h"

#include "GUI_Overlay.h"

#include "UI/Components/GUI_Toggle.h"

#include "Globals/Preferences.h"
#include "Globals/ScreenshotLookup.h"

#include "UI/Misc/ComponentFactory.h"

//-----------------------------------------------------------------------------

class GUI_CRT final
	: public juce::Component
	, private juce::MultiTimer
	, public juce::FileDragAndDropTarget
	, public juce::TextDragAndDropTarget
{
public:
	GUI_CRT ();

	// juce::Component
	void resized () override;
	void mouseWheelMove ( const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel ) override;
	void visibilityChanged () override;

	// juce::FileDragAndDropTarget
	bool isInterestedInFileDrag ( const juce::StringArray& files ) override;
	void filesDropped ( const juce::StringArray& files, int x, int y ) override;

	// juce::TextDragAndDropTarget
	bool isInterestedInTextDrag ( const juce::String& text ) override;
	void textDropped ( const juce::String& text, int x, int y ) override;

	// this
	void setRoot ( const juce::File& _root );
	void setStrings ( const SidTuneInfoEZ& src );
	void reloadOverlayProfile ()	{	overlay.reloadOverlayProfile ();	}

	void loadGameArtwork ( const juce::String& sidname, const juce::String& index = "" );
	void loadGameArtwork ( const int index );

	juce::File getLastLoadedFile ()		{	return lastLoaded;		}
	int getGameArtworkIndex () const	{	return tuneArtIndex;	}

	bool areSettingsVisible () const	{ return settingsVisible;	}
	void showSettings ( const bool visible );
	void setBackgroundColour ( const juce::Colour& bckCol );

	void timerUpdate ( const float secondsPassed, const uint16_t cpuCycles );

	void setCRTPage ( const int page )	{	overlay.setCRTPage ( page );	}
	int getCRTPage () const				{	return overlay.getCRTPage ();	}

private:
	// juce::MultiTimer
	void timerCallback ( int timerID ) override;

	// this
	void renderCRT ( const bool generate = false );
	VIC2_Render::settings getVIC2SettingsFromPreferences () const;
	lime::CRTEmulation::settings getCRTEmulationSettingsFromPreferences () const;

	void updateOverlayCRTSettings ();

	void updateCRTsettingsUI ();
	void updateCRTPalette ( const VIC2_Render::settings& vic2Settings );

	std::pair<juce::File, const int> findArtwork ( const juce::String& sidname, const juce::String& index );

	void connectComponents ();

	SidTuneInfoEZ		sidInfoStr;
	juce::String		sidname;
	juce::File			screenshotRoot;
	juce::File			lastLoaded;

	const colodore				colo;
	colodore::shaderPalette		yuvE_yuvO_yiq;
	VIC2_Render::settings		curVicSettings;

	bool	lastWasGenerated = false;
	bool	lastFirstLuma = false;

	bool	isBasicScreen = false;
	bool	cursorVisible = true;

	juce::SharedResourcePointer<Preferences>		preferences;
	juce::SharedResourcePointer<ScreenshotLookup>	scrshot;

	int							tuneArtIndex = 0;
	std::vector<std::string>	tuneArtwork;

	VIC2_Render		vicRender { true };
	GUI_Overlay		overlay;
	float			timePassed = 0.0f;

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
