#pragma once

#include <JuceHeader.h>

#include <melatonin_inspector/melatonin_inspector.h>

#include "Misc/FX_Helpers.h"

//#include "UI/Badge/GUI_ultraSID_badge.h"
#include "UI/GUI_TooltipWindow.h"

#include "UI/GUI_Main.h"

#include "Globals/Icons.h"
#include "Globals/Preferences.h"
#include "Globals/Settings.h"
#include "Globals/Strings.h"
#include "Globals/Theme.h"

#include "Network/C64u_UDP_Receiver.h"
#include "Network/AsyncNetwork.h"

//-----------------------------------------------------------------------------

class GUI_ultraView final
	: public juce::AudioAppComponent
	, public juce::DragAndDropContainer
	, public juce::FileDragAndDropTarget
	, public juce::ActionBroadcaster
	, private gin::FileSystemWatcher::Listener
	, private juce::ActionListener
{
public:
	GUI_ultraView ();
	~GUI_ultraView () override;

	// juce::AudioAppComponent
	void prepareToPlay ( int samplesPerBlockExpected, double sampleRate ) override;
	void getNextAudioBlock ( const juce::AudioSourceChannelInfo& bufferToFill ) override;
	void releaseResources () override;

	// juce::Component
	void resized () override;

	// juce::MouseListener
	void mouseDoubleClick ( const juce::MouseEvent& evt ) override;

	// this
	void setDataRoot ();
	void setUserRoot ();

	bool isDataRootValid () const;

	// App state
	void saveState ();
	void restoreState ();

	// juce::FileDragAndDropTarget
	bool isInterestedInFileDrag ( const juce::StringArray& files ) override;
	void filesDropped ( const juce::StringArray& files, int x, int y ) override;

private:
	gin::LayoutSupport	layout { *this };

	// juce::ActionListener
	void actionListenerCallback ( const juce::String& message ) override;

	// gin::FileSystemWatcher::Listener
	void fileChanged ( const juce::File& file, gin::FileSystemWatcher::FileSystemEvent event ) override;

	// juce::Component
	bool keyPressed ( const juce::KeyPress& key ) override;

	// this
	bool	settingsAreVisible = false;

	void toggleFullscreen ();
	bool wasFullscreen = false;
	bool isFullscreen () const;
	void toFullscreen ();
	void toWindowed ();

	void loadTheme ();

	void updateColors ();

	void setupNetworking ();

	juce::CriticalSection	inAudio;
	std::atomic<int>		muted = 0;
	SmoothedValue			curOutVol;
	void disableAudio ();
	void enableAudio ();

	// Global objects
	juce::SharedResourcePointer<Icons>			icons;
	juce::SharedResourcePointer<Preferences>	preferences;
	juce::SharedResourcePointer<Settings>		settings;
	juce::SharedResourcePointer<Strings>		strings;
	juce::SharedResourcePointer<Theme>			theme;

	juce::File	dataRoot;
	juce::File	userRoot;

	GUI_Main		mainScreen;

	gin::FileSystemWatcher	folderWatcher;

	// 60Hz worth of audio-data (technically only 44100 / 60 are needed, but better safe than sorry)
	int	sampleRate = 0;
    gin::ResamplingFifo			resamplingFifo { 1024 * 5 };

	C64u_UDP_Receiver			c64uReceiver { C64u_UDP_Receiver::streamType::audio };
	juce::AudioBuffer<float>	c64uBuffer { 2, 192 };

	gin::AudioFifo				streamFifo;
	juce::AudioBuffer<float>	streamBuffer;
	gin::ResamplingFifo			streamResamplingFifo { 1024 * 5 };

	// AsyncNetwork
	AsyncNetwork				network;

	juce::SharedResourcePointer<GUI_TooltipWindow>	tooltipWindow;

	std::unique_ptr<melatonin::Inspector>	inspector;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_ultraView )
};
//-----------------------------------------------------------------------------
