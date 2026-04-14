#pragma once

#include <JuceHeader.h>

#include "iniFile.h"

//-----------------------------------------------------------------------------

class Settings final : public iniFile
{
public:
	Settings () : iniFile ( getDefaultValues () )
	{
		auto	file = juce::File::getSpecialLocation ( juce::File::SpecialLocationType::userApplicationDataDirectory )
						.getChildFile ( ProjectInfo::projectName )
						.getChildFile ( "settings.ini" );

		load ( file );
	}

private:
	static std::vector<iniFile::value> getDefaultValues ()
	{
		return {
			{ "Output",		"device",			"System default" },

			{ "UI",			"window-position",	"" },

			{ "Paths",		"user",			juce::File::getSpecialLocation ( juce::File::SpecialLocationType::userDocumentsDirectory ).getChildFile ( juce::String ( ProjectInfo::projectName ) + " user-data" ).getFullPathName ().toStdString () },
		};
	}
};
//-----------------------------------------------------------------------------
