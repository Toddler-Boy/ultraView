#pragma once

#include <JuceHeader.h>
#include <optional>

//-------------------------------------------------------------------------------------------------

class GUI_TextEditorEx final : public juce::TextEditor
{
public:
	GUI_TextEditorEx (const juce::String& componentName = juce::String (), juce::juce_wchar passwordCharacter = 0);

	// juce::Component
	void mouseDoubleClick (const juce::MouseEvent& event) override;
	void mouseUp (const juce::MouseEvent& event) override;

	void mouseDown ( const juce::MouseEvent& e ) override;

	void focusGained ( juce::Component::FocusChangeType cause) override;
	void focusLost ( juce::Component::FocusChangeType cause) override;

	// juce::TextEditor
	void returnPressed () override;
	void escapePressed () override;
	juce::Rectangle<int> getCaretRectangleForCharIndex ( int index ) const override;

	// This
	void disableClickOnlyFocus ();
	void enableSingleClick ();
	void selectAllText ();

	std::function<juce::Rectangle<int> ()> onGetScreenBounds;
	std::function<void ()>	onReturnPressed;

private:
	class ClickoutSideListener : private juce::MouseListener
	{
	public:
		ClickoutSideListener ( GUI_TextEditorEx& e );
		~ClickoutSideListener () override;

	private:
		void mouseDown ( const juce::MouseEvent& e ) override;

		GUI_TextEditorEx& editor;
	};

	ClickoutSideListener clickoutSideListener { *this };

	bool			wantsDoubleClick = true;
	bool			clickOnlyFocus = true;
	bool			hasFocus = false;
	bool			mouseDownInEditor = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GUI_TextEditorEx)
};
//-------------------------------------------------------------------------------------------------
