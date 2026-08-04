#pragma once

#include <fmt/format.h>
#include <iostream>
#include <string>
#include <utility>

#include "ultra-shared/Helpers/MessageRouter.h"
#include "ultra-shared/Helpers/ComponentUtils.h"
#include "ultra-shared/UI/UI_Helpers.h"
#include "UI/ui-colors.h"

template <>
struct fmt::formatter<juce::String> : fmt::formatter<std::string_view> {
	auto format ( const juce::String& s, fmt::format_context& ctx ) const {
		return fmt::formatter<std::string_view>::format ( std::string ( s.toStdString () ), ctx );
	}
};

namespace UI
{
	enum columnId
	{
		name = 1,
	};

	// The legacy string actions ("c64action ...", "showAbout"); typed messages
	// live in Helpers/Messages.h and share the same broadcaster
	template<typename... Args>
	void sendGlobalMessage ( std::string_view fmt_str, Args&&... args )
	{
		auto	format_args = fmt::make_format_args ( args... );
		auto	formatted = fmt::vformat ( fmt_str, format_args );

		if ( auto* broadcaster = ab.load () )
			broadcaster->sendActionMessage ( formatted );
	}


	// Height-based fonts for the app's own UI; shared code uses the point-based
	// UI::font ( role ) / UI::fontSized ()
	[[ nodiscard ]] juce::Font font ( const float height, const int weight = 400 );
	[[ nodiscard ]] juce::Font monoFont ( const float height );

	constexpr auto	bentoRadius = 8.0f;
	constexpr auto	bentoGap = 8;

	constexpr auto	disabledAlpha = 0.35f;

	namespace strings
	{
		constexpr auto	hexDigits = "0123456789ABCDEF";
	}
}

namespace helpers
{
	// Window base title: app + version, plus the config suffix outside Release
	[[ nodiscard ]] juce::String appTitle ();

	std::pair<juce::String, juce::StringArray> parseActionMessage ( const juce::String& message );
	std::string createActionMessage ( const juce::String& command, const juce::StringArray& args );

	inline std::string strToLower ( std::string str )
	{
		std::ranges::transform ( str, str.begin (), [] ( unsigned char c ) { return std::tolower ( c ); } );
		return str;
	}

	int strnatcmp ( const char* const a, const char* const b );

	int romanToInt ( std::string s );
	std::string normalizeGamesString ( const std::string& input );
}

namespace paths
{
	juce::File getAppDataPath ( const juce::String& file );
	juce::File getSearchtermsPath ();
}
