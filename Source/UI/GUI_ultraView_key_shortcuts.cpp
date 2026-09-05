#include "GUI_ultraView.h"
#include "ultra-shared/UI/GUI_LookAndFeel.h"

//-----------------------------------------------------------------------------

// The user-facing keys are data (Data/UI/shortcuts.csv, verb per key) and go
// through actionListenerCallback like any other message. The modal Escape and
// the hidden developer combos stay ahead of the lookup, they are not in the
// list by design.

bool GUI_ultraView::keyPressed ( const juce::KeyPress& key )
{
	// Any key arms the focus ring
	focusRing.keyboardUsed ();

	// Tab with the focus on this component itself steps into the children
	if ( key.isKeyCode ( juce::KeyPress::tabKey ) && hasKeyboardFocus ( false ) )
	{
		if ( auto traverser = createKeyboardFocusTraverser () )
		{
			if ( const auto all = traverser->getAllComponents ( this ); ! all.empty () )
				( key.getModifiers ().isShiftDown () ? all.back () : all.front () )->grabKeyboardFocus ();
		}

		return true;
	}

	if ( key == juce::KeyPress ( juce::KeyPress::escapeKey, juce::ModifierKeys::noModifiers, 0 ) )
	{
		if ( aboutScreen.isVisible () )
			actionListenerCallback ( "closeAbout" );
		else if ( shortcutsScreen.isVisible () )
			actionListenerCallback ( "closeShortcuts" );
		else
			return false;

		return true;
	}

#if ULTRA_INSPECTOR
	if ( key == juce::KeyPress ( juce::KeyPress::F11Key, juce::ModifierKeys::ctrlModifier, 0 ) )
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
	else
#endif
	if ( key == juce::KeyPress ( juce::KeyPress::F11Key, juce::ModifierKeys::shiftModifier, 0 ) )
	{
		auto&	laf = static_cast<GUI_LookAndFeel&> ( getLookAndFeel () );

		// Toggle log-window
		const lime::LoggerOptions opts {
			.name = "ultraView",
			.settingsFolder = "ultraView",
			.font = laf.monoFontWithHeight ( 16.0f ),
		};

		auto& lw = lime::Logger::getInstance ()->getLoggingWindow ( opts );
		lw.setVisible ( ! lw.isVisible () );
	}
	else if ( const auto verb = shortcuts->find ( key ); verb.isNotEmpty () )
	{
		actionListenerCallback ( verb );
	}
	else
		return false;

	return true;
}
//-----------------------------------------------------------------------------
