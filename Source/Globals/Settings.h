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
			{ "UI",			"page",				"search" },
			{ "UI",			"crt-image",		0 },

			{ "UI",			"search-str",		"" },
			{ "UI",			"search-selected",	0 },
			{ "UI",			"search-pos",		0.0 },
			{ "UI",			"search-col",		4 },
			{ "UI",			"search-forwards",	true },

			{ "UI",			"playlist",				"" },
			{ "UI",			"playlist-selected",	0 },
			{ "UI",			"playlist-pos",			0.0 },

			{ "Paths",		"hvsc",			juce::SystemStats::getEnvironmentVariable ( "HVSC_BASE", juce::File::getSpecialLocation ( juce::File::SpecialLocationType::userDocumentsDirectory ).getChildFile ( "C64Music" ).getFullPathName () ).toStdString () },
			{ "Paths",		"user",			juce::File::getSpecialLocation ( juce::File::SpecialLocationType::userDocumentsDirectory ).getChildFile ( juce::String ( ProjectInfo::projectName ) + " user-data" ).getFullPathName ().toStdString () },
			{ "Paths",		"export",		juce::File::getSpecialLocation ( juce::File::SpecialLocationType::userDocumentsDirectory ).getChildFile ( juce::String ( ProjectInfo::projectName ) + " exports" ).getFullPathName ().toStdString () },
		};
	}
};
//-----------------------------------------------------------------------------
