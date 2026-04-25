#include "GUI_SearchBar.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

class GUI_SearchBar::HistoryRow : public juce::Component
{
public:
	HistoryRow ( GUI_SearchBar& _owner, const juce::String& _text )
		: owner ( _owner )
		, text ( _text )
	{
		setWantsKeyboardFocus ( false );
		setRepaintsOnMouseActivity ( true );

		deleteButton.setWantsKeyboardFocus ( false );
		deleteButton.onClick = [ this ]
		{
			owner.removeSearchTerm ( text );
			owner.updateHistory ();
		};
		addChildComponent ( deleteButton );

		addMouseListener ( &mouseListener, true );
		mouseListener.onMouseEnter = [ this ] ( const juce::MouseEvent& ) { updateVisibility (); };
		mouseListener.onMouseExit  = [ this ] ( const juce::MouseEvent& ) { updateVisibility (); };
	}
	//----------------------------------------------------------------------------------

	void resized () override
	{
		auto	b = getLocalBounds ();

		deleteButton.setBounds ( b.removeFromRight ( b.getHeight () ).reduced ( 0, 7 ) );
	}
	//----------------------------------------------------------------------------------

	void updateVisibility ()
	{
		auto	oldMouseOver = mouseOver;
		mouseOver = isMouseOver ( true );

		if ( oldMouseOver == mouseOver )
			return;

		repaint ();
		deleteButton.setVisible ( mouseOver );
	}
	//----------------------------------------------------------------------------------

	void paint ( juce::Graphics& g ) override
	{
		const auto	h = float ( getHeight () );
		const auto	radius = h / 2.0f;
		const auto	txtCol = findColour ( UI::colors::text );

		// Fill
		if ( mouseOver )
		{
			const auto	rc = getLocalArea ( &owner, owner.getLocalBounds () ).toFloat ();

			g.setColour ( txtCol.withMultipliedAlpha ( 0.1f ) );
			g.fillRoundedRectangle ( rc, radius );
		}

		const auto	gap = radius * 0.33f;

		// Icon
		{
			const auto	rect = juce::Rectangle<float> ( gap, 0.0f, h, h );

			g.setColour ( txtCol.withMultipliedAlpha ( 0.5f ) );
			g.fillPath ( UI::getScaledPath ( "clock-rotate-left-solid-full", rect, 0, 0.3f ) );
		}

		// Text
		g.setColour ( txtCol );
		g.setFont ( owner.textEditor.getFont () );

		const auto	b = getLocalBounds ().toFloat ().reduced ( gap + h - 2.0f, 2.0f );
		g.drawText ( text, b, juce::Justification::centredLeft, false );
	}
	//----------------------------------------------------------------------------------

	void mouseDown ( const juce::MouseEvent& /*e*/ ) override
	{
		owner.textEditor.setText ( text, juce::sendNotificationSync );
		juce::Timer::callAfterDelay ( 10, [ &o = owner ] { o.closeHistory (); } );
	}
	//----------------------------------------------------------------------------------

	GUI_SearchBar&		owner;
	GUI_SVG_Button		deleteButton { "delete", { "xmark-solid-full" } };
	juce::String		text;

	gin::LambdaMouseListener	mouseListener;
	bool						mouseOver = false;
};
//----------------------------------------------------------------------------------

GUI_SearchBar::GUI_SearchBar ()
{
	setName ( "searchBar" );

	// Load search terms history
	loadSearchTermHistory ();

	// Mark this component as a searchBar
	getProperties ().set ( "type", "searchBar" );

	textEditor.disableClickOnlyFocus ();
	textEditor.addListener ( this );
	textEditor.addMouseListener ( &mouseListener, true );
	textEditor.setFont ( UI::font ( 18.0f ) );
	textEditor.applyFontToAllText ( textEditor.getFont () );

	textEditor.setColour ( juce::TextEditor::outlineColourId, juce::Colours::transparentBlack );
	textEditor.setColour ( juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack );

	addAndMakeVisible ( textEditor );

	textEditor.onReturnPressed = [ this ]
	{
		if ( textEditor.getText ().isNotEmpty () )
			addToSearchTermsHistory ( textEditor.getText () );
	};

	mouseListener.onMouseDown = [ this ] ( const juce::MouseEvent& )
	{
 		if ( ! expanded && originalHeight > 0 )
 			updateHistory ();
	};

	clearSearch.margin = 4.0f;
	clearSearch.alpha[ 0 ] = 0.5f;
	clearSearch.useOrgSize = false;
	clearSearch.setWantsKeyboardFocus ( false );

	addChildComponent ( clearSearch );
	clearSearch.onClick = [ this ]	{	textEditor.setText ( "" );	};
	textEditor.onGetScreenBounds = [ this ]	{	return getScreenBounds ();	};

	juce::Desktop::getInstance ().addFocusChangeListener ( this );
}
//-----------------------------------------------------------------------------

GUI_SearchBar::~GUI_SearchBar ()
{
	juce::Desktop::getInstance ().removeFocusChangeListener ( this );
}
//-----------------------------------------------------------------------------

void GUI_SearchBar::resized ()
{
	// Place text-editor and delete button
	{
		const auto	h = getEditorHeight ();

		auto	b = getLocalBounds ().withHeight ( h ).reduced ( 0, 2 );

		// Leave room on the left for the icon
		b.removeFromLeft ( h );

		// Place clear button at the end
		clearSearch.setBounds ( b.removeFromRight ( h ).reduced ( 0, 5 ) );

		// Place text-editor
		textEditor.setBounds ( b.withSizeKeepingCentre ( b.getWidth (), b.getHeight () - 4  ).translated ( 0, -2 ) );
	}

	// Place history rows
	{
		auto	b = getLocalBounds ();
		b.removeFromTop ( originalHeight );

		for ( auto r : rows )
			r->setBounds ( b.removeFromTop ( originalHeight ) );
	}
}
//-----------------------------------------------------------------------------

void GUI_SearchBar::paint ( juce::Graphics& g )
{
	const auto	b = getLocalBounds ().toFloat ();
	const auto	radius = getEditorHeight () / 2.0f;

	// Fill background
	{
		g.setColour ( textEditor.findColour ( juce::TextEditor::backgroundColourId ) );
		g.fillRoundedRectangle ( b, radius );
	}

	// Outline
	if ( textEditor.isEnabled () && textEditor.hasKeyboardFocus ( true ) && ! textEditor.isReadOnly () )
	{
		g.setColour ( findColour ( juce::TextEditor::focusedOutlineColourId ) );
		g.drawRoundedRectangle ( b.reduced ( 1.0f ), radius - 1.0f, 2.0f );
	}

	// Icon
	{
		const auto	col = textEditor.findColour ( juce::TextEditor::textColourId );
		const auto	rect = juce::Rectangle<float> ( radius * 0.33f, 0.0f, getEditorHeight (), getEditorHeight () );

		g.setColour ( col );
		g.fillPath ( UI::getScaledPath ( "magnifying-glass-solid-full", rect, 0, 0.3f ) );
	}
}
//-----------------------------------------------------------------------------

void GUI_SearchBar::mouseDown ( const juce::MouseEvent& /*event*/ )
{
	textEditor.grabKeyboardFocus ();
}
//-----------------------------------------------------------------------------

void GUI_SearchBar::focusGained ( juce::Component::FocusChangeType /*cause*/ )
{
	repaint ();
}
//-----------------------------------------------------------------------------

void GUI_SearchBar::focusLost ( juce::Component::FocusChangeType /*cause*/ )
{
	repaint ();
}
//-----------------------------------------------------------------------------

void GUI_SearchBar::textEditorTextChanged ( juce::TextEditor& e )
{
	clearSearch.setVisible ( e.getText ().isNotEmpty () );
	clearSearch.repaint ();

	if ( showSearchHistory )
		updateHistory ();

	if ( onTextChange )
		onTextChange ();
}
//----------------------------------------------------------------------------------

void GUI_SearchBar::updateClearButton ()
{
	clearSearch.setVisible ( textEditor.getText ().isNotEmpty () );
}
//----------------------------------------------------------------------------------

void GUI_SearchBar::enableSearchHistory ( bool e )
{
	showSearchHistory = e;
}
//----------------------------------------------------------------------------------

int GUI_SearchBar::getEditorHeight ()
{
	if ( expanded )
		return originalHeight;

	return getHeight ();
}
//----------------------------------------------------------------------------------

void GUI_SearchBar::globalFocusChanged ( juce::Component* /*focusedComponent*/ )
{
	if ( ! showSearchHistory )
		return;

	if ( hasKeyboardFocus ( true ) )
	{
		if ( ! expanded )
		{
			if ( originalHeight == 0 )
				originalHeight = getHeight ();

			updateHistory ();
		}
	}
	else if ( expanded )
	{
		expanded = false;
		rows.clear ();
		setSize ( getWidth (), originalHeight );
	}
}
//----------------------------------------------------------------------------------

void GUI_SearchBar::closeHistory ()
{
	expanded = false;
	rows.clear ();
	setSize ( getWidth (), originalHeight );
}
//----------------------------------------------------------------------------------

void GUI_SearchBar::updateHistory ()
{
	auto	history = searchTermsHistory;

	// Filter out entries that don't contain the text
	if ( auto text = textEditor.getText (); text.isNotEmpty () )
		for ( auto i = history.size (); --i >= 0; )
			if ( ! history[ i ].containsIgnoreCase ( text ) )
				history.remove ( i );

	// Build new rows (up to 10)
	rows.clear ();

	for ( auto cnt = 0; const auto& h : history )
	{
		auto	row = new HistoryRow ( *this, h );
		addAndMakeVisible ( row );
		rows.add ( row );
		if ( ++cnt == 10 )
			break;
	}

	expanded = true;
	setSize ( getWidth (), originalHeight * ( rows.size () + 1 ) + ( rows.size () != 0 ? 2 : 0 ) );
	resized ();
}
//----------------------------------------------------------------------------------

void GUI_SearchBar::addToSearchTermsHistory ( const juce::String& s )
{
	if ( s.isEmpty () )
		return;

	searchTermsHistory.removeString ( s, true );
	searchTermsHistory.insert ( 0, s );

	searchTermsHistory.removeRange ( 100, searchTermsHistory.size () - 100 );

	saveSearchTermHistory ();
}
//-------------------------------------------------------------------------------------------------

void GUI_SearchBar::removeSearchTerm ( const juce::String& s )
{
	searchTermsHistory.removeString ( s, true );
	saveSearchTermHistory ();
}
//-------------------------------------------------------------------------------------------------

void GUI_SearchBar::saveSearchTermHistory ()
{
	auto	file = paths::getSearchtermsPath ();
	if ( file == juce::File () )
		return;

	gin::overwriteWithText ( file, searchTermsHistory.joinIntoString ( "\n" ) );
}
//----------------------------------------------------------------------------------

void GUI_SearchBar::loadSearchTermHistory ()
{
	searchTermsHistory.clear ();

	auto	file = paths::getSearchtermsPath ();
	if ( file == juce::File () )
		return;

	file.readLines ( searchTermsHistory );

	searchTermsHistory.removeEmptyStrings ();
}
//-------------------------------------------------------------------------------------------------
