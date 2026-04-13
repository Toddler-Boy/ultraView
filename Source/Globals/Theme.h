#pragma once

#include <JuceHeader.h>
#include "Globals/constants.h"

//-----------------------------------------------------------------------------

class Theme final
{
public:
	Theme ()
	{
		jassert ( colorDefinitions.size () == ( UI::colors::count - UI::colors::window ) );
	}
	//-----------------------------------------------------------------------------

	void setTargetLAF ( juce::LookAndFeel& _laf )
	{
		laf = &_laf;
	}
	//-----------------------------------------------------------------------------

	void load ( const juce::String& name )
	{
		// This needs a target juce::LookAndFeel to work
		jassert ( laf );

		auto	themeMap = std::unordered_map<juce::String, juce::Colour> {};

		if ( root.isDirectory () )
		{
			if ( auto file = root.getChildFile ( name + ".ini" ); file.existsAsFile () )
			{
				enum section
				{
					secGlobal,
					secColors,
					secUnknown
				};

				section	sec = secGlobal;

				// Convert theme file to map of colors
				juce::StringArray	lines;
				file.readLines ( lines );

				lines.removeEmptyStrings ();
				lines.trim ();

				for ( const auto& line : lines )
				{
					if ( line.startsWithChar ( '[' ) && line.endsWithChar ( ']' ) )
					{
						auto	sectionName = line.substring ( 1, line.length () - 1 ).toLowerCase ().trim ();
						if ( sectionName == "colors" )
							sec = secColors;
						else
							sec = secUnknown;

						continue;
					}

					auto	key = line.upToFirstOccurrenceOf ( "=", false, false ).trim ().toLowerCase ();
					auto	value = line.fromFirstOccurrenceOf ( "=", false, false ).trim ().toLowerCase ();

					if ( sec == secColors )
					{
						auto getFileColor = [] ( juce::String colStr )
						{
							// Named color
							auto	col = juce::Colours::findColourForName ( colStr, juce::Colours::transparentWhite );
							if ( col != juce::Colours::transparentWhite )
								return col;

							// Not correct format?
							if ( ! colStr.startsWithChar ( '#' ) )
								return juce::Colours::transparentWhite;

							colStr = colStr.substring ( 1 );
							if ( ! colStr.containsOnly ( "0123456789abcdef" ) )
								return juce::Colours::transparentWhite;

							// Swap alpha into the right position
							if ( colStr.length () == 3 )
								colStr = juce::String::formatted ( "ff%c%c%c%c%c%c", colStr[ 0 ], colStr[ 0 ], colStr[ 1 ], colStr[ 1 ], colStr[ 2 ], colStr[ 2 ] );
							else if ( colStr.length () == 4 )
								colStr = juce::String::formatted ( "%c%c%c%c%c%c%c%c", colStr[ 3 ], colStr[ 3 ], colStr[ 0 ], colStr[ 0 ], colStr[ 1 ], colStr[ 1 ], colStr[ 2 ], colStr[ 2 ] );
							else if ( colStr.length () == 6 )
								colStr = "ff" + colStr;
							else if ( colStr.length () == 8 )
								colStr = colStr.substring ( 6, 6 + 2 ) + colStr.substring ( 0, 6 );
							else
								return juce::Colours::transparentWhite;

							return juce::Colour::fromString ( colStr );
						};

						// Skip unknown color names
						if ( ! colorDefinitions.contains ( key ) )
						{
							Z_ERR ( "Unknown color name (" << key << ") in theme" );
							continue;
						}

						// Store valid colors in themeMap
						if ( const auto col = getFileColor ( value ); col != juce::Colours::transparentWhite )
							themeMap[ key ] = col;
					}
				}
			}
		}

		// Apply colors
		{
			for ( const auto& [ colName, idDef ] : colorDefinitions )
				if ( themeMap.contains ( colName ) )
					laf->setColour ( idDef.first, themeMap[ colName ] );
				else
					laf->setColour ( idDef.first, idDef.second );
		}
	}
	//-----------------------------------------------------------------------------

	void setRoot ( const juce::File& _root )
	{
		if ( _root.isDirectory () )
			root = _root;
		else
			root = juce::File ();
	}
	//-----------------------------------------------------------------------------

	juce::File& getRoot ()	{	return root;	}
	//-----------------------------------------------------------------------------

private:
	const std::map<juce::String, std::pair<int, juce::Colour>>	colorDefinitions =
	{
		// Window
		{ "window",					{ UI::colors::window,				juce::Colour ( 0xff'070912 ) } },
		{ "text",					{ UI::colors::text,					juce::Colour ( 0xff'E4E9F4 ) } },

		// Accent
		{ "accent",					{ UI::colors::accent,				juce::Colours::deeppink } },
		{ "accent2",				{ UI::colors::accent2,				juce::Colours::deeppink.withRotatedHue ( 300.0f / 360.0f ) } },

		// Browser
		{ "tag-liked",				{ UI::colors::tagLiked,				juce::Colour ( 0xff'f23d5b ) } },
		{ "tag-golden-age",			{ UI::colors::tagGoldenAge,			juce::Colour ( 0xff'ffd432 ) } },
		{ "tag-gem",				{ UI::colors::tagGem,				juce::Colour ( 0xff'77b9e8 ) } },

		// STIL
		{ "stil-toggle-stingers",	{ UI::colors::stilToggleStingers,	juce::Colour ( 0xff'00b3ff ) } },
		{ "stil-toggle-stil",		{ UI::colors::stilToggleSTIL,		juce::Colour ( 0xff'00ff95 ) } },

		{ "stil-box-title",			{ UI::colors::stilBoxTitle,			juce::Colour ( 0xff'333355 ) } },
		{ "stil-box-title-icon1",	{ UI::colors::stilBoxTitleIcon1,	juce::Colour ( 0xff'9999ff ) } },
		{ "stil-box-title-icon2",	{ UI::colors::stilBoxTitleIcon2,	juce::Colour ( 0xff'0000ff ) } },
		{ "stil-box-quote",			{ UI::colors::stilBoxQuote,			juce::Colour ( 0xff'2f3235 ) } },
		{ "stil-box-bug",			{ UI::colors::stilBoxBug,			juce::Colour ( 0x99'ff0000 ) } },

		// Chips
		{ "chip-divot",				{ UI::colors::chipDivot,			juce::Colour ( 0x88'000000 ) } },
		{ "chip-text",				{ UI::colors::chipText,				juce::Colour ( 0x66'f0f8ff ) } },

		// Off state for all voices and filters
		{ "voice-off",				{ UI::colors::voiceOff,				juce::Colour ( 0xff'565e66 ) } },
		{ "voice-on",				{ UI::colors::voiceOn,				juce::Colour ( 0xff'66ff99 ) } },
		{ "voice-muted",			{ UI::colors::voiceMuted,			juce::Colour ( 0xff'ff3636 ) } },
		{ "filter-on",				{ UI::colors::filterOn,				juce::Colour ( 0xff'66ffff ) } },
		{ "digi",					{ UI::colors::digi,					juce::Colour ( 0xff'00ffc8 ) } },

		// FX modes
		{ "fx-real",				{ UI::colors::fxReal,				juce::Colour ( 0xff'e4e4e7 ) } },
		{ "fx-pure",				{ UI::colors::fxPure,				juce::Colour ( 0xff'33ffee ) } },
		{ "fx-magic",				{ UI::colors::fxMagic,				juce::Colour ( 0xff'ffd432 ) } },

		// Status display
		{ "status-ok",				{ UI::colors::statusOk,				juce::Colour ( 0xff'66ff99 ) } },
		{ "status-warning",			{ UI::colors::statusWarning,		juce::Colour ( 0xff'ffd432 ) } },
		{ "status-error",			{ UI::colors::statusError,			juce::Colour ( 0xff'ff3636 ) } },
	};

	const std::array<juce::Colour, UI::colors::count - UI::colors::accent>	colors;

	juce::File			root;
	juce::LookAndFeel*	laf = nullptr;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( Theme )
};
//-----------------------------------------------------------------------------
