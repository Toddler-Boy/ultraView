#include "GUI_ultraView.h"

//-----------------------------------------------------------------------------

bool GUI_ultraView::isInterestedInFileDrag ( const juce::StringArray& files )
{
	if ( files.size () != 1 )
		return false;

	return files[ 0 ].endsWithIgnoreCase ( ".crt" ) || files[ 0 ].endsWithIgnoreCase ( ".prg" );
}
//-----------------------------------------------------------------------------

void GUI_ultraView::filesDropped ( const juce::StringArray& files, int /*x*/, int /*y*/ )
{
	UI::sendGlobalMessage ( "c64run {}", files[ 0 ].quoted () );
}
//-----------------------------------------------------------------------------
