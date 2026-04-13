#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class GUI_RoundedClip final : public juce::Graphics::ScopedSaveState
{
public:
	GUI_RoundedClip ( juce::Graphics& g, const juce::Rectangle<float>& b, const float r )
		: juce::Graphics::ScopedSaveState ( g )
	{
		juce::Path	p;

		if ( r > 10'000.0f )
			p.addEllipse ( b );
		else if ( r > 0.5f )
			p.addRoundedRectangle ( b, r );
		else
			p.addRectangle ( b );

		g.reduceClipRegion ( p );
	}
};
//-----------------------------------------------------------------------------
