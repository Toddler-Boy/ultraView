#pragma once

#include <JuceHeader.h>

#include <cstdint>

#include "UI/ui-roles.h"

//-----------------------------------------------------------------------------

// Themed paddings: enum token, "block/key" path in the theme file (blocks
// group all of a component's properties, CSS-class style), single default
// applied to all four sides.
// The theme file takes CSS shorthand: one value = all sides, two = top/bottom
// left/right, three = top left&right bottom, four = top right bottom left.
// Negative values grow the rect instead of shrinking it
#define PADDING_ROLES(X) \
	X(tooltip,			"tooltip/padding",		9.0f) \
	X(grid_mini,		"grid-mini/padding",	8.0f) \
	X(chip_history,		"chip-states/history-padding",		2.0f) \
	X(chip_filter_mode,	"chip-states/filter-mode-padding",	4.0f) \
	X(chip_waveform,	"chip-states/waveform-padding",		6.0f) \
	X(chip_control,		"chip-states/control-padding",		2.0f) \
	X(dialog_entry,		"dialog/entry-padding",				8.0f)

namespace UI::paddings
{
	enum Role : int8_t
	{
		#define X(role, name, p) role,
		PADDING_ROLES ( X )
		#undef X
		count
	};

	// Per-side padding, CSS clockwise order
	struct Def
	{
		float	top;
		float	right;
		float	bottom;
		float	left;
	};

	// Resolves a role by its theme-file path ("stil-box/padding", ...);
	// count = no such role
	[[ nodiscard ]] inline Role fromName ( const juce::String& roleName )
	{
		static constexpr const char* names[] =
		{
			#define X(role, name, p) name,
			PADDING_ROLES ( X )
			#undef X
		};

		return Role ( UI::roleIndex ( names, int ( std::size ( names ) ), roleName ) );
	}
}
//-----------------------------------------------------------------------------
