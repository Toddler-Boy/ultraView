#include "GUI_CRTSliderIcon.h"

//-----------------------------------------------------------------------------

GUI_CRTSliderIcon::GUI_CRTSliderIcon ( const juce::String& setSection, const juce::String& setName, const bool bidirectional )
	: settingSection ( setSection )
	, settingName ( setName )
	, icon ( "icon", "crt/settings/" + setSection + "/" + setName )
{
	icon.setInterceptsMouseClicks ( false, false );

	slider.setName ( "slider" );

	slider.setScrollWheelEnabled ( false );
	slider.setRange ( bidirectional ? -100.0 : 0.0, 100.0, 1.0 );
	slider.setTextBoxStyle ( juce::Slider::TextBoxRight, true, 30, 20 );

	slider.setDoubleClickReturnValue ( true, double ( preferences->getDefault<int> ( settingSection, settingName ) ) );
	slider.setValue ( double ( preferences->get<int> ( settingSection, settingName ) ), juce::dontSendNotification );

	slider.onValueChange = [ this ]
	{
		preferences->set ( settingSection, settingName, slider.getValue () );

		if ( onValueChange )
			onValueChange ();
	};

	addAndMakeVisible ( icon );
	addAndMakeVisible ( slider );
}
//-----------------------------------------------------------------------------

void GUI_CRTSliderIcon::resized ()
{
	layout.setLayout ( {	paths::getDataRoot ( "UI/layouts/constants.json" ),
							paths::getDataRoot ( "UI/layouts/components/crt-slider-icon.json" ) } );
}
//-----------------------------------------------------------------------------
