#include "GUI_About.h"
#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_About::GUI_About ()
	: juce::Component ( "about" )
{
	mdDisplay.setName ( "display" );
	mdDisplay.setFont ( UI::monoFont ( 16.0f ) );
	addAndMakeVisible ( mdDisplay );

	updateColors ();

	closeAbout.margin = 14.0f;
	closeAbout.bckAlpha[ 0 ] = 0.2f;
	closeAbout.bckAlpha[ 1 ] = 0.5f;
	closeAbout.bckMargin = 6.0f;
	closeAbout.setSize ( 48, 48 );
	closeAbout.setWantsKeyboardFocus ( false );
	addAndMakeVisible ( closeAbout );

	closeAbout.onClick = [] {	UI::sendGlobalMessage ( "closeAbout" );	};

	loadContent ();
}
//-----------------------------------------------------------------------------

void GUI_About::resized ()
{
	layout.setLayout ( { paths::getDataRoot ( "UI/layouts/constants.json" ),
						 paths::getDataRoot ( "UI/layouts/screens/about.json" ) } );
}
//-----------------------------------------------------------------------------

bool GUI_About::handleURL ( juce::String url )
{
	juce::URL ( url ).launchInDefaultBrowser ();
	return true;
}
//-----------------------------------------------------------------------------

void GUI_About::updateColors ()
{
	const auto	col = UI::getShade ( 1.0f ).toString ();

	juce::StringPairArray	colors;

	colors.set ( "black", "#000" );
	colors.set ( "blue", "#00F" );
	colors.set ( "green", "#0B0" );
	colors.set ( "red", "#C00" );
	colors.set ( "yellow", "#BB0" );
	colors.set ( "orange", "#F92" );
	colors.set ( "linkcolour", "#77F" );

	colors.set ( "default", col );

	mdDisplay.setColours ( colors );
	mdDisplay.setBGColour ( UI::getShade ( 0.1f ) );
}
//-----------------------------------------------------------------------------

void GUI_About::loadContent ()
{
	const auto	str = paths::getDataRoot ( "UI/about.md" ).loadFileAsString ();
	mdDisplay.setMarkdownString ( str );
}
//-----------------------------------------------------------------------------
