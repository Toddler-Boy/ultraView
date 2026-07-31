#include "ComponentFactory.h"

#include "UI/Components/GUI_AudioDeviceSelector.h"
#include "UI/Components/GUI_SettingsNumberEdit.h"
#include "UI/Components/GUI_SettingsToggle.h"
#include "ultra-shared/UI/Components/GUI_Label.h"
#include "ultra-shared/UI/SharedComponentFactory.h"

//-----------------------------------------------------------------------------

std::pair<juce::Component*, bool> componentFactory ( const juce::String& typeName )
{
	auto	typeParts = juce::StringArray::fromTokens ( typeName, "(,)", "" );
	typeParts.trim ();
	const auto	compType = typeParts[ 0 ].toLowerCase ();
	typeParts.remove ( 0 );
	typeParts.removeEmptyStrings ();

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
	// Header1
	//
	if ( compType == "header1" )
	{
		jassert ( typeParts.size () == 1 );

		auto	header = new GUI_DynamicLabel ( "settings/header/" + typeParts[ 0 ], 32.0f, 700 );
		header->setJustification ( juce::Justification::topLeft );

		return { header, false };
	}

	//
	// Header2
	//
	if ( compType == "header2" )
		return { new GUI_DynamicLabel ( "settings/header/" + typeParts[ 0 ], 18.0f, 700 ), false };

	//
	// Settings label
	//
	if ( compType == "set-label" )
		return { new GUI_DynamicLabel ( "settings/" + typeParts[ 0 ], 14.0f, 600 ), false };

	//
	// The generic and CRT-settings types
	//
	if ( auto shared = sharedComponentFactory ( compType, typeParts ); shared.first != nullptr )
		return shared;

	// Unknown component type
	Z_ERR ( "Unknown component type: " << compType );
	jassertfalse;
	return { nullptr, false };
}
//-----------------------------------------------------------------------------
