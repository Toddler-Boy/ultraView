#include "GUI_SettingsToggle.h"

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_SettingsToggle::GUI_SettingsToggle ( const juce::String& setSection, const juce::String& setName )
	: juce::Component ( setSection + "-" + setName )
	, settingSection ( setSection )
	, settingName ( setName )
	, label ( "settings/" + setSection + "/" + setName, 14.0f * 1.3f, 600, UI::colors::text)
	, help ( "settings/" + setSection + "/" + setName + "-help", 12.0f * 1.3f, 500, UI::colors::textMuted )
	, toggle ( "toggle" )
{
	help.setName ( "help" );

	addAndMakeVisible ( label );
	addAndMakeVisible ( help );

	toggle.onClick = [ this ]
	{
		preferences->set ( settingSection, settingName, toggle.getToggleState () );

		UI::sendGlobalMessage ( "settingChanged {} {}", settingSection, settingName );
	};

	addAndMakeVisible ( toggle );
}
//-----------------------------------------------------------------------------

void GUI_SettingsToggle::resized ()
{
	layout.setLayout ( {	paths::getDataRoot ( "UI/layouts/constants.json" ),
							paths::getDataRoot ( "UI/layouts/components/settings-toggle.json" ) } );
}
//-----------------------------------------------------------------------------

void GUI_SettingsToggle::restorePreference ()
{
	toggle.setToggleState ( preferences->get<bool> ( settingSection, settingName ), juce::dontSendNotification );
}
//-----------------------------------------------------------------------------
