#include <JuceHeader.h>

#include <atomic>

#include "UI/GUI_ultraView.h"
#include "ultra-shared/UI/GUI_LookAndFeel.h"

#include "Config/Settings.h"
#include "Globals/constants.h"

#include "ultra-shared/App/AppUpdater.h"
#include "ultra-shared/Helpers/PlatformHelper.h"

//-----------------------------------------------------------------------------

class ultraViewApp : public juce::JUCEApplication
{
public:
	ultraViewApp ()
	{
		juce::Logger::setCurrentLogger ( lime::Logger::getInstance () );
	}

	~ultraViewApp () override
	{
		juce::Logger::setCurrentLogger ( nullptr );
	}

	const juce::String getApplicationName () override { return ProjectInfo::projectName; }
	const juce::String getApplicationVersion () override { return ProjectInfo::versionString; }
	bool moreThanOneInstanceAllowed () override { return false; }

	//-----------------------------------------------------------------------------

	void initialise ( const juce::String& /*commandLine*/ ) override
	{
		mainWindow = std::make_unique<MainWindow> ( helpers::appTitle (), laf );

		// The Authenticode digest seals code and appended pak in one hash, so
		// this doubles as a data-integrity check. Off the message thread, the
		// verify hashes the whole exe. Hard-coded text: a corrupted file must
		// not source its error message from its own (corrupted) data
		juce::Thread::launch ( juce::Thread::Priority::low, [ this ]
		{
			if ( verifyExecutableSignature () != SignatureState::corrupted )
				return;

			Z_ERR ( "Executable signature check failed, the file is corrupted" );

			juce::MessageManager::callAsync ( [ this ]
			{
				mainWindow->setVisible ( false );

				juce::NativeMessageBox::showMessageBoxAsync ( juce::MessageBoxIconType::WarningIcon,
					"ultraView is damaged",
					"The ultraView program file is corrupted. Please download and install it again.",
					nullptr,
					juce::ModalCallbackFunction::create ( [] ( int ) { juce::JUCEApplication::quit (); } ) );
			} );
		} );
	}
	//-----------------------------------------------------------------------------

	void shutdown () override
	{
		mainWindow = nullptr;
	}
	//-----------------------------------------------------------------------------

	void systemRequestedQuit () override
	{
		quit ();
	}
	//-----------------------------------------------------------------------------

	void anotherInstanceStarted ( const juce::String& /*commandLine*/ ) override
	{
	}
	//-----------------------------------------------------------------------------

	class MainWindow : public juce::DocumentWindow
	{
	public:
		MainWindow ( juce::String name, juce::LookAndFeel& laf )
			: juce::DocumentWindow ( name, juce::Colours::black, juce::DocumentWindow::allButtons )
		{
			juce::LookAndFeel::setDefaultLookAndFeel ( &laf );
			juce::Desktop::setScreenSaverEnabled ( false );

			// Set up window
			setUsingNativeTitleBar ( true );

			auto	ultra = new GUI_ultraView;
			setContentOwned ( ultra, false );

			setResizeLimits ( 1'000, 500, 100'000, 100'000 );

			setResizable ( true, false );
			setWantsKeyboardFocus ( false );

			// Restore state
			{
				const auto	pos = settings->get<juce::String> ( "ui/window-position" );
				restoreWindowStateFromString ( pos );

				// First start, center window and set size
				if ( pos.isEmpty () )
				{
					const auto&	displays = juce::Desktop::getInstance ().getDisplays ();
					const auto*	display = displays.getDisplayForRect ( getScreenBounds () );

					if ( display )
					{
						const auto	b = display->userBounds.toNearestIntEdges ();
						centreWithSize ( std::clamp ( b.getWidth () - 100, 890, 1280 ),
										 std::clamp ( b.getHeight () - 100, 700, 100'000 ) );
					}
				}
				else
				{
					// If window is out of screen to the top or left, center it
					if ( const auto rect = getBounds (); rect.getX () < 0 || rect.getY () < 0 )
						centreWithSize ( rect.getWidth (), rect.getHeight () );
				}
			}

			ultra->sendActionMessage ( "restoreState" );

			setVisible ( true );
			bringWindowToForeground ( getWindowHandle () );
		}

		~MainWindow () override
		{
			lime::Logger::getInstance ()->closeLoggingWindow ();
		}

		#if JUCE_WINDOWS || JUCE_MAC
			void parentHierarchyChanged () override
			{
				juce::DocumentWindow::parentHierarchyChanged ();
				setBorderColor ();
			}
		#endif

		void colourChanged () override
		{
			juce::DocumentWindow::colourChanged ();
			setBorderColor ();
		}

		void closeButtonPressed () override
		{
			auto	content = dynamic_cast<GUI_ultraView*> ( getContentComponent () );
			content->saveState ();
			JUCEApplication::getInstance ()->systemRequestedQuit ();
		}

		void moved () override
		{
			juce::DocumentWindow::moved ();

			// Pre-visible moves are construction artifacts (macOS pushes the
			// window below the menu bar); saving one would clobber the stored
			// position before the constructor restores it
			if ( ! isVisible () )
				return;

			if ( auto cc = getContentComponent () )
			{
				cc->moved ();
				saveState ();
			}
		}

		void resized () override
		{
			juce::DocumentWindow::resized ();

			if ( ! isVisible () )
				return;

			saveState ();
		}

	private:
		void saveState ()
		{
			settings->set ( "ui/window-position", getWindowStateAsString () );
		}

		void setBorderColor ()
		{
			if ( auto peer = getPeer () )
				setWindowProperties ( peer->getNativeHandle (), getBackgroundColour ().getARGB () );
		}

		juce::SharedResourcePointer<Settings>	settings;

		juce::Colour	titleColor { 0xFF10141C };

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( MainWindow )
	};

private:
	std::unique_ptr<MainWindow> mainWindow;
	GUI_LookAndFeel				laf;
};
//-----------------------------------------------------------------------------

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ( "-Wmissing-prototypes" )
JUCE_CREATE_APPLICATION_DEFINE ( ultraViewApp )

// An installed update relaunches once JUCE has released the instance lock
JUCE_MAIN_FUNCTION
{
	juce::JUCEApplicationBase::createInstance = &juce_CreateApplication;

	const auto	result = juce::JUCEApplicationBase::main ( JUCE_MAIN_FUNCTION_ARGS );

	AppUpdater::relaunchIfInstalled ();

	return result;
}
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
