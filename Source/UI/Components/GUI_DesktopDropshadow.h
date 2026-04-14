#pragma once

#include <JuceHeader.h>
//-------------------------------------------------------------------------------------------------

class GUI_DesktopDropshadow	final : public juce::Component, private juce::ComponentListener
{
public:
	GUI_DesktopDropshadow ( juce::Component& owner );
	~GUI_DesktopDropshadow () override;

private:
	void updatePosition ();
		
	void componentMovedOrResized ( juce::Component&, bool, bool ) override;
	void componentVisibilityChanged ( juce::Component& ) override;
	void componentBroughtToFront (juce::Component& ) override;
	void componentBeingDeleted ( juce::Component& ) override;
	void paint ( juce::Graphics& ) override;

	juce::Component& owner;

	juce::Path				shadowPath;
	melatonin::DropShadow	shadow { 11.0 };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( GUI_DesktopDropshadow )
};
//-------------------------------------------------------------------------------------------------
