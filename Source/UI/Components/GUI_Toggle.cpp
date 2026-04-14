#include "GUI_Toggle.h"

#include "Globals/constants.h"

//----------------------------------------------------------------------------------

GUI_Toggle::GUI_Toggle ( const juce::String& name )
	: juce::ToggleButton ( name )
{
	enablementChanged ();
}
//----------------------------------------------------------------------------------

void GUI_Toggle::enablementChanged ()
{
	juce::ToggleButton::enablementChanged ();

	setAlpha ( isEnabled () ? 1.0f : 0.5f );
	setMouseCursor ( isEnabled () ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::ParentCursor );
}
//----------------------------------------------------------------------------------

void GUI_Toggle::paintButton ( juce::Graphics& g, bool /*isHover*/, bool /*isDown*/ )
{
	auto	b = getLocalBounds ().toFloat ();
	b.reduce ( 0.0f, b.getHeight () * 0.1f );

	//
	// Checkbox
	//
	{
		// Background
		{
			const auto	onCol = findColour ( UI::accent );
			const auto	offCol = UI::getShade ( 0.0f );

			g.setColour ( offCol.interpolatedWith ( onCol, animPosition * animPosition ) );
			g.fillRoundedRectangle ( b, b.getHeight () / 2.0f );
		}

		// Circle
		{
			const auto	r = b.reduced ( b.getHeight () * 0.1f );

			const auto	w = r.getHeight ();
			const auto	circleBounds = r.withWidth ( w ).translated ( ( r.getWidth () - w ) * animPosition, 0.0f ).reduced ( 1.5f );
			const auto	radius = circleBounds.getHeight () / 2.0f;

			g.setColour ( UI::getShade ( animPosition * 0.7f + 0.3f ) );
			g.fillRoundedRectangle ( circleBounds, radius );
		}
	}
}
//----------------------------------------------------------------------------------

void GUI_Toggle::buttonStateChanged ()
{
	const auto	newState = getToggleState () ? 1.0f : 0.0f;

	if ( newState != animPosition )
	{
		if ( isShowing () )
			startTimerHz ( 60 );
		else
			animPosition = newState;
	}
}
//----------------------------------------------------------------------------------

void GUI_Toggle::timerCallback ()
{
	const auto	rate = 0.2f * ( getToggleState () ? 1.0f : -1.0f );

	animPosition = std::clamp ( animPosition + rate, 0.0f, 1.0f );

	if ( animPosition == 0.0f || animPosition == 1.0f )
		stopTimer ();

	repaint ();
}
//----------------------------------------------------------------------------------
