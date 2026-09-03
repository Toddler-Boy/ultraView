#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class GUI_ultraView_logo final : public juce::Button
{
public:
	GUI_ultraView_logo ( const juce::String& name, const juce::String& resource );

	// juce::Button
	void paintButton ( juce::Graphics& g, bool isHover, bool isDown ) override;

private:
	struct SvgOutline
	{
		int			roleId;
		juce::Path	path;
	};

	void collectPaths ( const juce::Drawable& d, const juce::AffineTransform& parentTransform );

	std::vector<SvgOutline>	paths;
	juce::Rectangle<float>	logoBounds;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_ultraView_logo )
};
//-----------------------------------------------------------------------------
