#include "GUI_ultraView.h"
#include "SID_LookAndFeel.h"

//-----------------------------------------------------------------------------

bool GUI_ultraView::keyPressed ( const juce::KeyPress& key )
{
	if ( key == juce::KeyPress ( juce::KeyPress::F12Key, juce::ModifierKeys::noModifiers, 0 ) )
	{
		showRasterTime = ! showRasterTime;
		mainScreen.crt.showRasterTime ( showRasterTime );
	}
	else if ( (		key == juce::KeyPress ( juce::KeyPress::returnKey, juce::ModifierKeys::altModifier, 0 )
				||	key == juce::KeyPress ( juce::KeyPress::F11Key, juce::ModifierKeys::noModifiers, 0 )
			 )
				&&	mainScreen.crt.isVisible () )
	{
		toggleFullscreen ();
	}
	else if ( key == juce::KeyPress ( juce::KeyPress::F11Key, juce::ModifierKeys::ctrlModifier, 0 ) )
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
			.name = "ultraView",
			.settingsFolder = "ultraView",
			.font = laf.monoFontWithHeight ( 16.0f ),
		};

		auto& lw = lime::Logger::getInstance ()->getLoggingWindow ( opts );
		lw.setVisible ( ! lw.isVisible () );
	}
	else
		return false;

	return true;
}
//-----------------------------------------------------------------------------
