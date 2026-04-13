#pragma once

#include <JuceHeader.h>

#include "yamlFile.h"
//-----------------------------------------------------------------------------

class Icons final : public yamlFile
{
public:
	Icons ();

	void load () override;
	const juce::String& get ( const juce::String& name );

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( Icons )
};
//-----------------------------------------------------------------------------
