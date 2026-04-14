#include "GUI_SettingsNumberEdit.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_SettingsNumberEdit::GUI_SettingsNumberEdit ( const juce::String& setSection, const juce::String& setName )
	: juce::Component ( setSection + "-" + setName )
	, settingSection ( setSection )
	, settingName ( setName )
	, label ( "settings/" + setSection + "/" + setName, 14.0f * 1.3f, 600, UI::colors::text )
	, help ( "settings/" + setSection + "/" + setName + "-help", 12.0f * 1.3f, 500, UI::colors::textMuted )
{
	help.setName ( "help" );

	// Number field
	number.applyFontToAllText ( UI::font ( 14.0f * 1.3f, 600 ), true );
	number.setJustification ( juce::Justification::centred );
	number.setIndents ( 4, 0 );
	number.setBorder ( {} );

	addAndMakeVisible ( label );
	addAndMakeVisible ( help );
	addAndMakeVisible ( number );

	auto finishedEdit = [ this ]
	{
		number.onFocusLost ();
		giveAwayKeyboardFocus ();
	};

	number.onReturnKey = finishedEdit;
	number.onEscapeKey = finishedEdit;

	number.onFocusLost = [ this ]
	{
		preferences->set ( settingSection, settingName, number.getText ().getIntValue () );
	};
}
//-----------------------------------------------------------------------------

void GUI_SettingsNumberEdit::resized ()
{
	layout.setLayout ( {	paths::getDataRoot ( "UI/layouts/constants.json" ),
							paths::getDataRoot ( "UI/layouts/components/settings-number.json" ) } );

	lookAndFeelChanged ();
}
//-----------------------------------------------------------------------------

void GUI_SettingsNumberEdit::lookAndFeelChanged ()
{
	const auto	txtCol = UI::getShade ( 1.0f );

	number.setColour ( juce::TextEditor::backgroundColourId, UI::getShade ( 0.2f ) );
	number.setColour ( juce::TextEditor::textColourId, txtCol );
	number.applyColourToAllText ( txtCol );
}
//-----------------------------------------------------------------------------

void GUI_SettingsNumberEdit::restorePreference ()
{
	number.setText ( juce::String ( preferences->get<int> ( settingSection, settingName ) ), juce::dontSendNotification );
}
//-----------------------------------------------------------------------------
