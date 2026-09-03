#pragma once

#include <JuceHeader.h>

#include <cstdint>

#include "UI/ui-roles.h"

//-----------------------------------------------------------------------------

// Themed font roles: enum token, "block/key" path in the theme file (blocks
// group all of a component's properties, CSS-class style), default point size
// and weight (the values every theme starts from).
// Point size = the em size, directly comparable to a CSS px value
#define FONT_ROLES(X) \
	X(browser_text,	"browser/text-font",	13.1f,	500) \
	X(browser_small, "browser/small-font",	11.4f,	500) \
	X(page_title,	"page-title/font",		32.0f,	800) \
	X(crt_header,	"crt/header-font",		15.0f,	700) \
	X(crt_label,	"crt/label-font",		11.0f,	600) \
	X(drop_down,	"drop-down/font",		12.0f,	700) \
	X(tooltip,		"tooltip/font",			13.8f,	500) \
	X(table_header,	"table-header/font",	12.0f,	700) \
	X(main_menu,	"main-menu/font",		13.1f,	500) \
	X(settings_location_button, "settings-location-button/font", 12.3f, 600) \
	X(onboarding_progress, "onboarding/progress-font", 12.3f, 600) \
	X(footer_title,		"footer/title-font",		14.0f,	700) \
	X(footer_author,	"footer/author-font",		13.0f,	600) \
	X(footer_released,	"footer/released-font",		13.0f,	550) \
	X(settings_section,	"settings/section-font",	18.0f,	800) \
	X(settings_entry,	"settings/entry-font",		14.0f,	700) \
	X(settings_field,	"settings/field-font",		13.0f,	500) \
	X(settings_help,	"settings/help-font",		12.0f,	600) \
	X(settings_label,	"settings/label-font",		14.0f,	700) \
	X(stil_text,		"stil/text-font",			13.1f,	500) \
	X(stil_list,		"stil/list-font",			10.8f,	500) \
	X(stil_mono,		"stil/mono-font",			10.7f,	500) \
	X(stil_author,		"stil/author-font",			18.4f,	600) \
	X(onboarding_text,	"onboarding/text-font",		13.8f,	500) \
	X(search_bar,		"search-bar/font",			13.8f,	500) \
	X(search_info,		"search-info/font",			11.5f,	500) \
	X(fft_caption,		"fft/caption-font",			8.5f,	500) \
	X(transport_text,	"transport/font",			10.0f,	700) \
	X(crt_slider_choice, "crt/slider-choice-font",	10.0f,	700) \
	X(error_banner,		"error-banner/font",		13.1f,	800) \
	X(tag_button,		"tag-button/font",			10.0f,	600) \
	X(chip_labels,		"chips/labels-font",		9.6f,	800) \
	X(onboarding_button, "onboarding/button-font",	15.4f,	600) \
	X(onboarding_status, "onboarding/status-font",	12.3f,	600) \
	X(quality_button,	"quality-button/font",		13.8f,	600) \
	X(quality_selector_header,	"quality-selector/header-font",	15.4f,	800) \
	X(quality_selector_help,	"quality-selector/help-font",	13.1f,	500) \
	X(quality_selector_button,	"quality-selector/button-font",	8.5f,	800) \
	X(playlist_info,	"playlist/info-font",		10.0f,	700) \
	X(grid_big_header,	"grid-big/header-font",		32.0f,	700) \
	X(grid_mini_header,	"grid-mini/header-font",	12.9f,	700) \
	X(grid_mini_name,	"grid-mini/name-font",		12.9f,	700) \
	X(grid_mini_info,	"grid-mini/info-font",		12.0f,	600) \
	X(grid_big_title,	"grid-big/title-font",		15.4f,	700) \
	X(grid_big_authors,	"grid-big/authors-font",	10.8f,	500) \
	X(settings_location, "settings/location-font",	14.0f,	500) \
	X(dialog_title,		"dialog/title-font",		16.0f,	700) \
	X(dialog_entry,		"dialog/entry-font",		15.0f,	500) \
	X(badge,			"badge/font",				11.0f,	700)

namespace UI::fonts
{
	enum Role : int8_t
	{
		#define X(role, name, s, w) role,
		FONT_ROLES ( X )
		#undef X
		count
	};

	struct Def
	{
		float	size;	// in points
		int		weight;
	};

	// Resolves a role by its theme-file path ("search-bar/font", ...);
	// count = no such role
	[[ nodiscard ]] inline Role fromName ( const juce::String& roleName )
	{
		static constexpr const char* names[] =
		{
			#define X(role, name, s, w) name,
			FONT_ROLES ( X )
			#undef X
		};

		return Role ( UI::roleIndex ( names, int ( std::size ( names ) ), roleName ) );
	}
}
//-----------------------------------------------------------------------------
