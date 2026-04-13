#pragma once

#include <JuceHeader.h>

#include "yamlFile.h"

//-----------------------------------------------------------------------------

class Strings final : public yamlFile
{
public:
	Strings ();

	void setLanguage ( const juce::String& language );

	void load () override;
	const juce::String& get ( const juce::String& name );

private:
	juce::File		root;
	juce::String	language = "en";

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( Strings )
};
//-----------------------------------------------------------------------------
