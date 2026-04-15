#pragma once

#include <format>
#include <iostream>
#include <string>
#include <utility>

template <>
struct std::formatter<juce::String> : std::formatter<std::string_view> {
	auto format ( const juce::String& s, std::format_context& ctx ) const {
		return std::formatter<string_view>::format ( std::string ( s.toStdString () ), ctx );
	}
};

namespace UI
{
	enum colors
	{
		// Basic
		window = 0x1008000,
		text,

		accent,

		// FX mode
		fxReal,
		fxPure,
		fxMagic,

		// Status display
		statusOk,
		statusWarning,
		statusError,

		// Action buttons
		actionOk,
		actionInfo,
		actionWarning,
		actionDanger,

		// End of stuff
		count,

		bento,
		textMuted,
		accentBright,
	};

	namespace shades
	{
		constexpr auto	hover = 1.0f / 12.0f;
		constexpr auto	selected = 1.0f / 6.0f;
	}

	extern juce::ActionBroadcaster*	ab;
	void setActionBroadCaster ( juce::ActionBroadcaster* _ab ) noexcept;

	template<typename... Args>
	void sendGlobalMessage ( std::string_view fmt_str, Args&&... args )
	{
		auto	format_args = std::make_format_args ( args... );
		auto	formatted = std::vformat ( fmt_str, format_args );

		jassert ( ab );
		ab->sendActionMessage ( formatted );
	}

	void setShades ( const juce::Colour col1, const juce::Colour col2 ) noexcept;
	[[ nodiscard ]] juce::Colour getShade ( const float blend ) noexcept;
	[[ nodiscard ]] juce::Colour getColorFromName ( const juce::String& name, const float brightness = 0.25f );
	[[ nodiscard ]] juce::String getHumanNumber ( int64_t number, const char thousand_separator = ',' );
	[[ nodiscard ]] float easeInOutQuad ( float t );
	[[ nodiscard ]] juce::Colour getColorWithPerceivedBrightness ( const juce::Colour col, const float perceivedBrightness ) noexcept;

	[[ nodiscard ]] juce::Font font ( const float height, const int weight = 400 );
	[[ nodiscard ]] juce::Font monoFont ( const float height );

	[[ nodiscard ]] std::pair<std::unique_ptr<juce::Drawable>, int> getSVG ( const juce::String& svgName );
	[[ nodiscard ]] juce::Path& getScaledPath ( const juce::String& resourceName, juce::Rectangle<float> rect, juce::RectanglePlacement placement = 0, float padding = 0.0f );
	[[ nodiscard ]] juce::Path& getScaledPathWithSize ( const juce::String& resourceName, juce::Rectangle<float> rect, juce::RectanglePlacement placement = 0, float padding = 0.0f );

	constexpr auto	bentoRadius = 8.0f;
	constexpr auto	bentoGap = 8;

	constexpr auto	disabledAlpha = 0.35f;

	extern juce::Colour	startColor;
	extern juce::Colour	endColor;

	namespace strings
	{
		constexpr auto	hexDigits = "0123456789ABCDEF";
	}
}

namespace helpers
{
	std::pair<juce::String, juce::StringArray> parseActionMessage ( const juce::String& message );
	std::string createActionMessage ( const juce::String& command, const juce::StringArray& args );
	juce::StringArray getFilteredStrings ( const juce::StringArray& arr, const juce::StringArray& ext );

	inline std::string strToLower ( std::string str )
	{
		std::ranges::transform ( str, str.begin (), [] ( unsigned char c ) { return std::tolower ( c ); } );
		return str;
	}

	int strnatcmp ( const char* const a, const char* const b );

	void buildComponentMap ( std::unordered_map<juce::String, juce::Component*>& compMap, juce::Component* parent, const juce::String& name = "" );

	template<typename T>
	void getChildrenOfClass ( juce::Component* parent, std::vector<T*>& comps )
	{
		// Loop over all children recursivly and build a vector of components of class T
		for ( auto comp : parent->getChildren () )
		{
			if ( auto tComp = dynamic_cast<T*> ( comp ) )
				comps.push_back ( tComp );

			getChildrenOfClass<T> ( comp, comps );
		}
	}

	template<typename T>
	T* findComponent ( const juce::String& name, const std::unordered_map<juce::String, juce::Component*>& compMap )
	{
		auto	it = compMap.find ( name );

		// Component not found
		jassert ( it != compMap.end () );

		return dynamic_cast<T*> ( it->second );
	}
}

namespace paths
{
	juce::File getDataRoot ( juce::String path = "" );
}
