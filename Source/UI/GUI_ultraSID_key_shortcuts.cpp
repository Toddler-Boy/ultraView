#include "GUI_ultraSID.h"
#include "SID_LookAndFeel.h"

//-----------------------------------------------------------------------------

bool GUI_ultraSID::keyPressed ( const juce::KeyPress& key )
{
	if ( key == juce::KeyPress ( juce::KeyPress::upKey, juce::ModifierKeys::commandModifier, 0 ) )			mainScreen.footer.volume.changeVolume ( 5.0 );
	else if ( key == juce::KeyPress ( juce::KeyPress::downKey, juce::ModifierKeys::commandModifier, 0 ) )	mainScreen.footer.volume.changeVolume ( -5.0 );
	else if ( key == juce::KeyPress ( 'M', juce::ModifierKeys::commandModifier, 0) )						mainScreen.footer.volume.mute.triggerClick ();
	else if ( key == juce::KeyPress ( 'F', juce::ModifierKeys::commandModifier, 0 ) )						mainScreen.footer.volume.quality.triggerClick ();
	else if ( key == juce::KeyPress ( juce::KeyPress::leftKey, juce::ModifierKeys::commandModifier, 0 ) )	mainScreen.footer.transport.previous.triggerClick ();
	else if ( key == juce::KeyPress ( juce::KeyPress::rightKey, juce::ModifierKeys::commandModifier, 0 ) )	mainScreen.footer.transport.next.triggerClick ();
	else if ( key == juce::KeyPress ( 'S', juce::ModifierKeys::commandModifier, 0 ) )						mainScreen.footer.transport.shuffle.triggerClick ();
	else if ( key == juce::KeyPress ( 'R', juce::ModifierKeys::commandModifier, 0 ) )						mainScreen.footer.transport.repeat.triggerClick ();
	else if ( key == juce::KeyPress ( juce::KeyPress::leftKey, juce::ModifierKeys::shiftModifier, 0 ) )		mainScreen.footer.transport.seekRelative ( -5.0 );
	else if ( key == juce::KeyPress ( juce::KeyPress::rightKey, juce::ModifierKeys::shiftModifier, 0 ) )	mainScreen.footer.transport.seekRelative ( 5.0 );
	else if ( key == juce::KeyPress ( juce::KeyPress::spaceKey, juce::ModifierKeys::noModifiers, 0 ) )		mainScreen.footer.transport.play.triggerClick ();
	else if ( (		key == juce::KeyPress ( juce::KeyPress::returnKey, juce::ModifierKeys::altModifier, 0 )
				||	key == juce::KeyPress ( juce::KeyPress::F11Key, juce::ModifierKeys::noModifiers, 0 ) )
			  && mainScreen.pages.crtPage.isVisible () )
	{
		toggleFullscreen ();
	}
	else if ( key == juce::KeyPress ( 'L', juce::ModifierKeys::commandModifier, 0 ) )
	{
		showPage ( "search" );
		mainScreen.pages.showSearch ();
	}
	else if ( key == juce::KeyPress ( '1', juce::ModifierKeys::altModifier | juce::ModifierKeys::shiftModifier, 0 ) )
	{
		showPage ( "playlists" );
		mainScreen.pages.showPlaylist ( "" );
		mainScreen.pages.setPage ( "playlists" );
	}
	else if ( key == juce::KeyPress ( juce::KeyPress::F11Key, juce::ModifierKeys::noModifiers, 0 ) )
	{
		// Toggle inspector
		if ( ! inspector )
		{
			inspector = std::make_unique<melatonin::Inspector> ( *this );
			inspector->setRootFollowsComponentUnderMouse ( true );
			inspector->setVisible ( true );
			inspector->setAlwaysOnTop ( true );
			inspector->onClose = [ this ] { inspector = nullptr; };
		}
		else
		{
			inspector = nullptr;
		}
	}
	else if ( key == juce::KeyPress ( juce::KeyPress::F11Key, juce::ModifierKeys::shiftModifier, 0 ) )
	{
		auto&	laf = static_cast<SID_LookAndFeel&> ( getLookAndFeel () );

		// Toggle log-window
		const lime::LoggerOptions opts {
			.name = "ultraSID",
			.settingsFolder = "ultraSID",
			.font = laf.monoFontWithHeight ( 16.0f ),
		};

		auto& lw = lime::Logger::getInstance ()->getLoggingWindow ( opts );
		lw.setVisible ( ! lw.isVisible () );
	}
	else if ( key == juce::KeyPress ( juce::KeyPress::F11Key, juce::ModifierKeys::commandModifier, 0 ) )
	{
		// Toggle peak-meters
		const auto	visible = ! inputMeter[ 0 ].isVisible ();

		inputMeter[ 0 ].setVisible ( visible );
		inputMeter[ 1 ].setVisible ( visible && player.getNumChips () > 1 );
		outputMeter[ 0 ].setVisible ( visible );
		outputMeter[ 1 ].setVisible ( visible );
	}
	else
		return false;

	return true;
}
//-----------------------------------------------------------------------------
