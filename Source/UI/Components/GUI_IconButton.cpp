#include <JuceHeader.h>

#include "GUI_IconButton.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_IconButton::GUI_IconButton ( const juce::String& name, const int _colId )
	: juce::Button ( name )
	, icon ( "actions/" + name.toLowerCase () )
	, colId ( _colId )
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
