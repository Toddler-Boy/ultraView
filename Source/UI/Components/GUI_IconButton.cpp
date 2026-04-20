#include <JuceHeader.h>

#include "GUI_IconButton.h"

#include "UI/Misc/GUI_RoundedClip.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_IconButton::GUI_IconButton ( const juce::String& name, const int _colId, const int _timeoutMs )
	: juce::Button ( name )
	, icon ( "actions/" + name.toLowerCase () )
	, colId ( _colId )
	, timeoutMs ( _timeoutMs )
{
	setName ( name );
	setButtonText ( "actions/" + name );
	setWantsKeyboardFocus ( false );
}
//-----------------------------------------------------------------------------

void GUI_IconButton::paintButton ( juce::Graphics& g, bool hover, bool /*down*/ )
{
	auto	b = getLocalBounds ().toFloat ();

	const auto	col = findColour ( colId ).withMultipliedAlpha ( hover ? 1.0f : 0.5f );

	// Draw background
	g.setColour ( col );
	g.fillRoundedRectangle ( b, UI::bentoRadius );

	if ( timeoutMs && progress > 0.0f )
	{
		const GUI_RoundedClip	clip ( g, b, UI::bentoRadius );

		g.setColour ( col.withMultipliedSaturation ( 2.0f ).withMultipliedBrightness ( 2.0f ) );
		g.fillRect ( b.withWidth ( b.getWidth () * progress ) );
	}

	g.setColour ( juce::Colours::white.withAlpha ( 0.05f ) );
	g.drawRoundedRectangle ( b.reduced ( 0.5f ), UI::bentoRadius - 0.5f, 1.0f );

	// Calculate layout
	const auto& f = UI::font ( b.getHeight () * 0.5f, 600 );

	const auto	txt = strings->get ( getButtonText () );

	const auto	iconWidth = b.getHeight () / 2.5f;
	const auto	iconGap = iconWidth * 0.5f;
	const auto	txtWidth = juce::GlyphArrangement::getStringWidth ( f, txt );

	const auto	totalWidth = iconWidth + iconGap + txtWidth;

	b = b.withSizeKeepingCentre ( totalWidth, b.getHeight () );

	g.setColour ( col.interpolatedWith ( juce::Colours::white, hover ? 0.9f : 0.5f ) );

	// Draw icon
	g.fillPath ( UI::getScaledPath ( icons->get ( icon ), b.removeFromLeft ( iconWidth ).withSizeKeepingCentre ( iconWidth, iconWidth ) ) );
	b.removeFromLeft ( iconGap );

	// Draw text
	{
		g.setFont ( f );
		g.drawText ( txt, b, juce::Justification::centredLeft, false );
	}
}
//-----------------------------------------------------------------------------

void GUI_IconButton::enablementChanged ()
{
	setAlpha ( isEnabled () ? 1.0f : 0.5f );
}
//-----------------------------------------------------------------------------

void GUI_IconButton::mouseDown ( const juce::MouseEvent& evt )
{
	if ( ! timeoutMs )
	{
		juce::Button::mouseDown ( evt );
		return;
	}

	startTime = std::chrono::steady_clock::now ();
	targetTime = startTime + std::chrono::milliseconds ( timeoutMs );

	startTimerHz ( 60 );
}
//-----------------------------------------------------------------------------

void GUI_IconButton::mouseUp ( const juce::MouseEvent& evt )
{
	if ( ! timeoutMs )
	{
		juce::Button::mouseUp ( evt );
		return;
	}

	stopTimer ();

	const auto	finished = progress >= 1.0f;

	progress = 0.0f;
	repaint ();

	if ( finished )
	{
		juce::Button::mouseUp ( evt );

		if ( isMouseOver () )
			triggerClick ();
	}
}
//-----------------------------------------------------------------------------

void GUI_IconButton::timerCallback ()
{
	const auto	now = std::chrono::steady_clock::now ();

	const auto	isOver = isMouseOver ();
	if ( now >= targetTime || ! isOver )
	{
		stopTimer ();
		progress = isOver ? 1.0f : 0.0f;
		repaint ();
		return;
	}

	// Calculate durations
	std::chrono::duration<float>	elapsed = now - startTime;
	std::chrono::duration<float>	total = targetTime - startTime;

	// Normalize and clamp between 0 and 1
	progress = std::clamp ( elapsed.count () / total.count (), 0.0f, 1.0f );

	repaint ();
}
//-----------------------------------------------------------------------------
