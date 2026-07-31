#pragma once

#include <JuceHeader.h>

#include <cstdint>

//-----------------------------------------------------------------------------

// Themed line widths: enum token, "block/key" path in the theme file (blocks
// group all of a component's properties, CSS-class style), default width in
// pixels (the values every theme starts from).
// Derived strokes stay in code: window-button glyphs (shape space), GUI_Line's
// per-instance width
#define LINE_ROLES(X) \
	X(tooltip,			"tooltip/line",			1.2f) \
	X(text_editor,		"text-editor/line",		2.0f) \
	X(menu_body,		"menu/line",			1.0f) \
	X(menu_separator,	"menu/separator-line",	1.0f) \
	X(menu_column_separator, "menu/column-separator-line", 1.5f) \
	X(slider_bubble,	"slider-bubble/line",	1.0f) \
	X(settings_location_button,	"settings-location-button/line",	1.5f) \
	X(settings_box,		"settings/box-line",	1.0f) \
	X(search_bar,		"search-bar/line",		2.0f) \
	X(tag_button,		"tag-button/line",		1.5f) \
	X(tag_button_on,	"tag-button/line-on",	0.5f) \
	X(main_menu_badge,	"main-menu/badge-line",	1.0f) \
	X(grid_outline,		"grid/outline-line",	0.75f) \
	X(grid_drag,		"grid/drag-line",		2.0f) \
	X(stil_box,			"stil-box/line",		0.75f) \
	X(fft_curve,		"fft/curve-line",		1.0f) \
	X(chip_voice_frequency, "fft/voice-frequency-line", 2.0f) \
	X(chip_states,		"chip-states/line",		1.5f) \
	X(eq_curve,			"eq-curve/line",		1.5f) \
	X(xy_pad,			"xy-pad/line",			1.0f) \
	X(chip_divot,		"chips/divot-line",		1.5f)

namespace UI::lines
{
	// A theme width of 0 hides a line: callers skip the stroke (and building
	// its path) entirely when this says no
	[[ nodiscard ]] constexpr bool visible ( const float width )	{	return width > 0.01f;	}

	enum Role : int8_t
	{
		#define X(role, name, w) role,
		LINE_ROLES ( X )
		#undef X
		count
	};

	// Resolves a role by its theme-file path ("search-bar/line", ...);
	// count = no such role
	[[ nodiscard ]] inline Role fromName ( const juce::String& roleName )
	{
		static constexpr const char* names[] =
		{
			#define X(role, name, w) name,
			LINE_ROLES ( X )
			#undef X
		};

		for ( auto i = 0; i < int ( std::size ( names ) ); ++i )
			if ( roleName == names[ i ] )
				return Role ( i );

		return count;
	}
}
//-----------------------------------------------------------------------------
