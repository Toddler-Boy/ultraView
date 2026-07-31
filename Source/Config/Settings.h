#pragma once

#include <JuceHeader.h>

#include "ultra-shared/Config/YamlFile.h"

//-----------------------------------------------------------------------------

class Settings final : public YamlFile
{
public:
	Settings () : YamlFile ( getDefaultValues () )
	{
		auto	file = juce::File::getSpecialLocation ( juce::File::SpecialLocationType::userApplicationDataDirectory )
						.getChildFile ( ProjectInfo::projectName )
						.getChildFile ( "settings.yml" );

		load ( file );
	}

private:
	[[ nodiscard ]] static std::vector<YamlFile::value> getDefaultValues ()
	{
		return {
			{ "output",		"device",			"System default" },

			{ "ui",			"window-position",	"" },

			{ "network",	"last-ip",			"" },
			{ "network",	"password",			"" },

			{ "paths",		"user",			juce::File::getSpecialLocation ( juce::File::SpecialLocationType::userDocumentsDirectory ).getChildFile ( juce::String ( ProjectInfo::projectName ) + " user-data" ).getFullPathName ().toStdString () },
			{ "paths",		"apps",			"" },
		};
	}
};
//-----------------------------------------------------------------------------
