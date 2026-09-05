#pragma once

// The custom juce::Colour IDs, keep this header cheap, (almost) everybody
// includes it. Only the tokens expand here; Theme.cpp expands paths and
// defaults into its color table, so juce is not needed at this point.

// X ( enum token, "block/key" theme path, default colour )
#define COLOR_ROLES(X) \
	X(window,			"colors/window",			juce::Colour ( 0xff'070912 )) \
	X(text,				"colors/text",				juce::Colour ( 0xff'E4E9F4 )) \
	X(accent,			"colors/accent",			juce::Colour ( 0xff'25856b )) \
	X(fxReal,			"colors/fx-real",			juce::Colour ( 0xff'e4e4e7 )) \
	X(fxPure,			"colors/fx-pure",			juce::Colour ( 0xff'33ffee )) \
	X(fxMagic,			"colors/fx-magic",			juce::Colour ( 0xff'ffd432 )) \
	X(statusOk,			"colors/status-ok",			juce::Colour ( 0xff'66ff99 )) \
	X(statusWarning,	"colors/status-warning",	juce::Colour ( 0xff'ffd432 )) \
	X(statusError,		"colors/status-error",		juce::Colour ( 0xff'ff3636 )) \
	X(statusUnknown,	"colors/status-unknown",	juce::Colour ( 0xff'5a6673 )) \
	X(actionOk,			"colors/action-ok",			juce::Colour ( 0xff'00aa55 )) \
	X(actionInfo,		"colors/action-info",		juce::Colour ( 0xff'4466bb )) \
	X(actionWarning,	"colors/action-warning",	juce::Colour ( 0xff'bb7700 )) \
	X(actionDanger,		"colors/action-danger",		juce::Colour ( 0xff'cc0000 )) \
	X(logo,				"colors/logo",				juce::Colour ( 0xff'ffff00 )) \
	X(logoOutline,		"colors/logo-outline",		juce::Colour ( 0xff'ff0000 )) \
	X(keycapFill,		"keycap/fill",				juce::Colour ( 0x1f'ffffff )) \
	X(keycapOutline,	"keycap/outline",			juce::Colour ( 0x40'ffffff ))

namespace UI
{
	enum colors
	{
		// Custom ColourIds must not collide with JUCE's built-in IDs (which
		// live around 0x100xxxx), this base starts a private range above them
		colorsIdBase = 0x1008000 - 1,

		#define X(role, name, col) role,
		COLOR_ROLES ( X )
		#undef X

		// End of themed stuff
		count,

		bento,
		textMuted,
		accentBright,
	};
}
