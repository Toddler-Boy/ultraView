#include "GUI_XYPad.h"
#include "Globals/constants.h"

//-----------------------------------------------------------------------------

static constexpr auto	dotSize = 7;

//-----------------------------------------------------------------------------

GUI_XYPad::GUI_XYPad ( const juce::String& _sectionId, const juce::String& _compId, const juce::Colour col )
	: color ( col )
	, sectionId ( _sectionId )
	, compId ( _compId )
{
	// Recall settings
	value = preferences->get<std::pair<int,int>> ( sectionId, compId );
}
//-----------------------------------------------------------------------------

void GUI_XYPad::paint ( juce::Graphics& g )
{
	auto	bounds = getLocalBounds ().toFloat ();

	// Draw background
	g.setColour ( UI::getShade ( 0.2f ) );
	g.fillRoundedRectangle ( bounds, dotSize / 2.0f );

	// Draw grid
	juce::Path	p;
	p.addRectangle ( bounds.withSizeKeepingCentre ( 1.0f, bounds.getHeight () ) );
	p.addRectangle ( bounds.withSizeKeepingCentre ( bounds.getWidth (), 1.0f ) );

	g.setColour ( UI::getShade ( 0.5f ) );
	g.fillPath ( p );

	g.drawEllipse ( bounds.reduced ( 0.5f ), 1.0f );

	// Draw dot
	const auto	w = ( bounds.getWidth () - dotSize ) / 2.0f;
	const auto	h = ( bounds.getHeight () - dotSize ) / 2.0f;

	const auto	dot = juce::Rectangle<float> { value.first * 0.01f * w + w, value.second * 0.01f * h + h, dotSize, dotSize };

	if ( hover )
	{
		g.setColour ( color.withMultipliedAlpha ( 0.3f ) );
		g.fillEllipse ( dot.expanded ( dotSize / 2.0f ) );
	}

	g.setColour ( color );
	g.fillEllipse ( dot );
}
//-----------------------------------------------------------------------------

void GUI_XYPad::mouseEnter ( const juce::MouseEvent& /*evt*/ )
{
	hover = true;
	setMouseCursor ( juce::MouseCursor::PointingHandCursor );
	repaint ();
}
//-----------------------------------------------------------------------------

void GUI_XYPad::mouseExit ( const juce::MouseEvent& /*evt*/ )
{
	hover = false;
	setMouseCursor ( juce::MouseCursor::NormalCursor );
	repaint ();
}
//-----------------------------------------------------------------------------

void GUI_XYPad::mouseDown ( const juce::MouseEvent& evt )
{
	const auto	changed = evt.mods.isPopupMenu () || evt.mods.isAltDown ();

	if ( evt.mods.isPopupMenu () )
		value = { 0, 0 };

	if ( evt.mods.isAltDown () )
		value = preferences->getDefault<std::pair<int,int>> ( sectionId, compId );

	if ( changed )
	{
		repaint ();
		valueChanged ();
		return;
	}

	updatePosition ( evt );
}
//-----------------------------------------------------------------------------

void GUI_XYPad::mouseDrag ( const juce::MouseEvent& evt )
{
	mouseDown ( evt );
}
//-----------------------------------------------------------------------------

void GUI_XYPad::updatePosition ( const juce::MouseEvent& evt )
{
	const auto	bounds = getLocalBounds ().toFloat ().reduced ( dotSize / 2.0f, dotSize / 2.0f );

	const auto	x = int ( std::clamp ( ( evt.position.x - bounds.getX () ) / bounds.getWidth (), 0.0f, 1.0f ) * 200.0f - 100.0f );
	const auto	y = int ( std::clamp ( ( evt.position.y - bounds.getY () ) / bounds.getHeight (), 0.0f, 1.0f ) * 200.0f - 100.0f );

	if ( x == value.first && y == value.second )
		return;

	value = { x, y };

	repaint ();
	valueChanged ();
}
//-----------------------------------------------------------------------------

void GUI_XYPad::valueChanged ()
{
	if ( ! isTimerRunning () )
		startTimer ( 1000 / 60 );
}
//-----------------------------------------------------------------------------

void GUI_XYPad::timerCallback ()
{
	if ( onValueChange )
		onValueChange ();

	// Store settings
	preferences->set ( sectionId, compId, value );
}
//-----------------------------------------------------------------------------
