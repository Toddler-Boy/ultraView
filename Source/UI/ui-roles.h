#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

// Shared resolver for the role registries: index of roleName in names, or n
// when absent (the registries' count sentinel)
namespace UI
{
	[[ nodiscard ]] inline int roleIndex ( const char* const* names, const int n, const juce::String& roleName )
	{
		for ( auto i = 0; i < n; ++i )
			if ( roleName == names[ i ] )
				return i;

		return n;
	}
}
//-----------------------------------------------------------------------------
