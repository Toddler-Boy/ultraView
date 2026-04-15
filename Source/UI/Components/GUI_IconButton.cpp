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

void GUI_IconButton::paintButton ( juce::Graphics& g, bool hover, bool )
{
	auto	b = getLocalBounds ().toFloat ();

	const auto	col = findColour ( colId ).withMultipliedAlpha ( hover ? 1.0f : 0.5f );

	// Draw background
	g.setColour ( col );
	g.fillRoundedRectangle ( b, UI::bentoRadius );

	g.setColour ( juce::Colours::white.withAlpha ( 0.1f ) );
	g.drawRoundedRectangle ( b.reduced ( 0.5f ), UI::bentoRadius - 0.5f, 1.0f );

	g.setColour ( col.interpolatedWith ( juce::Colours::white, hover ? 0.9f : 0.5f ) );

	const auto& f = UI::font ( 20.0f, 600 );

	const auto	txt = strings->get ( getButtonText () );

	constexpr auto	iconWidth = 16.0f;
	constexpr auto	iconGap = 8.0f;
	const auto		txtWidth = juce::GlyphArrangement::getStringWidth ( f, txt );

	const auto	totalWidth = iconWidth + iconGap + txtWidth;

	b = b.withSizeKeepingCentre ( totalWidth, b.getHeight () );

	// Draw icon
	g.fillPath ( UI::getScaledPath ( icons->get ( icon ), b.removeFromLeft ( iconWidth ).withSizeKeepingCentre ( iconWidth, iconWidth ) ) );
	b.removeFromLeft ( iconGap );

	// Draw text
	{
		g.setFont ( UI::font ( 20.0f, 600 ) );
		g.drawText ( txt, b, juce::Justification::centredLeft, false );
	}
}
//-----------------------------------------------------------------------------

void GUI_IconButton::enablementChanged ()
{
	setAlpha ( isEnabled () ? 1.0f : 0.5f );
}
//-----------------------------------------------------------------------------
