#include "GUI_Disabler.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_Disabler::GUI_Disabler ()
{
	setInterceptsMouseClicks ( false, true );
}
//-----------------------------------------------------------------------------

void GUI_Disabler::enablementChanged ()
{
	const auto	enabled = isEnabled ();

	setAlpha ( enabled ? 1.0f : UI::disabledAlpha );
	setInterceptsMouseClicks ( false, enabled );
}
//-----------------------------------------------------------------------------
