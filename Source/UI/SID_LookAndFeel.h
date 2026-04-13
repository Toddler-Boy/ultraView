#pragma once

#include <JuceHeader.h>

class MipMap;

class SID_LookAndFeel final : public juce::LookAndFeel_V4, public juce::MouseListener
{
public:
	SID_LookAndFeel ();
	~SID_LookAndFeel () override;

	juce::Font fontWithHeight ( const float height )
	{
		return juce::Font ( defaultFont.withHeight ( height) );
	}

	juce::Font monoFontWithHeight ( const float height )
	{
		return juce::Font ( monoFont.withHeight ( height ) );
	}

	juce::Font font ( const float height, const int weight )
	{
		fontTableUsed[ weight / 100 ] = true;
		return juce::Font ( fontTable[ weight / 100 ].withHeight ( height ) );
	}

	//
	// Standard JUCE look and feel functions
	//
	juce::Font getSliderPopupFont ( juce::Slider& ) override	{		return fontWithHeight ( 17.0f );	}
	juce::Font getPopupMenuFont () override						{		return font ( 12.0f * 1.3f, 600 );	}
	juce::Font getTextButtonFont ( juce::TextButton&, int buttonHeight ) override
	{
		return fontWithHeight ( std::min ( 12.0f, buttonHeight * 0.6f ) );
	}

	juce::Font getComboBoxFont ( juce::ComboBox& box ) override
	{
		return font ( 12.0f * 1.3f, 600 );
	}

	juce::Font getLabelFont ( juce::Label& label ) override		{		return label.getFont ();	}

	//
	// Window decorations (titlebar, border, frame, etc.)
	//
	juce::Button* createDocumentWindowButton ( int ) override;
	void positionDocumentWindowButtons ( juce::DocumentWindow&, int titleBarX, int titleBarY, int titleBarW, int titleBarH, juce::Button* minimiseButton, juce::Button* maximiseButton, juce::Button* closeButton, bool positionTitleBarButtonsOnLeft ) override;
 	void drawResizableWindowBorder ( juce::Graphics&, int w, int h, const juce::BorderSize<int>& border, juce::ResizableWindow& window ) override;
	void drawResizableFrame ( juce::Graphics& g, int w, int h, const juce::BorderSize<int>& border ) override;
	void drawDocumentWindowTitleBar ( juce::DocumentWindow& window, juce::Graphics& g, int w, int h, int titleSpaceX, int titleSpaceW, const juce::Image* icon, bool drawTitleTextOnLeft ) override;
	void mouseEnter ( const juce::MouseEvent& e ) override;
	void mouseExit ( const juce::MouseEvent& e ) override;

	//
	// Scrollbar
	//
	int getDefaultScrollbarWidth () override;
	int getMinimumScrollbarThumbSize ( juce::ScrollBar& ) override;
	void drawScrollbar ( juce::Graphics&, juce::ScrollBar&, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown ) override;

	//
	// Tooltip
	//
	juce::TextLayout layoutTooltipText ( const juce::String& text ) noexcept;
	juce::Rectangle<int> getTooltipBounds ( const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea ) override;
	void drawTooltip ( juce::Graphics&, const juce::String& text, int width, int height ) override;

 	std::unique_ptr<juce::DropShadower> createDropShadowerForComponent ( juce::Component& comp ) override
 	{
		if ( dynamic_cast<juce::TooltipWindow*> ( &comp ) )
			return nullptr;

		return juce::LookAndFeel_V4::createDropShadowerForComponent ( comp );
 	}

	//
	// PopupMenu
	//
	int getMenuWindowFlags () override { return 0; }	// This disable the default shadow (completely square and ugly)
	void preparePopupMenuWindow ( juce::Component& newWindow ) override;
	void drawPopupMenuBackground ( juce::Graphics& g, int width, int height ) override;
	void drawPopupMenuItem ( juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText, const juce::Drawable* icon, const juce::Colour* textColour ) override;
	void drawPopupMenuSectionHeader ( juce::Graphics& g, const juce::Rectangle<int>& area, const juce::String& sectionName ) override;
	void drawPopupMenuColumnSeparatorWithOptions ( juce::Graphics& g, const juce::Rectangle<int>& area, const juce::PopupMenu::Options& ) override;
	int getPopupMenuColumnSeparatorWidthWithOptions ( const juce::PopupMenu::Options&) override;
	int getPopupMenuBorderSizeWithOptions ( const juce::PopupMenu::Options& ) override;
	void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int standardMenuItemHeight, int& idealWidth, int& idealHeight) override;

	//
	// Misc
	//
	void drawBubble ( juce::Graphics&, juce::BubbleComponent&, const juce::Point<float>& tip, const juce::Rectangle<float>& body ) override;
	int getSliderPopupPlacement ( juce::Slider& ) override;

	void positionComboBoxText ( juce::ComboBox& box, juce::Label& label ) override;

	void drawLabel ( juce::Graphics&, juce::Label& ) override;
	void drawComboBox ( juce::Graphics&, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& ) override;

	void drawButtonBackground ( juce::Graphics& g, juce::Button& b, const juce::Colour& backgroundColour, bool isHover, bool isDown ) override;
	void drawButtonText ( juce::Graphics&, juce::TextButton&, bool isHover, bool isDown ) override;

	void fillTextEditorBackground ( juce::Graphics&, int width, int height, juce::TextEditor& ) override;
	void drawTextEditorOutline ( juce::Graphics&, int width, int height, juce::TextEditor& ) override;

	void drawTableHeaderBackground ( juce::Graphics& g, juce::TableHeaderComponent& th ) override;
	void drawTableHeaderColumn ( juce::Graphics& g, juce::TableHeaderComponent& th, const juce::String& columnName, int columnId, int width, int height, bool isMouseOver, bool isMouseDown, int columnFlags ) override;

	juce::Label* createSliderTextBox ( juce::Slider& ) override;
	void drawLinearSlider ( juce::Graphics&, int x, int y, int width, int height,
							float sliderPos, float minSliderPos, float maxSliderPos,
							const juce::Slider::SliderStyle, juce::Slider& ) override;

	//
	// ultraSID specific stuff
	//
	static void drawRasterBars ( juce::Graphics& g, juce::Rectangle<float> b );
	static void drawPlaybackAnimation ( juce::Graphics& g, const juce::Rectangle<float>& rect, const juce::Colour color, const float animSpeed );
	static void drawOutlinedRect ( juce::Graphics& g, const juce::Rectangle<float>& rect, const float radius, const float outline, const juce::Colour outlineCol );

	void updateProgressColors ();

private:
	juce::FontOptions	defaultFont;

	std::array<juce::FontOptions, 10>	fontTable;	// weights 100-900 (including unused 0)
	juce::FontOptions	monoFont;

	bool	fontTableUsed[ 10 ] = { false };

	juce::ColourGradient	progressSlider;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( SID_LookAndFeel )
};
//----------------------------------------------------------------------------------
