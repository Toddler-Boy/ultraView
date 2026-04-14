#include "ComponentFactory.h"

#include "Globals/Preferences.h"

#include "UI/Components/GUI_AudioDeviceSelector.h"
#include "UI/Components/GUI_ComboBox.h"
#include "UI/Components/GUI_CRTSliderIcon.h"
#include "UI/Components/GUI_CRTSliderLabel.h"
#include "UI/Components/GUI_Disabler.h"
#include "UI/Components/GUI_Label.h"
#include "UI/Components/GUI_Line.h"
#include "UI/Components/GUI_SettingsBox.h"
#include "UI/Components/GUI_SettingsNumberEdit.h"
#include "UI/Components/GUI_SettingsToggle.h"
#include "UI/Components/GUI_Slider.h"
#include "UI/Components/GUI_SVG_Button.h"
#include "UI/Components/GUI_Toggle.h"
#include "UI/Components/GUI_VIC2_Palette.h"
#include "UI/Components/GUI_XYPad.h"

//-----------------------------------------------------------------------------

std::pair<juce::Component*, bool> componentFactory ( const juce::String& typeName )
{
	const juce::SharedResourcePointer<Preferences>	preferences;

	auto	typeParts = juce::StringArray::fromTokens ( typeName, "(,)", "" );
	typeParts.trim ();
	const auto	compType = typeParts[ 0 ].toLowerCase ();
	typeParts.remove ( 0 );
	typeParts.removeEmptyStrings ();

	//
	// Box (dark grey with thin outline)
	//
	if ( compType == "bento" )
		return { new GUI_SettingsBox, false };

	//
	// Disabler
	//
	if ( compType == "disabler" )
		return { new GUI_Disabler, false };

	//
	// Toggle
	//
	if ( compType == "toggle" )
	{
		jassert ( typeParts.size () == 2 );

		auto	toggle = new GUI_Toggle;

		toggle->setToggleState ( preferences->get<bool> ( typeParts[ 0 ], typeParts[ 1 ] ), juce::dontSendNotification );

		return { toggle, false };
	}

	//
	// Settings toggle (with description text)
	//
	if ( compType == "set-toggle" )
		return { new GUI_SettingsToggle ( typeParts[ 0 ], typeParts[ 1 ] ), false };

	//
	// Settings number edit (with description text)
	//
	if ( compType == "set-number" )
		return { new GUI_SettingsNumberEdit ( typeParts[ 0 ], typeParts[ 1 ] ), false };

	//
	// Audio device selector
	//
	if ( compType == "audio-device" )
		return { new GUI_AudioDeviceSelector, false };

	//
	// Button
	//
	if ( compType == "button" )
	{
		jassert ( typeParts.size () == 1 );

		auto	but = new juce::TextButton;

		but->setButtonText ( typeParts[ 0 ] );

		return { but, false };
	}

	//
	// Selector (drop-down)
	//
	if ( compType == "selector" )
	{
		auto	combo = new GUI_ComboBox;
		combo->getProperties ().set ( "drawArrow", false );
		combo->setScrollWheelEnabled ( false );
		return { combo, false };
	}

	//
	// Slider
	//
	if ( compType == "slider" )
	{
		jassert ( typeParts.size () >= 2 );

		auto	slider = new GUI_Slider ( juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight );
		{
			const auto	bidirectional = typeParts[ 2 ].toLowerCase () == "true";

			slider->setScrollWheelEnabled ( false );
			slider->setRange ( bidirectional ? -100.0 : 0.0, 100.0, 1.0 );
			slider->setTextBoxStyle ( juce::Slider::TextBoxRight, true, 30, 20 );
			slider->setDoubleClickReturnValue ( true, double ( preferences->getDefault<int> ( typeParts[ 0 ], typeParts[ 1 ] ) ) );
			slider->setValue ( double ( preferences->get<int> ( typeParts[ 0 ], typeParts[ 1 ] ) ), juce::dontSendNotification );
		}

		return { slider, false };
	}

	//
	// Slider with text-choices
	//
	if ( compType == "choices" )
	{
		jassert ( typeParts.size () >= 4 );

		auto	section = typeParts[ 0 ];
		auto	id = typeParts[ 1 ];
		typeParts.removeRange ( 0, 2 );

		auto	slider = new GUI_Slider ( juce::Slider::LinearHorizontal, juce::Slider::NoTextBox );
		slider->setRange ( 0.0, typeParts.size () - 1.0, 1.0 );
		slider->setScrollWheelEnabled ( false );

		slider->getProperties ().set ( "choices", typeParts.size () );

		auto	chosenStr = preferences->get<juce::String> ( section, id );
		auto	chosenIndex = 0;

		for ( auto i = 0; const auto& choice : typeParts )
		{
			slider->getProperties ().set ( "choice" + juce::String ( i ), choice );

			if ( chosenStr.equalsIgnoreCase ( choice ) )
				chosenIndex = i;

			++i;
		}

		slider->setValue ( double ( chosenIndex ), juce::dontSendNotification );

		return { slider, false };
	}

	//
	// Header1
	//
	if ( compType == "header1" )
	{
		jassert ( typeParts.size () == 1 );

		auto	header = new GUI_DynamicLabel ( "settings/header/" + typeParts[ 0 ], 32.0f * 1.3f, 700 );
		header->setJustification ( juce::Justification::topLeft );

		return { header, false };
	}

	//
	// Header2
	//
	if ( compType == "header2" )
		return { new GUI_DynamicLabel ( "settings/header/" + typeParts[ 0 ], 18.0f * 1.3f, 700 ), false };

	//
	// crt-header
	//
	if ( compType == "crt-header" )
		return { new GUI_DynamicLabel ( "crt/settings/" + typeParts[ 0 ] + "/header", 15.0f * 1.3f, 600 ), false };

	//
	// crt-label
	//
	if ( compType == "crt-label" )
		return { new GUI_DynamicLabel ( typeParts[ 0 ], 11.0f * 1.3f, 500, UI::colors::text ), false };

	//
	// crt-slider-label
	//
	if ( compType == "crt-slider-label" )
		return { new GUI_CRTSliderLabel ( typeParts[ 0 ], typeParts[ 1 ], typeParts[ 2 ].equalsIgnoreCase ( "true" ) ), false };

	//
	// crt-slider-icon
	//
	if ( compType == "crt-slider-icon" )
		return { new GUI_CRTSliderIcon ( typeParts[ 0 ], typeParts[ 1 ], typeParts[ 2 ].equalsIgnoreCase ( "true" ) ), false };

	//
	// Settings label
	//
	if ( compType == "set-label" )
		return { new GUI_DynamicLabel ( "settings/" + typeParts[ 0 ], 14.0f * 1.3f, 600 ), false };

	//
	// Label
	//
	if ( compType == "label" )
	{
		const auto	fontSize = typeParts.size () >= 2 ? typeParts[ 1 ].getFloatValue () : 14.0f;
		const auto	fontWeight = typeParts.size () >= 3 ? typeParts[ 2 ].getIntValue () : 500;

		return { new GUI_Label ( typeParts[ 0 ], fontSize, fontWeight ), false };
	}

	//
	// VIC-II palette display
	//
	if ( compType == "vic2palette" )
		return { new GUI_VIC2_Palette, false };

	//
	// Line
	//
	if ( compType == "line" )
		return { new GUI_Line, false };

	//
	// XY-Pad (for color bleed)
	//
	if ( compType == "xypad" )
		return { new GUI_XYPad ( typeParts[ 0 ], typeParts[ 1 ], juce::Colour::fromString ( typeParts[ 2 ] ).withAlpha ( 1.0f ) ), false };

	// Unknown component type
	jassertfalse;
	return { nullptr, false };
}
//-----------------------------------------------------------------------------
