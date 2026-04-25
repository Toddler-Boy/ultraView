#pragma once

#include <JuceHeader.h>

#include "GUI_TextEditorEx.h"
#include "GUI_SVG_Button.h"

//----------------------------------------------------------------------------------

class GUI_SearchBar final : public juce::Component, public juce::TextEditor::Listener, private juce::FocusChangeListener
{
public:
	GUI_SearchBar ();
	~GUI_SearchBar () override;

	// juce::Component
	void resized () override;
	void paint ( juce::Graphics& g ) override;
	void mouseDown ( const juce::MouseEvent& event ) override;
	void focusGained ( juce::Component::FocusChangeType cause ) override;
	void focusLost ( juce::Component::FocusChangeType cause ) override;

	// juce::FocusChangeListener
	void globalFocusChanged ( juce::Component* ) override;

	// This
	void textEditorTextChanged ( juce::TextEditor& ) override;
	void updateClearButton ();

	void enableSearchHistory ( bool e );
	void addToSearchTermsHistory ( const juce::String& s );
	void removeSearchTerm ( const juce::String& s );
	GUI_TextEditorEx& getTextEditor ()	{	return textEditor;	}
	std::function<void ()>	onTextChange;

private:
	GUI_TextEditorEx	textEditor { "editor" };

	juce::StringArray	searchTermsHistory;

	class HistoryRow;

	int getEditorHeight ();
	void updateHistory ();
	void closeHistory ();
	void loadSearchTermHistory ();
	void saveSearchTermHistory ();

	gin::LambdaMouseListener mouseListener;
	juce::OwnedArray<HistoryRow> rows;

	GUI_SVG_Button	clearSearch { "delete", { "delete" } };

	bool	showSearchHistory = true;
	bool	expanded = false;
	int		originalHeight = 0;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_SearchBar )
};
//----------------------------------------------------------------------------------
