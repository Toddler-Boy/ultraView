#pragma once

#include <JuceHeader.h>

#include "iniFile.h"

//-----------------------------------------------------------------------------

class Preferences final : public iniFile
{
public:
	Preferences () : iniFile ( getDefaultValues () )
	{
	}

	void setRoot ( const juce::File& _root )
	{
		Z_DLOG ( "loading preferences" );
		load ( _root.getChildFile ( "preferences.ini" ) );
	}

private:
	static std::vector<iniFile::value> getDefaultValues ()
	{
		return {
			{ "UI",			"allow-screensaver",	false },
			{ "UI",			"theme",				"default" },

			{ "Overlay",	"enabled",				true },
			{ "Overlay",	"bitmap",				"C1702 Bedroom" },
			{ "Overlay",	"daytime",				35 },
			{ "Overlay",	"bezel",				66 },
			{ "Overlay",	"shadow",				75 },
			{ "Overlay",	"zoom",					50 },
			{ "Overlay",	"dust",					50 },
			{ "Overlay",	"chromatic-aberration",	50 },
			{ "Overlay",	"grain",				50 },

			{ "TV",			"system",				"AUTO" },
			{ "TV",			"first-luma",			"AUTO" },
			{ "TV",			"brightness",			55 },
			{ "TV",			"contrast",				85 },
			{ "TV",			"saturation",			55 },
			{ "TV",			"overscan",				25 },

			{ "CRT",		"emulation",			true },
			{ "CRT",		"jailbars",				30 },
			{ "CRT",		"sharpening",			30 },
			{ "CRT",		"luma-blur",			50 },
			{ "CRT",		"chroma-blur",			50 },
			{ "CRT",		"interference",			20 },
			{ "CRT",		"crosstalk",			20 },
			{ "CRT",		"sub-carrier",			10 },
			{ "CRT",		"noise",				15 },
			{ "CRT",		"curve",				25 },
			{ "CRT",		"bleed",				20 },
			{ "CRT",		"bleed-red",			iniFile::vec2i { -100, 0 } },
			{ "CRT",		"bleed-green",			iniFile::vec2i { 75, -75 } },
			{ "CRT",		"bleed-blue",			iniFile::vec2i { 75, 75 } },
			{ "CRT",		"h-wave",				50 },
			{ "CRT",		"scanlines",			50 },
			{ "CRT",		"mask",					50 },
			{ "CRT",		"mask-bitmap",			"Shadow Mask EDP" },
			{ "CRT",		"glow",					66 },
			{ "CRT",		"ambient",				60 },
			{ "CRT",		"phosphor-decay",		40 },
			{ "CRT",		"vignette",				15 },
			{ "CRT",		"reflection",			50 },

			{ "Webcam",		"enabled",				true },
			{ "Webcam",		"brightness",			50 },
			{ "Webcam",		"contrast",				50 },
			{ "Webcam",		"saturation",			50 },
		};
	}
};
//-----------------------------------------------------------------------------
