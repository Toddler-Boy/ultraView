#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class GUI_LargeTextView : public juce::Component
{
public:
	GUI_LargeTextView () = default;

	void setText ( const juce::String& text );
	void setFont ( const juce::Font& newFont );

	void resized () override;
	void paint ( juce::Graphics& g ) override;

private:
	[[ nodiscard ]] static bool isLineALink ( const juce::String& text );
	void updateLayout ();
	void updateButtonPositions ();

	juce::StringArray lines;
	juce::OwnedArray<juce::HyperlinkButton> linkButtons;

	juce::Font	textFont { juce::FontOptions ( 16.0f ) };
	int		rowHeight = 0;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_LargeTextView )
};
//-----------------------------------------------------------------------------

class GUI_ScrollTextViewer : public juce::Component
{
public:
	GUI_ScrollTextViewer ();

	void paint ( juce::Graphics& g ) override;
	void setText ( const juce::String& text );
	void setFont ( const juce::Font& newFont );

	void resized () override;

private:
	juce::Viewport	viewport;
		GUI_LargeTextView	textRenderer;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_ScrollTextViewer )
};
//-----------------------------------------------------------------------------
