#include "GUI_About.h"
#include "Globals/constants.h"

//-----------------------------------------------------------------------------

GUI_About::GUI_About ()
	: juce::Component ( "about" )
{
	icon.setName ( "icon" );
	title.setName ( "title" );
	copyright.setName ( "copyright" );

	// URL
	link.setName ( "link" );
	link.setFont ( UI::font ( 20.0f, 700 ), false, juce::Justification::centredLeft );

	icon.mipMap.setImage ( paths::getDataRoot ( "UI/png/ultraView.png" ) );

	about.addAndMakeVisible ( icon );
	about.addAndMakeVisible ( title );
	about.addAndMakeVisible ( copyright );
	about.addAndMakeVisible ( link );
	addAndMakeVisible ( about );

	scrollTextViewer.setName ( "scrollText" );
	scrollTextViewer.setFont ( UI::monoFont ( 16.0f ) );
	addAndMakeVisible ( scrollTextViewer );

	updateColors ();

	closeAbout.margin = 14.0f;
	closeAbout.bckAlpha[ 0 ] = 0.2f;
	closeAbout.bckAlpha[ 1 ] = 0.4f;
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
