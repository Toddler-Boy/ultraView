#pragma once

#include <JuceHeader.h>

#if ULTRA_INSPECTOR
	#include <melatonin_inspector/melatonin_inspector.h>
#endif

#include "Misc/FX_Helpers.h"

#include "ultra-shared/UI/Components/GUI_TooltipWindow.h"

#include "UI/GUI_Main.h"
#include "ultra-shared/UI/GUI_About.h"

#include "ultra-shared/App/AppUpdater.h"

#include "ultra-shared/Resources/Icons.h"
#include "Config/Preferences.h"
#include "Config/Settings.h"
#include "ultra-shared/Resources/Strings.h"
#include "ultra-shared/Resources/Theme.h"

#include "ultra-shared/Network/AsyncNetwork.h"
#include "ultra-shared/Network/C64u_Scanner.h"
#include "ultra-shared/Network/C64u_UDP_Receiver.h"

//-----------------------------------------------------------------------------

class GUI_ultraView final
	: public juce::AudioAppComponent
	, public juce::DragAndDropContainer
	, public juce::FileDragAndDropTarget
	, public juce::ActionBroadcaster
	, private gin::FileSystemWatcher::Listener
	, private juce::ActionListener
	, private juce::Timer
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
	bool keyPressed ( const juce::KeyPress& key ) override;

	// juce::MouseListener
	void mouseDoubleClick ( const juce::MouseEvent& evt ) override;

	// this
	void setDataRoot ();
	void setUserRoot ();

	// App state
	void saveState ();
	void restoreState ();

	// juce::FileDragAndDropTarget
	bool isInterestedInFileDrag ( const juce::StringArray& files ) override;
	void filesDropped ( const juce::StringArray& files, int x, int y ) override;

private:
	// Our own manager, so initAudio () can set rate and block size.
	// Stays first, it has to outlive the device selector
	juce::AudioDeviceManager	ownedDeviceManager;

	gin::LayoutSupport	layout { *this };

	// juce::ActionListener
	void actionListenerCallback ( const juce::String& message ) override;

	// gin::FileSystemWatcher::Listener
	void fileChanged ( const juce::File& file, gin::FileSystemWatcher::FileSystemEvent event ) override;

	// this
	bool	browserIsVisible = false;
	bool	settingsAreVisible = false;

	void toggleFullscreen ();
	bool wasFullscreen = false;
	bool isFullscreen () const;
	void toFullscreen ();
	void toWindowed ();

	void loadTheme ();

	void updateColors ();

	void findC64OnNetwork ();
	void setupNetworking ();
	void healStreams ();
	void showFirewallNotice ();

	// juce::Timer: the reconnect tick, a no-op while the stream is alive
	void timerCallback () override;

	void reconnectTick ();
	void setTitleStatus ( const juce::String& status );

	std::atomic<bool>	netReceiving = false;
	std::atomic<bool>	netScanning = false;
	std::atomic<bool>	netProbing = false;
	std::atomic<int>	probeFailures = 0;

	// Stream targets as the C64u reported them; a receiver still silent when
	// the watchdog fires gets retargeted to this machine (healStreams)
	juce::String		videoStreamTarget, audioStreamTarget;
	juce::TimedCallback	healWatchdog { [ this ] { healStreams (); } };
	bool				firewallNoticeShown = false;

	// The "ip (hostname)" the title shows while the stream is alive; the
	// receiver thread restores it on regained data
	juce::CriticalSection	titleLock;
	juce::String			connectedTitle;

	void c64_reboot ();
	void c64_run ( const juce::String& type, const juce::MemoryBlock& crtData, const juce::String& filename );
	void c64_forceSystemMode ( const juce::String& mode );
	void c64_forceJoystickSwapper ( const juce::String& mode );

	// The C64u audio stream's rate
	static constexpr auto	internalSamplerate = 48000;

	juce::CriticalSection	inAudio;
	std::atomic<int>		muted = 0;
	SmoothedValue			curOutVol;
	void initAudio ();
	void disableAudio ();
	void enableAudio ();

	// Global objects
	juce::SharedResourcePointer<Icons>			icons;
	juce::SharedResourcePointer<Preferences>	preferences;
	juce::SharedResourcePointer<Settings>		settings;
	juce::SharedResourcePointer<Strings>		strings;
	juce::SharedResourcePointer<Theme>			theme;

	juce::File	dataRoot;
	juce::File	sharedDataRoot;
	juce::File	userRoot;

	GUI_Main		mainScreen;
	GUI_About		aboutScreen;

	AppUpdater		appUpdater { "https://toddler-boy.github.io/ultraView/api" };

	gin::FileSystemWatcher	folderWatcher;

	// 60Hz worth of audio-data (technically only 44100 / 60 are needed, but better safe than sorry)
	int	sampleRate = 0;
    gin::ResamplingFifo			resamplingFifo { 1024 * 5 };

	juce::AudioBuffer<float>	c64uBuffer { 2, 192 };
	gin::AudioFifo				streamFifo;
	juce::AudioBuffer<float>	streamBuffer;
	gin::ResamplingFifo			streamResamplingFifo { 1024 * 5 };
	C64u_UDP_Receiver			c64uReceiver { C64u_UDP_Receiver::streamType::audio };

	// AsyncNetwork
	AsyncNetwork				network;

	// C64 network scanner
	C64uScanner					c64uScanner;

	// Games database
	struct GameEntry
	{
		std::string		name;
		bool			isNTSC = false;
		bool			firstLuma = false;
		bool			firstJoyport = false;
	};
	std::vector<GameEntry>		gamesDatabase;

	void loadGamesDatabase ();
	GameEntry findGameEntry ( juce::String filename ) const;

	// Last loaded was an EasyFlash cartridge
	bool	loadedEasyFlash = true;

	juce::SharedResourcePointer<GUI_TooltipWindow>	tooltipWindow;
	bool	showRasterTime = false;

#if ULTRA_INSPECTOR
	std::unique_ptr<melatonin::Inspector>	inspector;
#endif

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_ultraView )
};
//-----------------------------------------------------------------------------
