#pragma once

#include <JuceHeader.h>

#include "Helpers/MessageRouter.h"

//-----------------------------------------------------------------------------

// Typed messages for the global message bus (the ultraSID scheme; ultraView's
// legacy string actions keep flowing through UI::sendGlobalMessage and are
// parsed in actionListenerCallback).
//
// Each message is a struct: named fields, a wire verb, encode () to the
// string form and decode () back from the parsed parameter list.

namespace msg
{

// The transport, all the send () members below funnel through here.
// Null broadcaster = teardown, the message is dropped
template<typename E>
void send ( const E& e )
{
	if ( auto* broadcaster = UI::ab.load () )
		broadcaster->sendActionMessage ( e.encode () );
}
//-----------------------------------------------------------------------------

// Screenshot/artwork curation (developer mode, sent by the shared VIC2
// palette's right-click menu; ultraView has no handlers and just logs them)

#define SIMPLE_MESSAGE(Name, wire)								\
	struct Name													\
	{															\
		static constexpr auto	verb = wire;					\
		[[ nodiscard ]] juce::String encode () const	{	return verb;	}	\
		void send () const				{	msg::send ( *this );	}	\
	}

SIMPLE_MESSAGE ( ToggleFirstLuma,	"toggleFirstLuma" );
SIMPLE_MESSAGE ( ToggleFirstLumaAll,"toggleFirstLumaAll" );
SIMPLE_MESSAGE ( ToggleThumbnail,	"toggleThumbnail" );
SIMPLE_MESSAGE ( DeleteImage,		"deleteImage" );
SIMPLE_MESSAGE ( RemoveBorderColor,	"removeBorderColor" );

#undef SIMPLE_MESSAGE

struct AssignBorderColor
{
	int	index = 0;

	static constexpr auto	verb = "assignBorderColor";
	[[ nodiscard ]] juce::String encode () const								{	return juce::String ( verb ) + " " + juce::String ( index );	}
	[[ nodiscard ]] static AssignBorderColor decode ( const juce::StringArray& p )	{	return { p[ 0 ].getIntValue () };	}

	void send () const	{	msg::send ( *this );	}
};
//-----------------------------------------------------------------------------

struct SettingChanged
{
	juce::String	section;
	juce::String	key;	// optional, a bare section broadcasts it whole

	static constexpr auto	verb = "settingChanged";

	[[ nodiscard ]] juce::String encode () const
	{
		return juce::String ( verb ) + " " + section + ( key.isEmpty () ? juce::String () : " " + key );
	}

	[[ nodiscard ]] static SettingChanged decode ( const juce::StringArray& p )	{	return { p[ 0 ], p[ 1 ] };	}

	[[ nodiscard ]] juce::String sectionKey () const	{	return section + "/" + key;	}

	void send () const	{	msg::send ( *this );	}
};

}	// namespace msg
//-----------------------------------------------------------------------------
