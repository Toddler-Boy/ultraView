#pragma once

#include <JuceHeader.h>

#include <melatonin_inspector/melatonin_inspector.h>

#include "Footer/GUI_Footer.h"

#include "Globals/ScreenshotLookup.h"
#include "Globals/ThumbnailCache.h"

#include "UI/Badge/GUI_ultraSID_badge.h"
#include "UI/GUI_TooltipWindow.h"

#include "UI/Pages/GUI_Pages.h"
#include "UI/SidebarLeft/GUI_SidebarLeft.h"
#include "UI/SidebarRight/GUI_SidebarRight.h"

#include "UI/Screens/GUI_Main.h"
#include "UI/Screens/GUI_Onboarding.h"
#include "UI/Screens/GUI_UpdateHVSC.h"

#include "Database/Database.h"
#include "Database/HVSCDatabase.h"

#include "SID/SIDEffects.h"
#include "SID/SIDPlayer.h"

#include "Globals/Icons.h"
#include "Globals/InstallState.h"
#include "Globals/Likes.h"
#include "Globals/Playlists.h"
#include "Globals/Preferences.h"
#include "Globals/Settings.h"
#include "Globals/Strings.h"
#include "Globals/Tags.h"
#include "Globals/Theme.h"

#include "UI/Pages/CRT/C64u_UDP_Receiver.h"

//-----------------------------------------------------------------------------

class GUI_ultraSID final
	: public juce::AudioAppComponent
	, public juce::DragAndDropContainer
	, public juce::FileDragAndDropTarget
	, public juce::TextDragAndDropTarget
	, public juce::ActionBroadcaster
	, private gin::FileSystemWatcher::Listener
	, private juce::ActionListener
	, public juce::Thread
{
public:
	GUI_ultraSID ();
	~GUI_ultraSID () override;

	// juce::AudioAppComponent
	void prepareToPlay ( int samplesPerBlockExpected, double sampleRate ) override;
	void getNextAudioBlock ( const juce::AudioSourceChannelInfo& bufferToFill ) override;
	void releaseResources () override;

	// juce::Component
	void moved () override;
	void resized () override;
	void parentHierarchyChanged () override;

	// juce::MouseListener
	void mouseDoubleClick ( const juce::MouseEvent& evt ) override;
	void mouseDown ( const juce::MouseEvent& evt ) override;

	// juce::VBlankAttachment
	void update ( double time );

	// juce:Thread
	void run () override;

	// this
	void setHVSCRoot ();
	void setDataRoot ();
	void setUserRoot ();

	bool isHVSCRootValid () const;
	bool isDataRootValid () const;
	bool areRootsValid () const		{	return isHVSCRootValid () && isDataRootValid (); }

	void loadDatabase ();
	void loadROMs ();
	void preProcessSTIL ( const juce::String& filename, const unsigned int mainSong );

	// Error messages
	void checkHVSCStatus ();
	void setHVSCStatus ( GUI_SettingsLocationStatus::Status status, const juce::String& message );
	void checkDatabaseStatus ();

	// Playlist/transport
	void loadTune ( const juce::String& name, const int subtune, const juce::String& src, const int playlistPosition = -1 );
	[[ nodiscard ]] bool loadSong ( const juce::String& filename );
	void initSong ( unsigned int songNum, const bool subTuneOnly );
	void renderSong ();

	void togglePause ();

	// App state
	void saveState ();
	void restoreState ();

	// juce::FileDragAndDropTarget
	bool isInterestedInFileDrag ( const juce::StringArray& files ) override;
	void filesDropped ( const juce::StringArray& files, int x, int y ) override;

	// juce::TextDragAndDropTarget
	bool isInterestedInTextDrag ( const juce::String& text ) override;
	void textDropped ( const juce::String& text, int x, int y ) override;

private:
	gin::LayoutSupport	layout { *this };
	std::once_flag		firstStart;

	// V-blank stuff
	juce::VBlankAttachment	vBlankAttachment { this, [ this ] ( double time )	{	update ( time );	} };
	double	lastTimer = 0.0;

	// juce::ActionListener
	void actionListenerCallback ( const juce::String& message ) override;

	// gin::FileSystemWatcher::Listener
	void fileChanged ( const juce::File& file, gin::FileSystemWatcher::FileSystemEvent event ) override;

	// juce::Component
	bool keyPressed ( const juce::KeyPress& key ) override;

	// this
	std::string		lastPage;
	bool			settingsAreVisible = false;

	void toggleFullscreen ();
	bool wasFullscreen = false;
	bool isFullscreen () const;
	void toFullscreen ();
	void toWindowed ();

	std::string getFullFilename ( const juce::String& filename );

	void showPage ( const std::string& name );
	void showPlaylist ( const std::string& name );
	void updateTransportButtons ();
	bool isOnboardingOrUpdating () const { return onboardingScreen.isVisible () || updateHVSCScreen.isVisible (); }
	void loadTheme ();
	void loadSIDPlayerProfilesAndOverrides ();

	void updateVoices ();
	void updateVolume ();
	void updateFooterThumbnail ( const std::string& tunename, const bool isNTSC );
	void updateColors ();

	void updateFX ();

	juce::CriticalSection	inAudio;
	std::atomic<int>		muted = 0;
	SmoothedValue			curOutVol;
	void disableAudio ();
	void enableAudio ();

	unsigned int	lastSong = 0;
	std::string		lastFilename;

	// Playlist stuff
	int	playlistPosition = -1;
	int	playlistPlayPosition = -1;
	int	curSubtune = 0;
	void updatePlaylistPosition ();
	void nextPreviousPlaylistItem ( const int delta );

	// Sub-tunes
	void playSubtune ( const int subtune );

	// Downloading
	void downloadInfo ();
	void downloadHVSC_update ();
	void downloadHVSC_full ();
	void stopFullHVSCInstall ();
	void stopHVSCupdate ();
	void downloadDatabase_full ();

	void downloadScreenshot ( const juce::String& dlUrl );
	void downloadCoverImage ( const juce::String& targetPlaylist, const juce::String& dlUrl );

	// Global objects
	juce::SharedResourcePointer<Icons>			icons;
	juce::SharedResourcePointer<InstallState>	installState;
	juce::SharedResourcePointer<Likes>			likes;
	juce::SharedResourcePointer<Playlists>		playlists;
	juce::SharedResourcePointer<Preferences>	preferences;
	juce::SharedResourcePointer<Settings>		settings;
	juce::SharedResourcePointer<Strings>		strings;
	juce::SharedResourcePointer<Tags>			tags;
	juce::SharedResourcePointer<Theme>			theme;

	juce::File	hvscRoot;
	juce::File	dataRoot;
	juce::File	userRoot;

	juce::SharedResourcePointer<HVSC_database>		hvscDatabase;
	juce::SharedResourcePointer<Database>			database;
	juce::SharedResourcePointer<UserDatabase>		userDatabase;
	juce::SharedResourcePointer<ThumbnailCache>		thumbnailCache;
	juce::SharedResourcePointer<ScreenshotLookup>	screenshots;

	SIDPlayer		player;
	SIDEffects		dspEffects;

	GUI_Main		mainScreen;
	GUI_Onboarding	onboardingScreen;
	GUI_UpdateHVSC	updateHVSCScreen;

	gin::LevelMeter		inputMeter[ 2 ] = { dspEffects.inputLevel[ 0 ],dspEffects.inputLevel[ 1 ] };
	gin::LevelMeter		outputMeter[ 2 ] = { dspEffects.outputLevel[ 0 ], dspEffects.outputLevel[ 1 ] };

	gin::FileSystemWatcher	folderWatcher;

	// 60Hz worth of audio-data (technically only 44100 / 60 are needed, but better safe than sorry)
	int	sampleRate = 0;
	juce::AudioBuffer<float>	sidBuffer;
    gin::ResamplingFifo			resamplingFifo { 1024 * 5 };

	C64u_UDP_Receiver			c64uReceiver { C64u_UDP_Receiver::streamType::audio };
	juce::AudioBuffer<float>	c64uBuffer { 2, 192 };

	gin::AudioFifo				streamFifo;
	juce::AudioBuffer<float>	streamBuffer;
	gin::ResamplingFifo			streamResamplingFifo { 1024 * 5 };

	juce::SharedResourcePointer<GUI_TooltipWindow>	tooltipWindow;

	void addScreenshots ( const juce::StringArray& filenames );
	void addSidTunes ( const juce::StringArray& filenames );

	void assignBorderColor ( const int index );
	void toggleFirstLuma ();
	void toggleFirstLumaAll ();
	void toggleThumbnail ();
	void deleteImage ();

	std::unique_ptr<melatonin::Inspector>	inspector;

	gin::DownloadManager	downloader { 5 * 1000, 5 * 1000 };
	juce::MemoryBlock		downloadedData;

	enum class threadTask : int8_t
	{
		none,
		HVSC_update,
		HVSC_full,
	};

	threadTask		threadTask = threadTask::none;

	void extractHVSC_Update ( const juce::MemoryBlock& data );
	void extractHVSC_Full ( const juce::MemoryBlock& data );

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_ultraSID )
};
//-----------------------------------------------------------------------------
