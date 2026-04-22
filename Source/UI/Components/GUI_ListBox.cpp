#include <JuceHeader.h>

#include "GUI_ListBox.h"

#include "UI/SID_LookAndFeel.h"
#include "UI/Misc/GUI_RoundedClip.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_ListBox::GUI_ListBox ()
	: hover ( *this )
{
	hover.addChangeListener ( this );

	setModel ( this );

	setOutlineThickness ( 0 );
	setRowHeight ( 28 );
	setMultipleSelectionEnabled ( false );

	setHeaderHeight ( 20 );
	getHeader ().setPopupMenuActive ( false );

	getViewport ()->setScrollBarsShown ( true, false );
}
//-----------------------------------------------------------------------------

GUI_ListBox::~GUI_ListBox ()
{
	hover.removeChangeListener ( this );
}
//-----------------------------------------------------------------------------

void GUI_ListBox::sortOrderChanged ( int newSortColumnId, bool isForwards )
{
	switch ( newSortColumnId )
	{
		case UI::columnId::name:
			// Sort only by name
			std::ranges::sort ( rowData, [ isForwards ] ( const auto& a, const auto& b ) {
				const auto	cmp = helpers::strnatcmp ( a->lowerName.c_str (), b->lowerName.c_str () );
				return isForwards ? cmp < 0 : cmp > 0;
			} );
			break;
	}
}
//-----------------------------------------------------------------------------

bool GUI_ListBox::keyPressed ( const juce::KeyPress& key )
{
	if ( key.getModifiers ().isCommandDown () )
		return false;

	return juce::TableListBox::keyPressed ( key );
}
//-----------------------------------------------------------------------------

void GUI_ListBox::resized ()
{
	juce::TableListBox::resized ();
}
//-----------------------------------------------------------------------------

int GUI_ListBox::getNumRows ()
{
	return int ( rowData.size () );
}
//-----------------------------------------------------------------------------

void GUI_ListBox::paintRowBackground ( juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected )
{
	if ( ! juce::isPositiveAndBelow ( rowNumber, getNumRows () ) )
		return;

	if ( rowIsSelected )
		g.setColour ( UI::getShade ( UI::shades::selected ) );
	else if ( rowNumber == hoverPosition )
		g.setColour ( UI::getShade ( UI::shades::hover ) );
	else
		return;

	const auto		b = juce::Rectangle<int> ( width, height ).toFloat ();
	constexpr auto	r = 3.0f;

	g.fillRoundedRectangle ( b, r );
}
//-----------------------------------------------------------------------------

void GUI_ListBox::paintCell ( juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected )
{
	if ( ! juce::isPositiveAndBelow ( rowNumber, getNumRows () ) )
		return;

	auto	b = juce::Rectangle<int> { width, height }.toFloat ().reduced ( 4.0f, 3.0f );

	const auto	col = findColour ( UI::colors::textMuted );
	const auto	txtCol = findColour ( UI::colors::text );

	g.setColour ( rowIsSelected ? txtCol : col );
	g.setFont ( UI::font ( 16.0f, 600 ) );

	switch ( columnId )
	{
		case UI::columnId::name:
			{
				g.drawText ( rowData[ rowNumber ]->name, b, juce::Justification::centredLeft, true );
			}
			break;
	}
}
//-----------------------------------------------------------------------------

juce::String GUI_ListBox::getCellTooltip ( int rowNumber, int columnId )
{
	if ( ! juce::isPositiveAndBelow ( rowNumber, getNumRows () ) )
		return {};

	return {};
}
//-----------------------------------------------------------------------------

void GUI_ListBox::returnKeyPressed ( int rowNumber )
{
	UI::sendGlobalMessage ( "c64run {}", rowData[ rowNumber ]->path.quoted () );
}
//-----------------------------------------------------------------------------

void GUI_ListBox::cellDoubleClicked ( int rowNumber, int columnId, const juce::MouseEvent& e )
{
	juce::TableListBoxModel::cellDoubleClicked ( rowNumber, columnId, e );
	GUI_ListBox::returnKeyPressed ( rowNumber );
}
//-----------------------------------------------------------------------------

void GUI_ListBox::addHeaderColumn ( int colId, bool sortable )
{
	struct columnProps
	{
		juce::String	name;
		int				width;
	};

	static const std::unordered_map<int, columnProps>	defaultCols =
	{
		{ UI::columnId::name,			{ "Title",		250 } },
	};

	const auto&	[ name, width ] = defaultCols.at ( colId );

	auto&	header = getHeader ();
	header.addColumn ( name, colId, width, width, colId == UI::columnId::name ? -1 : width, juce::TableHeaderComponent::visible | ( sortable ? juce::TableHeaderComponent::sortable : 0 ) );
}
//-----------------------------------------------------------------------------

void GUI_ListBox::setRowData ( std::vector<browserEntry>& newData )
{
	rowData.clear ();
	rowData.reserve ( newData.size () );

	for ( auto& entry : newData )
		rowData.emplace_back ( &entry );

	updateContent ();
	repaint ();
}
//-----------------------------------------------------------------------------

void GUI_ListBox::changeListenerCallback ( juce::ChangeBroadcaster* source )
{
	if ( source == &hover )
	{
		const auto	oldHover = hoverPosition;
		hoverPosition = hover.getHoverPos ();
		if ( oldHover >= 0 )
			repaintRow ( oldHover );

		repaintRow ( hoverPosition );
	}
}
//-----------------------------------------------------------------------------
