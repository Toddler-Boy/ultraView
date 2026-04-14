#pragma once

#include <JuceHeader.h>

#include "Globals/constants.h"

//-----------------------------------------------------------------------------

class GUI_Line final : public juce::Component
{
public:
	GUI_Line ( const juce::String& _name = "", const float _lineWidth = 1.0f)
		: juce::Component ( _name ), lineWidth ( _lineWidth )
	{
		setInterceptsMouseClicks ( false, false );
	}

	// juce::Component
	void paint ( juce::Graphics& g ) override
	{
		const auto	b = getLocalBounds ().toFloat ();

		g.setColour ( UI::getShade ( shade ).withMultipliedAlpha ( shadeAlpha ) );

		if ( b.getWidth () > b.getHeight () )
			g.fillRect ( b.withSizeKeepingCentre ( b.getWidth (), lineWidth ) );
		else
			g.fillRect ( b.withSizeKeepingCentre ( lineWidth, b.getHeight () ) );
	}

	// this
	void setShade ( const float newShade, const float newAlpha ) noexcept
	{
		shade = newShade;
		shadeAlpha = newAlpha;

		repaint ();
	}

private:
	const float	lineWidth;

	float	shade = 0.2f;
	float	shadeAlpha = 1.0f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_Line )
};
//-----------------------------------------------------------------------------
