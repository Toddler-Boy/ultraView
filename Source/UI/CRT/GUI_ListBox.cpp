#include <JuceHeader.h>

#include "GUI_ListBox.h"

#include "UI/SID_LookAndFeel.h"
#include "UI/Misc/GUI_RoundedClip.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_ListBox::GUI_ListBox ()
	: hover ( *this )
{
	setName ( "listbox" );

	hover.addChangeListener ( this );

	setModel ( this );

	setOutlineThickness ( 0 );
	setRowHeight ( 28 );
	setMultipleSelectionEnabled ( false );

	setHeaderHeight ( 28 );
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
				const auto	cmp = helpers::strnatcmp ( a->normalized.c_str (), b->normalized.c_str () );
				return isForwards ? cmp < 0 : cmp > 0;
			} );
			break;
	}
}
//-----------------------------------------------------------------------------

juce::var GUI_ListBox::getDragSourceDescription ( const juce::SparseSet<int>& selectedRows )
{
	return juce::String ( rowData[ selectedRows[ 0 ] ]->path );
}
//-----------------------------------------------------------------------------

bool GUI_ListBox::keyPressed ( const juce::KeyPress& key )
{
	if ( key.getModifiers ().isCommandDown () )
		return false;

	return juce::TableListBox::keyPressed ( key );
}
//-----------------------------------------------------------------------------

void GUI_ListBox::paint ( juce::Graphics& g )
{
	juce::TableListBox::paint ( g );

/*	const auto	bounds = getLocalBounds ().toFloat ();
	g.setColour ( UI::getShade ( 0.02f ) );
	g.fillRoundedRectangle ( bounds, 8.0f );*/
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

	const auto& entry = *rowData[ rowNumber ];

	const auto	font = UI::font ( 16.0f, 600 );
	const auto	fontsmall = UI::font ( 10.0f, 600 );

	const auto	rowCol = rowIsSelected ? txtCol : col;

	switch ( columnId )
	{
		case UI::columnId::name:
			{
				if ( entry.official )
				{
					static const auto	tw = juce::GlyphArrangement::getStringWidth ( fontsmall, "OFFICIAL" );

					g.setColour ( txtCol.withMultipliedBrightness ( 0.4f ) );
					const auto	badgeRect = b.removeFromLeft ( tw + 8.0 ).reduced ( 0.0f, 2.0f );
					g.fillRoundedRectangle ( badgeRect, 4.0f );

					g.setFont ( fontsmall );
					g.setColour ( txtCol.withMultipliedBrightness ( 0.1f ) );
					g.drawText ( "OFFICIAL", badgeRect, juce::Justification::centred, false );

					b.removeFromLeft ( 4.0f );
				}

				g.setFont ( font );
				g.setColour ( rowCol );
				g.drawText ( entry.name, b, juce::Justification::centredLeft, true );
		}
			break;
	}
}
//-----------------------------------------------------------------------------

juce::String GUI_ListBox::getCellTooltip ( int rowNumber, int columnId )
{
	if ( columnId != UI::columnId::name )
		return {};

	if ( ! juce::isPositiveAndBelow ( rowNumber, getNumRows () ) )
		return {};

	return rowData[ rowNumber ]->path;
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
		{ UI::columnId::name,			{ "Title",		280 } },
	};

	const auto&	[ name, width ] = defaultCols.at ( colId );

	auto&	header = getHeader ();
	header.addColumn ( name, colId, width, width, colId == UI::columnId::name ? -1 : width, juce::TableHeaderComponent::visible | ( sortable ? juce::TableHeaderComponent::sortable : 0 ) );
}
//-----------------------------------------------------------------------------

void GUI_ListBox::setRowData ( std::vector<browserEntry*>& newData )
{
	rowData = newData;

	updateContent ();
	getHeader ().reSortTable ();
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
