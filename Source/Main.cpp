#include <JuceHeader.h>

#include <atomic>

#include "UI/GUI_ultraView.h"
#include "UI/SID_LookAndFeel.h"

#include "Globals/Settings.h"
#include "Globals/constants.h"

#if JUCE_WINDOWS || JUCE_MAC
	extern void setWindowProperties ( void*, unsigned int titleColor );
#endif

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
		mainWindow.reset ( new MainWindow ( getApplicationName () + " " + getApplicationVersion (), laf ) );
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

			// Set up window
			setUsingNativeTitleBar ( true );

			auto	ultra = new GUI_ultraView;

			setContentOwned ( ultra, false );

			setResizeLimits ( 1'180, 700, 100'000, 100'000 );

			setResizable ( true, false );
			setWantsKeyboardFocus ( false );

			// Restore state
			{
				const auto	pos = settings->get<juce::String> ( "UI", "window-position" );
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
			settings->set ( "UI", "window-position", getWindowStateAsString () );
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
	SID_LookAndFeel				laf;
};
//-----------------------------------------------------------------------------

START_JUCE_APPLICATION ( ultraViewApp )
