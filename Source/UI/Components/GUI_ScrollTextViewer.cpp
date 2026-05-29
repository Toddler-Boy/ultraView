#include "GUI_ScrollTextViewer.h"

//-----------------------------------------------------------------------------

void GUI_LargeTextView::setText ( const juce::String& text )
{
	lines = juce::StringArray::fromLines ( text.trim () );
	linkButtons.clear ();

	for ( const auto& line : lines )
	{
		if ( ! isLineALink ( line ) )
			continue;

		const auto	url = juce::URL { line.trim () };
		const auto	btn = new juce::HyperlinkButton { line, url };

		btn->setName ( "linkButton" );
		btn->setFont ( textFont, false, juce::Justification::left );

		linkButtons.add ( btn );
		addAndMakeVisible ( btn );
	}

	updateLayout ();
}
//-----------------------------------------------------------------------------

void GUI_LargeTextView::setFont ( const juce::Font& newFont )
{
	textFont = newFont;

	rowHeight = juce::roundToInt ( textFont.getHeight () * 1.25f );

	for ( const auto btn : linkButtons )
		btn->setFont ( textFont, false, juce::Justification::left );

	updateLayout ();
}
//-----------------------------------------------------------------------------

void GUI_LargeTextView::resized ()
{
	updateButtonPositions ();
}
//-----------------------------------------------------------------------------

void GUI_LargeTextView::paint ( juce::Graphics& g )
{
	if ( lines.isEmpty () )
		return;

	if ( rowHeight <= 0 )
		return;

	const auto	clipBounds = g.getClipBounds ().expanded ( 0, 10 );

	const auto	firstVisibleIndex = std::max ( 0, clipBounds.getY () / rowHeight );
	const auto	lastVisibleIndex = std::min ( lines.size () - 1, ( clipBounds.getBottom () + rowHeight - 1 ) / rowHeight );

	g.setFont ( textFont );
	g.setColour ( findColour ( juce::Label::textColourId, true ) );

	for ( auto i = firstVisibleIndex; i <= lastVisibleIndex; ++i )
	{
		if ( isLineALink ( lines[ i ] ) )
			continue;

		g.drawText ( lines[ i ], 10, i * rowHeight + 10, getWidth () - 20, rowHeight, juce::Justification::left, false );
	}
}
//-----------------------------------------------------------------------------

bool GUI_LargeTextView::isLineALink ( const juce::String& text )
{
	return text.contains ( "http://" ) || text.contains ( "https://" );
}
//-----------------------------------------------------------------------------

void GUI_LargeTextView::updateLayout ()
{
	setSize ( getWidth (), lines.size () * rowHeight + 20 );

	updateButtonPositions ();
	repaint ();
}
//-----------------------------------------------------------------------------

void GUI_LargeTextView::updateButtonPositions ()
{
	auto	buttonIndex = 0;

	for ( auto i = 0; i < lines.size (); ++i )
	{
		if ( ! isLineALink ( lines[ i ] ) )
			continue;

		if ( const auto btn = linkButtons[ buttonIndex++ ] )
		{
			const auto yPosition = juce::roundToInt ( i * rowHeight ) + 10;
			btn->setBounds ( 10, yPosition, getWidth () - 20, juce::roundToInt ( rowHeight ) );
		}
	}
}
//-----------------------------------------------------------------------------

GUI_ScrollTextViewer::GUI_ScrollTextViewer ()
{
	viewport.setScrollBarsShown ( true, false );
	viewport.setViewedComponent ( &textRenderer, false );

	addAndMakeVisible ( viewport );
}
//-----------------------------------------------------------------------------

void GUI_ScrollTextViewer::paint ( juce::Graphics& g )
{
	const auto	b = getLocalBounds ().toFloat ().withTrimmedRight ( 20.0f );

	g.setColour ( findColour ( juce::Label::textColourId, true ).withMultipliedAlpha ( 0.15f ) );
	g.drawRoundedRectangle ( b, 10.0f, 1.0f );
}
//-----------------------------------------------------------------------------

void GUI_ScrollTextViewer::setText ( const juce::String& text )
{
	textRenderer.setText ( text );
}
//-----------------------------------------------------------------------------

void GUI_ScrollTextViewer::setFont ( const juce::Font& newFont )
{
	textRenderer.setFont ( newFont );
}
//-----------------------------------------------------------------------------

void GUI_ScrollTextViewer::resized ()
{
	viewport.setBounds ( getLocalBounds () );

	textRenderer.setSize ( viewport.getMaximumVisibleWidth (), textRenderer.getHeight () );
}
//-----------------------------------------------------------------------------
