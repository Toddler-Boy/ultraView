#include <JuceHeader.h>

#include "MessageRouter.h"

//-----------------------------------------------------------------------------

namespace UI
{
	std::atomic<juce::ActionBroadcaster*>	ab = nullptr;
}
//-----------------------------------------------------------------------------

void UI::setActionBroadCaster ( juce::ActionBroadcaster* _ab ) noexcept
{
	ab = _ab;
}
//-----------------------------------------------------------------------------

juce::String msg::quoted ( const juce::String& text )
{
	// '%' first, so no false "%22" patterns can exist
	return text.replace ( "%", "%25" ).replace ( "\"", "%22" ).quoted ();
}
//-----------------------------------------------------------------------------

std::pair<juce::String, juce::StringArray> msg::parseActionMessage ( const juce::String& message )
{
	auto	parts = juce::StringArray::fromTokens ( message, " ", "\"" );

	// Reverse of msg::quoted, "%22" back first
	for ( auto& part : parts )
		part = part.unquoted ().replace ( "%22", "\"" ).replace ( "%25", "%" );

	auto	cmd = parts[ 0 ];
	parts.remove ( 0 );
	return { cmd, parts };
}
//-----------------------------------------------------------------------------

bool msg::Router::dispatch ( const juce::String& message ) const
{
	if ( message.isEmpty () )
		return false;

	const auto	[ cmd, params ] = parseActionMessage ( message );

	// Sub-routed verbs first ("playlist new", "download HVSC"), then plain verbs
	if ( ! params.isEmpty () )
	{
		if ( const auto it = routes.find ( cmd + " " + params[ 0 ] ); it != routes.end () )
		{
			it->second ( params );
			return true;
		}
	}

	if ( const auto it = routes.find ( cmd ); it != routes.end () )
	{
		it->second ( params );
		return true;
	}

	return false;
}
//-----------------------------------------------------------------------------
