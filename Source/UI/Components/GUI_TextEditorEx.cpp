#include "GUI_TextEditorEx.h"

//-------------------------------------------------------------------------------------------------

GUI_TextEditorEx::ClickoutSideListener::ClickoutSideListener ( GUI_TextEditorEx& _editor )
	: editor ( _editor )
{
	juce::Desktop::getInstance ().addGlobalMouseListener ( this );
}
//-------------------------------------------------------------------------------------------------

GUI_TextEditorEx::ClickoutSideListener::~ClickoutSideListener ()
{
	juce::Desktop::getInstance ().removeGlobalMouseListener ( this );
}
//-------------------------------------------------------------------------------------------------

void GUI_TextEditorEx::ClickoutSideListener::mouseDown ( const juce::MouseEvent& event )
{
	if ( ! editor.onGetScreenBounds ().contains ( event.getScreenPosition () ) )
		editor.giveAwayKeyboardFocus ();
}
//-------------------------------------------------------------------------------------------------

GUI_TextEditorEx::GUI_TextEditorEx ( const juce::String& componentName, juce::juce_wchar passwordCharacter )
	: juce::TextEditor ( componentName, passwordCharacter )
{
	setSelectAllWhenFocused ( true );
	setWantsKeyboardFocus ( true );
	setJustification ( juce::Justification::centredLeft );
	setBorder ( juce::BorderSize<int> {} );

	onGetScreenBounds = [ this ]	{		return getScreenBounds ();	};
}
//-------------------------------------------------------------------------------------------------

void GUI_TextEditorEx::mouseDoubleClick (const juce::MouseEvent& event)
{
	if ( ! mouseDownInEditor )
		return;

	if ( wantsDoubleClick && ! hasFocus )
	{
		setWantsKeyboardFocus ( true );
		grabKeyboardFocus ();
		setWantsKeyboardFocus ( ! clickOnlyFocus );

		return;
	}

	juce::TextEditor::mouseDoubleClick ( event );
}
//-------------------------------------------------------------------------------------------------

void GUI_TextEditorEx::mouseUp ( const juce::MouseEvent& event )
{
	if ( ! mouseDownInEditor )
		return;

	if ( wantsDoubleClick )
	{
		juce::TextEditor::mouseUp ( event );
		return;
	}

	{
		const auto	relativePos = event.getEventRelativeTo ( this ).getPosition ();

		if (	clickOnlyFocus
			&&	! getWantsKeyboardFocus ()
			&&	this->hitTest ( relativePos.x, relativePos.y )
			)
		{
			setWantsKeyboardFocus ( true );
			grabKeyboardFocus ();
			setWantsKeyboardFocus ( false );
			return;
		}
	}
}
//-------------------------------------------------------------------------------------------------

void GUI_TextEditorEx::focusGained ( juce::Component::FocusChangeType fct )
{
	applyFontToAllText ( getFont () );

	juce::TextEditor::focusGained ( fct );
	hasFocus = true;

	if ( auto search = getParentComponent (); search->getProperties ().getWithDefault ( "type", "" ) == "searchBar" )
		search->focusGained ( fct );
}
//-------------------------------------------------------------------------------------------------

void GUI_TextEditorEx::focusLost ( juce::Component::FocusChangeType fct )
{
	TextEditor::focusLost ( fct );
	hasFocus = false;

	setHighlightedRegion ( juce::Range<int>::emptyRange ( 0 ) );

	applyFontToAllText ( getFont () );

	// Is this editor part of a search bar?
	if ( auto search = getParentComponent (); search->getProperties ().getWithDefault ( "type", "" ) == "searchBar" )
		search->focusLost ( fct );
}
//-------------------------------------------------------------------------------------------------

void GUI_TextEditorEx::disableClickOnlyFocus ()
{
	clickOnlyFocus = false;
	setWantsKeyboardFocus ( true );
}
//-------------------------------------------------------------------------------------------------

void GUI_TextEditorEx::enableSingleClick ()
{
	wantsDoubleClick = false;
}
//-------------------------------------------------------------------------------------------------

void GUI_TextEditorEx::returnPressed ()
{
	TextEditor::returnPressed ();

	juce::Component::unfocusAllComponents ();

	if ( onReturnPressed )
		onReturnPressed ();
}
//-------------------------------------------------------------------------------------------------

void GUI_TextEditorEx::escapePressed ()
{
	TextEditor::escapePressed ();

	juce::Component::unfocusAllComponents ();
}
//-------------------------------------------------------------------------------------------------

juce::Rectangle<int> GUI_TextEditorEx::getCaretRectangleForCharIndex ( int index ) const
{
	// Reduce height of caret, so it doesn't touch the bottom
	auto	b = juce::TextEditor::getCaretRectangleForCharIndex ( index );
	return b;
//	return b.withTrimmedBottom ( b.getHeight () / 15 );
}
//-------------------------------------------------------------------------------------------------

void GUI_TextEditorEx::mouseDown ( const juce::MouseEvent& e )
{
	mouseDownInEditor = e.originalComponent == this;

	juce::TextEditor::mouseDown ( e );
}
//----------------------------------------------------------------------------------

void GUI_TextEditorEx::selectAllText ()
{
	setWantsKeyboardFocus ( true );
	grabKeyboardFocus ();
	setWantsKeyboardFocus ( ! clickOnlyFocus );

	selectAll ();
}
//----------------------------------------------------------------------------------
