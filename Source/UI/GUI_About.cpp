#include "GUI_About.h"
#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_About::GUI_About ()
	: juce::Component ( "about" )
{
	scrollTextViewer.setName ( "display" );
	scrollTextViewer.setFont ( UI::monoFont ( 16.0f ) );
	addAndMakeVisible ( scrollTextViewer );

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

void GUI_About::updateColors ()
{
	scrollTextViewer.setColour ( juce::Label::textColourId, UI::getShade ( 1.0f ) );
}
//-----------------------------------------------------------------------------

void GUI_About::loadContent ()
{
	const auto	str = paths::getDataRoot ( "UI/about.txt" ).loadFileAsString ();
	scrollTextViewer.setText ( str );
}
//-----------------------------------------------------------------------------
