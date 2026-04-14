#pragma once

#include <JuceHeader.h>

#include "Globals/constants.h"
#include "Globals/Icons.h"
#include "Globals/Strings.h"

//-----------------------------------------------------------------------------

class GUI_SVG_Button : public juce::Button
{
public:
	GUI_SVG_Button ( const juce::String& buttonName, const juce::StringArray& _svgNames );

	// juce::Button
	void clicked ( const juce::ModifierKeys& modifiers ) override;
	void paintButton ( juce::Graphics& g, bool isHover, bool /*isDown*/ ) override;

	// juce::Component
	void enablementChanged () override;

	// juce::TooltipClient
	juce::String getTooltip () override;

	// this
	int getStage () const;
	void setStage ( int newStage );

	int		bckColId = UI::colors::text;
	float	bckAlpha[ 2 ] = { 0.0f, 0.0f };
	float	bckRadius = 100'000.0f;
	float	bckMargin = 0.0f;

	juce::StringArray			tooltips;

	juce::StringArray			svgNames;
	float						margin = 0.0f;
	int							colId = UI::colors::text;
	float						alpha[ 2 ] = { 1.0f, 1.0f };
	juce::RectanglePlacement	placement = juce::RectanglePlacement::centred;
	bool						useOrgSize = true;
	juce::Point<float>			translation;

protected:
	juce::SharedResourcePointer<Icons>		icons;
	juce::SharedResourcePointer<Strings>	strings;

private:
	int		stage = 0;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_SVG_Button )
};
//-----------------------------------------------------------------------------
