#include "GUI_ultraView_badge.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_ultraView_Badge::GUI_ultraView_Badge ()
	: juce::Component ( "badge" )
{
	setInterceptsMouseClicks ( false, true );

	addAndMakeVisible ( logoUltraView );
	addAndMakeVisible ( version );

	logoUltraView.onClick = []	{	UI::sendGlobalMessage ( "showAbout" );	};
}
//-----------------------------------------------------------------------------
