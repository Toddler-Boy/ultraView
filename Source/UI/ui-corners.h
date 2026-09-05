#pragma once

#include <JuceHeader.h>

#include <cstdint>

#include "UI/ui-roles.h"

//-----------------------------------------------------------------------------

// Themed corner radii: enum token, "block/key" path in the theme file (blocks
// group all of a component's properties, CSS-class style), default radius in
// pixels (the values every theme starts from).
// Derived radii (pills via height / 2, circles, dots) stay in code
#define CORNER_ROLES(X) \
	X(quality_selector,	"quality-selector/corner",	8.0f) \
	X(main_menu_button,	"main-menu/button-corner",	8.0f) \
	X(playlist_cover,	"playlist/cover-corner",	5.0f) \
	X(search_bar,		"search-bar/corner",		1000.0f) \
	X(tag_button,		"tag-button/corner",		1000.0f) \
	X(tooltip,			"tooltip/corner",			4.0f) \
	X(drop_down,		"drop-down/corner",			5.0f) \
	X(menu_body,		"menu/corner",				8.0f) \
	X(menu_highlight,	"menu/highlight-corner",	5.0f) \
	X(dialog_body,		"dialog/corner",			10.0f) \
	X(text_editor,		"text-editor/corner",		2.5f) \
	X(settings_box,		"settings/box-corner",		10.0f) \
	X(transport_bubble,	"transport/bubble-corner",	4.0f) \
	X(quality_button,	"quality-button/corner",	8.0f) \
	X(quality_selector_button,	"quality-selector/button-corner",	8.0f) \
	X(grid_big,			"grid-big/corner",			8.0f) \
	X(grid_mini,		"grid-mini/corner",			12.0f) \
	X(stil_box,			"stil-box/corner",			8.0f) \
	X(chip_states,		"chip-states/corner",		2.0f) \
	X(memory_overview,	"memory-overview/corner",	2.0f) \
	X(fft_clip,			"fft/clip-corner",			2.0f) \
	X(fft_curve,		"fft/curve-corner",			3.0f) \
	X(footer_thumbnail,	"footer/thumbnail-corner",	3.0f) \
	X(browser_list_row,	"browser/row-corner",		3.0f) \
	X(browser_thumbnail, "browser/thumbnail-corner", 3.0f) \
	X(badge,			"badge/corner",				1000.0f) \
	X(keycap,			"keycap/corner",			5.0f) \
	X(focus_ring,		"focus-ring/corner",		6.0f)

namespace UI::corners
{
	enum Role : int8_t
	{
		#define X(role, name, r) role,
		CORNER_ROLES ( X )
		#undef X
		count
	};

	// Resolves a role by its theme-file path ("search-bar/corner", ...);
	// count = no such role
	[[ nodiscard ]] inline Role fromName ( const juce::String& roleName )
	{
		static constexpr const char* names[] =
		{
			#define X(role, name, r) name,
			CORNER_ROLES ( X )
			#undef X
		};

		return Role ( UI::roleIndex ( names, int ( std::size ( names ) ), roleName ) );
	}
}
//-----------------------------------------------------------------------------
