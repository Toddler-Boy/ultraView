#include "SID_LookAndFeel.h"

#include "Misc/colodore.h"
#include "Components/GUI_DesktopDropshadow.h"
#include "Components/GUI_Toggle.h"

#include "UI/Misc/VIC2_Render.h"

#include "Globals/constants.h"

constexpr auto	tooltipFontSize = 18.0f;

//-------------------------------------------------------------------------------------------------

SID_LookAndFeel::SID_LookAndFeel ()
{
	//
	// Font
	//
	juce::Font::setDefaultMinimumHorizontalScaleFactor ( 1.0f );

	//
	// Populate fonts table
	//
	{
		auto loadFont = [] ( const juce::String& name )
		{
			auto	mb = juce::MemoryBlock ();

			if ( juce::File ( paths::getDataRoot ( "UI/fonts/" + name ) ).loadFileAsData ( mb ) )
				return juce::FontOptions ( juce::Typeface::createSystemTypefaceFor ( mb.getData (), mb.getSize () ) );

			return juce::FontOptions ();
		};

		constexpr auto	firstIndex = 2;

		auto	idx = firstIndex;
		fontTable[ idx++ ] = loadFont ( "Figtree-Light.ttf" );
		fontTable[ idx++ ] = loadFont ( "Figtree-Regular.ttf" );
		fontTable[ idx++ ] = loadFont ( "Figtree-Medium.ttf" );
		fontTable[ idx++ ] = loadFont ( "Figtree-SemiBold.ttf" );
		fontTable[ idx++ ] = loadFont ( "Figtree-Bold.ttf" );
		fontTable[ idx++ ] = loadFont ( "Figtree-ExtraBold.ttf" );
		fontTable[ idx++ ] = loadFont ( "Figtree-Black.ttf" );

		// Make sure there are fonts from 0 to first weight
		for ( auto i = 0; i < firstIndex; ++i )
			fontTable[ i ] = fontTable[ firstIndex ];

		// Make sure the weight goes all the way to 900
		for ( auto i = idx; i < std::ssize ( fontTable ); ++i )
			fontTable[ i ] = fontTable[ idx - 1 ];

		monoFont = loadFont ( "NotoSansMono-SemiBold-subset.ttf" );		// Monospace, weight 500, only ASCII

		std::ranges::fill ( fontTableUsed, false );
	}

	defaultFont = fontTable[ 400 / 100 ].withHeight ( 14.0f );

	//
	// Default colors
	//
	const std::pair<const int, juce::Colour> juceDefaultColors[] =
	{
		// JUCE stuff
		{ juce::ListBox::backgroundColourId,				juce::Colours::transparentBlack },
		{ juce::ListBox::outlineColourId,					juce::Colours::transparentBlack },

		{ juce::ComboBox::backgroundColourId,				juce::Colour::fromRGB ( 33, 42, 48 ) },
		{ juce::ComboBox::buttonColourId,					juce::Colour::fromRGB ( 33, 42, 48 ) },
		{ juce::ComboBox::outlineColourId,					juce::Colours::black },
		{ juce::ComboBox::textColourId,						juce::Colours::whitesmoke },
		{ juce::ComboBox::arrowColourId,					juce::Colours::whitesmoke },

		{ juce::ToggleButton::textColourId,					juce::Colours::whitesmoke	},
		{ juce::ToggleButton::tickColourId,					juce::Colours::green		},

		{ juce::TextEditor::backgroundColourId,				juce::Colours::white },
		{ juce::TextEditor::textColourId,                 	juce::Colours::black },
		{ juce::TextEditor::highlightColourId,            	juce::Colours::cornflowerblue },
		{ juce::TextEditor::highlightedTextColourId,      	juce::Colours::white },
		{ juce::TextEditor::outlineColourId,              	juce::Colours::transparentBlack },
		{ juce::TextEditor::focusedOutlineColourId,       	juce::Colours::cornflowerblue },

		{ juce::Label::textColourId,						juce::Colours::whitesmoke },
		{ juce::Label::backgroundWhenEditingColourId,		juce::Colours::white },
		{ juce::Label::textWhenEditingColourId,				juce::Colours::black },
		{ juce::Label::outlineWhenEditingColourId,			juce::Colours::deeppink },

		{ juce::ScrollBar::backgroundColourId,				juce::Colours::black.withAlpha ( 0.25f ) },
		{ juce::ScrollBar::thumbColourId,					juce::Colours::white.withAlpha ( 0.25f ) },
		{ juce::ScrollBar::trackColourId,					juce::Colours::white.withAlpha ( 0.5f ) },

		{ juce::BubbleComponent::backgroundColourId,		juce::Colour::fromRGB ( 33, 42, 48 ) },
		{ juce::BubbleComponent::outlineColourId,			juce::Colour::fromRGB ( 0, 0, 0 ) },

		{ juce::TooltipWindow::backgroundColourId,			juce::Colour::fromRGB ( 33, 42, 48 ) },
		{ juce::TooltipWindow::textColourId,				juce::Colour ( 0xff'F5FBFF ).withAlpha ( 0.9f ) },
		{ juce::TooltipWindow::outlineColourId,				juce::Colours::black.withAlpha ( 0.1f ) },

		{ juce::PopupMenu::backgroundColourId,				juce::Colour::fromRGB ( 33, 42, 48 ) },
		{ juce::PopupMenu::textColourId,					juce::Colour ( 0xff'F5FBFF ).withAlpha ( 0.9f ) },
		{ juce::PopupMenu::highlightedBackgroundColourId,	juce::Colour::fromRGB ( 33, 42, 48 ).interpolatedWith ( juce::Colour::fromRGB ( 240, 248, 255 ), 0.2f ) },
		{ juce::PopupMenu::highlightedTextColourId,			juce::Colour ( 0xff'F5FBFF ).withAlpha ( 0.9f ) },
	};

	for ( auto [ colorId, color ] : juceDefaultColors )
		setColour ( colorId, color );
}
//----------------------------------------------------------------------------------

SID_LookAndFeel::~SID_LookAndFeel ()
{
}
//----------------------------------------------------------------------------------

void SID_LookAndFeel::positionComboBoxText ( juce::ComboBox& box, juce::Label& label )
{
	auto	f = getComboBoxFont ( box );

	auto	b = box.getLocalBounds ().reduced ( int ( f.getAscent () * 0.8f ), 0 );

	const bool	drawArrow = box.getProperties ().getWithDefault ( "drawArrow", true );
	if ( drawArrow )
		label.setBounds ( b.withTrimmedRight ( b.getHeight () ) );
	else
		label.setBounds ( b );

	label.setInterceptsMouseClicks ( false, false );
	label.setFont ( f );
}
//----------------------------------------------------------------------------------

void SID_LookAndFeel::drawLabel ( juce::Graphics& g, juce::Label& l )
{
	// Make sure the correct color is used
	if ( auto box = dynamic_cast<juce::ComboBox*> ( l.getParentComponent () ) )
	{
		g.setColour ( box->findColour ( juce::ComboBox::textColourId, true ) );
		g.setFont ( getComboBoxFont ( *box ) );
	}
	else
	{
		g.setColour ( l.findColour ( juce::Label::textColourId, true ) );
		g.setFont ( l.getFont () );
	}

	const auto	textArea = l.getLocalBounds ();

	auto text = l.getText ();

	g.drawFittedText ( text, textArea, l.getJustificationType (),
					   juce::jmax ( 1, int ( textArea.getHeight () / g.getCurrentFont ().getHeight () ) ),
					   l.getMinimumHorizontalScale () );
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawComboBox ( juce::Graphics& g, int width, int height, bool /*isButtonDown*/, int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/, juce::ComboBox& box )
{
	auto	boxBounds = juce::Rectangle<float> { float ( width ), float ( height ) };

	auto	backCol = box.findColour ( juce::ComboBox::backgroundColourId, true );
	auto	arrowCol = box.findColour ( juce::ComboBox::arrowColourId, true );

	g.setColour ( backCol );
	g.fillRoundedRectangle ( boxBounds, 5.0f );

	// Draw arrow
	const bool	drawArrow = box.getProperties ().getWithDefault ( "drawArrow", true );
	if ( drawArrow )
	{
		auto&	p = UI::getScaledPath ( "angle-down-solid-full", boxBounds.removeFromRight ( boxBounds.getHeight () ), 0, 0.3f );

		g.setColour ( arrowCol );
		g.fillPath ( p );
	}
}
//----------------------------------------------------------------------------------

void SID_LookAndFeel::drawButtonBackground ( juce::Graphics& g, juce::Button& button, const juce::Colour& /*backgroundColour*/, bool isHover, bool /*isDown*/ )
{
	auto	bounds = button.getLocalBounds ().toFloat ().reduced ( 0.75f );

	g.setColour ( findColour ( isHover ? UI::colors::text : UI::colors::textMuted ) );
	g.drawRoundedRectangle ( bounds, bounds.getHeight () / 2.0f, 1.5f );
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawButtonText ( juce::Graphics& g, juce::TextButton& button, bool /*isHover*/, bool /*isDown*/ )
{
	auto	bounds = button.getLocalBounds ().toFloat ();

	g.setColour ( findColour ( UI::colors::text ) );
	g.setFont ( UI::font ( 16.0f, 500 ) );
	g.drawText ( button.getButtonText (), bounds, juce::Justification::centred, false );
}
//-------------------------------------------------------------------------------------------------

int SID_LookAndFeel::getDefaultScrollbarWidth ()
{
	return 16 + 6;
}
//-------------------------------------------------------------------------------------------------

int SID_LookAndFeel::getMinimumScrollbarThumbSize ( juce::ScrollBar& sb )
{
	return juce::LookAndFeel_V4::getMinimumScrollbarThumbSize ( sb ) / 2;
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawScrollbar ( juce::Graphics& g, juce::ScrollBar& scrollbar, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown )
{
	const auto	withThumb = isScrollbarVertical ? ( height != thumbSize ) : ( width != thumbSize );
	const auto	showFull = ( isMouseOver || isMouseDown ) && withThumb;

	const auto	reducedBackX = showFull ? 0.0f : 3.0f;
	const auto	reducedThumbX = showFull ? 3.0f : 5.5f;
	constexpr auto	reducedThumbY = 3.0f;

	constexpr auto	paddingTopOrLeft = 6;
	if ( isScrollbarVertical )
	{
		x += paddingTopOrLeft;
		width -= paddingTopOrLeft;
	}
	else
	{
		y += paddingTopOrLeft;
		height -= paddingTopOrLeft;
	}

	// Background
	if ( const auto col = scrollbar.findColour ( juce::ScrollBar::backgroundColourId, true ); ! col.isTransparent () )
	{
		g.setColour ( col );

		const auto	r = juce::Rectangle<int> ( x, y, width, height ).toFloat ().reduced ( reducedBackX, 0.0f );
		const auto	borderRadius = ( isScrollbarVertical ? r.getWidth () : r.getHeight () ) / 2.0f;

		g.fillRoundedRectangle ( r, borderRadius );
	}

	// Thumb
	if ( withThumb )
	{
		const auto	thumbBounds = isScrollbarVertical ?
												juce::Rectangle<int> ( x, thumbStartPosition, width, thumbSize )
											:	juce::Rectangle<int> ( thumbStartPosition, y, thumbSize, height );

		auto	trackCol = scrollbar.findColour ( juce::ScrollBar::trackColourId, true );
		if ( ! isMouseDown )
			trackCol = scrollbar.findColour ( juce::ScrollBar::thumbColourId, true ).interpolatedWith ( trackCol, isMouseOver ? 0.5f : 0.0f );

		g.setColour ( trackCol );

		const auto	fThumbBounds = thumbBounds.toFloat ().reduced ( reducedThumbX, reducedThumbY );
		const auto	borderRadius = ( isScrollbarVertical ? fThumbBounds.getWidth () : fThumbBounds.getHeight () ) / 2.0f;

		g.fillRoundedRectangle ( fThumbBounds, borderRadius );
	}
}
//----------------------------------------------------------------------------------

juce::TextLayout SID_LookAndFeel::layoutTooltipText ( const juce::String& text ) noexcept
{
	constexpr auto	maxToolTipWidth = 1000.0f;

	juce::AttributedString	s;
	const auto	font = fontWithHeight ( tooltipFontSize );

	const auto	bck = findColour ( juce::TooltipWindow::backgroundColourId );
	const auto	col = findColour ( juce::TooltipWindow::textColourId );

	s.setJustification ( juce::Justification::centredLeft );

	// Add each line separately, so we have more control over colors etc.
	{
		auto	arr = juce::StringArray::fromLines ( text );
		for ( auto& str : arr )
		{
			const auto	other = str.startsWithChar ( '#' ) ? 0.5f : 0.0f;

			s.append ( str.replaceFirstOccurrenceOf ( "#", "" ) + "\n", font, col.interpolatedWith ( bck, other ) );
		}
	}

	juce::TextLayout	tl;
	tl.createLayoutWithBalancedLineLengths ( s, maxToolTipWidth );
	return tl;
}
//-------------------------------------------------------------------------------------------------

juce::Rectangle<int> SID_LookAndFeel::getTooltipBounds ( const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea )
{
	const juce::TextLayout	tl ( layoutTooltipText ( tipText ) );

	const auto	w = int ( tl.getWidth () + tooltipFontSize * 3.0f );
	const auto	h = int ( tl.getHeight () + tooltipFontSize * 2.0f );

	return juce::Rectangle<int> ( screenPos.x > parentArea.getCentreX () ?
											  screenPos.x - int ( w )
											  : int ( float ( screenPos.x ) + tooltipFontSize ),
								  screenPos.y > parentArea.getCentreY () ?
											  screenPos.y - int ( float ( h ) + tooltipFontSize / 3)
											  : screenPos.y + int ( tooltipFontSize / 3 ),
								  w, h )
								  .constrainedWithin ( parentArea );
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawTooltip ( juce::Graphics& g, const juce::String& text, int width, int height )
{
	const auto	b = juce::Rectangle<float>	( float ( width ), float ( height ) );
	const auto	radius = 4.0f;
	const auto	offset = juce::Point<float> ( 0.0f, -( tooltipFontSize / 8.0f ) );

	// Perfect drop-shadow
	{
		auto	img = juce::Image ( juce::Image::PixelFormat::SingleChannel, width, height, true );

		{
			juce::Graphics	ig ( img );

			ig.setColour ( juce::Colours::white.withAlpha ( 0.5f ) );
			ig.fillRoundedRectangle ( b.reduced ( tooltipFontSize / 2.0f ), radius );
		}

		gin::applyStackBlur ( img, int ( tooltipFontSize / 2.0f ) );

		g.setColour ( juce::Colours::black );
		g.drawImage ( img, b, 0, true );
	}

	g.setColour ( findColour ( juce::TooltipWindow::backgroundColourId ) );
	g.fillRoundedRectangle ( b.reduced ( tooltipFontSize / 2.0f + 1.0f ) + offset, radius );

	if ( auto outCol = findColour ( juce::TooltipWindow::outlineColourId ); ! outCol.isTransparent () )
	{
		g.setColour ( outCol );
		g.drawRoundedRectangle ( b.reduced ( tooltipFontSize / 2.0f + 0.6f ) + offset, radius, 1.2f );
	}

	const juce::TextLayout	tl ( layoutTooltipText ( text ) );

	const float x = ( float ( width ) - tl.getWidth () ) / 2.0f;
	const float y = ( float ( height ) - tl.getHeight () ) / 2.0f;

	tl.draw ( g, juce::Rectangle<float> ( x, y, tl.getWidth (), tl.getHeight () ) + offset );
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::fillTextEditorBackground ( juce::Graphics& g, int width, int height, juce::TextEditor& textEditor )
{
	const auto	col = textEditor.findColour ( juce::TextEditor::backgroundColourId );

	g.setColour ( col );

	g.fillRoundedRectangle ( juce::Rectangle<float> { float ( width ), float ( height ) }.reduced ( 1.0f ), 2.5f );
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawTextEditorOutline ( juce::Graphics& g, int width, int height, juce::TextEditor& textEditor )
{
	if ( ! textEditor.hasKeyboardFocus ( true ) )
		return;

	const auto	col = textEditor.findColour ( juce::TextEditor::focusedOutlineColourId );
	if ( col.isTransparent () )
		return;

	g.setColour ( col );

	const auto	b = juce::Rectangle<float> { float ( width ) , float ( height ) };

	constexpr auto	thickness = 2.0f;
	g.drawRoundedRectangle ( b.reduced ( thickness / 2.0f ), 2.5f - thickness / 2.0f, thickness );
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawTableHeaderBackground ( juce::Graphics& g, juce::TableHeaderComponent& header )
{
/*	g.setColour ( findColour ( UI::colors::text ).withMultipliedAlpha ( 0.1f ) );

	auto&	props = header.getProperties ();
	const float	yOff = props.getWithDefault ( "line", 10.0f );

	auto	area = header.getLocalBounds ().toFloat ();
	g.fillRect ( area.removeFromBottom ( yOff ).withHeight ( 1.5f ) );*/
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawTableHeaderColumn ( juce::Graphics& g, juce::TableHeaderComponent& header, const juce::String& columnName, int columnId, int width, int height, bool isMouseOver, bool /*isMouseDown*/, int columnFlags )
{
//	height -= 10;

	auto&	props = header.getProperties ();
	const float	xOff = props.getWithDefault ( "colOff" + juce::String ( columnId ), 4.0f );

	const auto	colId = isMouseOver && ( columnFlags & juce::TableHeaderComponent::sortable ) ? UI::colors::text : UI::colors::textMuted;
	g.setColour ( findColour ( colId ) );

	const auto	fnt = font ( 12.0f * 1.3f, 600 );
	const auto	th = float ( height - 2 );
	float		tw = float ( width ) - xOff;

	const juce::String	icon = props.getWithDefault ( "colIcon" + juce::String ( columnId ), juce::String () );

	const auto	colName = columnName.toUpperCase ();

	if ( ( columnFlags & ( juce::TableHeaderComponent::sortedForwards | juce::TableHeaderComponent::sortedBackwards ) ) != 0 )
	{
		tw = width - th;

		if ( icon.isEmpty () )
			tw = std::min ( juce::GlyphArrangement::getStringWidth ( fnt, colName ), tw );

		auto	area = juce::Rectangle<float> { tw, 0.0f, th, th };

		juce::Path sortArrow;

		sortArrow.startNewSubPath ( 0.0f, 0.0f );
		sortArrow.lineTo ( 0.5f, ( columnFlags & juce::TableHeaderComponent::sortedForwards ) ? -0.5f : 0.5f );
		sortArrow.lineTo ( 1.0f, 0.0f );

		g.strokePath ( sortArrow, juce::PathStrokeType ( 1.0f ), sortArrow.getTransformToScaleToFit ( area.removeFromRight ( th / 2.0f ).reduced ( 1.0f ), true ) );
	}

	if ( icon.isEmpty () )
	{
		const int	just = props.getWithDefault ( "colJust" + juce::String ( columnId ), int ( juce::Justification::Flags::centredLeft ) );
		g.setFont ( fnt );
		g.drawText ( colName, juce::Rectangle<float> { xOff, 0.0f, tw, th }, just, true);
	}
	else
	{
		const float	yOff = props.getWithDefault ( "colOffY" + juce::String ( columnId ), 3.0f );

		g.fillPath ( UI::getScaledPath ( icon, juce::Rectangle<float> { xOff, yOff, 14.0f, 14.0f }, juce::RectanglePlacement::centred ) );
	}
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::preparePopupMenuWindow ( juce::Component& newWindow )
{
	newWindow.setOpaque ( false );
	new GUI_DesktopDropshadow ( newWindow );
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawPopupMenuBackground ( juce::Graphics& g, int width, int height )
{
	constexpr auto	radius = 8.0f;

	const auto	rc = juce::Rectangle<float> { float ( width ), float ( height ) };
	const auto	colour = findColour ( juce::PopupMenu::backgroundColourId );

	g.setColour ( colour );
	drawOutlinedRect ( g, rc, radius, 1.0f, colour.darker () );
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawPopupMenuItem ( juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText, const juce::Drawable* icon, const juce::Colour* obs_textColour )
{
	const auto	destructive = obs_textColour;

	const auto	bckCol = findColour ( juce::PopupMenu::backgroundColourId );
	const auto	txtCol = findColour ( juce::PopupMenu::textColourId );
	const auto	bckHighCol = findColour ( destructive ? UI::colors::statusError : int ( juce::PopupMenu::highlightedBackgroundColourId ) );
	const auto	txtHighCol = findColour ( destructive ? UI::colors::text : int ( juce::PopupMenu::highlightedTextColourId ) );

	if ( isSeparator )
	{
		//
		// Separator
		//
		auto	r = area.toFloat ().withSizeKeepingCentre ( area.getWidth () - 20.0f, 1.0f );

		g.setColour ( txtCol.interpolatedWith ( bckCol, 0.9f ) );
		g.fillRect ( r );
	}
	else
	{
		auto r = area.toFloat ().reduced ( 2.5f, 0.0f );
		auto backRect = r;

		auto font = getPopupMenuFont ();
		auto maxFontHeight = r.getHeight () / 1.3f;

		if ( font.getHeight () > maxFontHeight )
			font.setHeight ( maxFontHeight );

		auto	forCol = txtCol;
		auto	bakCol = bckCol;

		if ( isHighlighted && isActive )
		{
			forCol = txtHighCol;
			bakCol = bckHighCol;

			g.setColour ( bakCol );
			g.fillRoundedRectangle ( backRect.reduced ( maxFontHeight / 4.0f, 0.5f ), 5.0f );
		}

		g.setColour ( forCol.interpolatedWith ( bakCol, isActive ? 0.0f : 0.75f ) );

		r.reduce ( std::min ( 5.0f, float ( area.getWidth () ) / 20.0f ), 0.0f );

		g.setFont ( font );

		auto iconArea = r.removeFromLeft ( maxFontHeight );

		if ( icon )
		{
			const auto	iconSize = font.getAscent () * 0.8f;

			const auto	iconOpacity = ( isHighlighted && destructive ) ? 2.5f : 1.0f;

			icon->drawWithin ( g, iconArea.withSizeKeepingCentre ( iconSize, iconSize ), juce::RectanglePlacement::centred, ( isActive ? 0.4f : 0.2f ) * iconOpacity );
		}
		else if ( isTicked )
		{
			if ( hasSubMenu )
			{
				g.fillEllipse ( iconArea.withSizeKeepingCentre ( iconArea.getWidth () * 0.45f, iconArea.getWidth () * 0.45f ) );
			}
			else
			{
				auto&	p = UI::getScaledPath ( "check-solid-full", iconArea, 0, 0.32f );
				g.fillPath ( p );
			}
		}

		if ( hasSubMenu )
		{
			auto&	p = UI::getScaledPath ( "chevron-right-solid-full", r.removeFromRight ( r.getHeight () ), 0, 0.32f );
			g.fillPath ( p );
		}

		r.removeFromRight ( 3.0f );
		g.drawText ( text, r, juce::Justification::centredLeft, false );

		if ( shortcutKeyText.isNotEmpty () )
		{
			auto f2 = font;
			f2.setHeight ( f2.getHeight () * 0.75f );
			f2.setHorizontalScale ( 0.95f );
			g.setFont ( f2 );

			g.drawText ( shortcutKeyText, r, juce::Justification::centredRight, true );
		}
	}
}
//-------------------------------------------------------------------------------------------------

int SID_LookAndFeel::getPopupMenuBorderSizeWithOptions ( const juce::PopupMenu::Options& )
{
	return 7;
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::getIdealPopupMenuItemSize ( const juce::String& text, bool isSeparator, int standardMenuItemHeight, int& idealWidth, int& idealHeight )
{
	if ( isSeparator )
	{
		idealWidth = 50;
		idealHeight = 15;
		return;
	}

	juce::LookAndFeel_V4::getIdealPopupMenuItemSize ( text, isSeparator, standardMenuItemHeight, idealWidth, idealHeight );

	idealWidth += juce::roundToInt ( getPopupMenuFont ().getHeight () * 2.2f );
	idealHeight = juce::roundToInt ( getPopupMenuFont ().getHeight () * 2.5f );
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawPopupMenuSectionHeader ( juce::Graphics& g, const juce::Rectangle<int>& area, const juce::String& sectionName )
{
	const auto	txtCol = findColour ( juce::PopupMenu::backgroundColourId );
	const auto	bckCol = findColour ( juce::PopupMenu::textColourId ).interpolatedWith ( txtCol, 0.5f );

	const auto	r = area.toFloat ().reduced ( 10.0f, 6.0f );

	g.setColour ( bckCol );
	g.fillRoundedRectangle ( r, 1.5f );

	g.setFont ( fontWithHeight ( r.getHeight () / 1.6f ) );
	g.setColour ( txtCol );
	g.drawText ( sectionName, r.reduced ( 10.0f, 0.0f), juce::Justification::centredLeft, true );
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawPopupMenuColumnSeparatorWithOptions ( juce::Graphics& g, const juce::Rectangle<int>& area, const juce::PopupMenu::Options& )
{
	const auto	bckCol = findColour ( juce::PopupMenu::backgroundColourId );
	const auto	txtCol = findColour ( juce::PopupMenu::textColourId );

	const	auto	margin = ( float )area.getWidth () / 2.0f;

	auto r = area.toFloat ().reduced ( margin - 0.75f, margin * 2.0f );

	g.setColour ( txtCol.interpolatedWith ( bckCol, 0.9f ) );
	g.fillRect ( r );
}
//-------------------------------------------------------------------------------------------------

int SID_LookAndFeel::getPopupMenuColumnSeparatorWidthWithOptions ( const juce::PopupMenu::Options& )
{
	return 4;
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawBubble ( juce::Graphics& g, juce::BubbleComponent& comp, const juce::Point<float>& tip, const juce::Rectangle<float>& _body )
{
	const auto	body = _body.reduced ( 0.5f );
	const auto	w = body.getWidth ();
	const auto	c = w / 2.0f;
	const auto	h = body.getHeight ();

	constexpr	auto	corner = 3.0f;

	if ( const auto tipSizeAbove = float ( _body.getY () - tip.getY () ) / 1.5f; tipSizeAbove > 0.1f )
	{
		juce::Path	p;

		p.startNewSubPath ( { corner, 0.0f } );
		p.lineTo ( { c - tipSizeAbove, 0.0f } );
		p.lineTo ( { c, -tipSizeAbove } );
		p.lineTo ( { c + tipSizeAbove, 0.0f } );
		p.lineTo ( { w - corner, 0.0f } );
		p.quadraticTo ( { w, 0.0f }, { w, corner } );
		p.lineTo ( { w, h - corner } );
		p.quadraticTo ( { w, h }, { w - corner, h } );
		p.lineTo ( { corner, h } );
		p.quadraticTo ( { 0.0f, h }, { 0.0f, h - corner } );
		p.lineTo ( { 0.0f, corner } );
		p.quadraticTo ( { 0.0f, 0.0f }, { corner, 0.0f } );
		p.closeSubPath ();

		p.applyTransform ( juce::AffineTransform::translation ( body.getTopLeft () ) );

		g.setColour ( comp.findColour ( juce::BubbleComponent::backgroundColourId ) );
		g.fillPath ( p );

		g.setColour ( comp.findColour ( juce::BubbleComponent::outlineColourId ) );
		g.strokePath ( p, juce::PathStrokeType ( 1.0f ) );
	}
	else if ( const auto tipSizeBelow = float ( tip.getY () - _body.getBottom () ) / 1.5f; tipSizeBelow > 0.1f )
	{
		juce::Path	p;

		p.startNewSubPath ( { corner, 0.0f } );
		p.lineTo ( { w - corner, 0.0f } );
		p.quadraticTo ( { w, 0.0f }, { w, corner } );
		p.lineTo ( { w, h - corner } );
		p.quadraticTo ( { w, h }, { w - corner, h } );
		p.lineTo ( { c + tipSizeBelow, h } );
		p.lineTo ( { c, h + tipSizeBelow } );
		p.lineTo ( { c - tipSizeBelow, h } );
		p.lineTo ( { corner, h } );
		p.quadraticTo ( { 0.0f, h }, { 0.0f, h - corner } );
		p.lineTo ( { 0.0f, corner } );
		p.quadraticTo ( { 0.0f, 0.0f }, { corner, 0.0f } );
		p.closeSubPath ();

		p.applyTransform ( juce::AffineTransform::translation ( body.getTopLeft () ) );

		g.setColour ( comp.findColour ( juce::BubbleComponent::backgroundColourId ) );
		g.fillPath ( p );

		g.setColour ( comp.findColour ( juce::BubbleComponent::outlineColourId ) );
		g.strokePath ( p, juce::PathStrokeType ( 1.0f ) );
	}
	else
	{
		LookAndFeel_V4::drawBubble ( g, comp, tip, _body );
	}
}
//-------------------------------------------------------------------------------------------------

int SID_LookAndFeel::getSliderPopupPlacement ( juce::Slider& /*slider*/ )
{
	return juce::BubbleComponent::below;
}
//----------------------------------------------------------------------------------

juce::Label* SID_LookAndFeel::createSliderTextBox ( juce::Slider& slider )
{
	auto	l = juce::LookAndFeel_V4::createSliderTextBox ( slider );

	l->setFont ( fontWithHeight ( l->getFont ().getHeight () ) );

	return l;
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawLinearSlider ( juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider )
{
	if ( slider.isBar () || style != juce::Slider::SliderStyle::LinearHorizontal )
	{
		juce::LookAndFeel_V4::drawLinearSlider ( g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider );
	}
	else
	{
		const auto	hover = slider.isMouseOverOrDragging ();

		auto&	prop = slider.getProperties ();

		if ( prop.contains ( "choices" ) )
		{
			const int	numChoices = prop[ "choices" ];

			auto	b = slider.getLocalBounds ().toFloat ();
			const auto	choice = int ( slider.getValue () );

			const int	highlightChoice = prop.getWithDefault ( "highlight", -1 );

			g.setFont ( UI::font ( 13.0f, 600 ) );

			// Background
			g.setColour ( UI::getShade ( 0.0f ) );
			g.fillRoundedRectangle ( b, b.getHeight () / 2.0f );

			const auto	choiceWidth = b.getWidth () / numChoices;

			// Render choice texts
			for ( auto i = 0; i < numChoices; ++i )
			{
				const auto	r = b.withX ( i * choiceWidth ).withWidth ( choiceWidth );
				if ( i == choice || i == highlightChoice )
				{
					// Chosen background
					g.setColour ( i == choice ? findColour ( UI::colors::accent ) : UI::getShade ( 0.2f ) );
					g.fillRoundedRectangle ( r.reduced ( 3.0f ), ( r.getHeight () - 6.0f ) / 2.0f );
				}

				g.setColour ( findColour ( UI::colors::text ) );
				g.drawText ( prop[ "choice" + juce::String ( i ) ], r, juce::Justification::centred, false );
			}
		}
		else
		{
			constexpr auto	trackHeight = 4.0f;

			juce::Point<float>	startPoint ( float ( x ), y + height * 0.5f );

			const auto	sliderCol = findColour ( UI::colors::text );

			// Background track
			{
				juce::Point<float>	endPoint ( float ( width + x ), startPoint.y );

				juce::Path	backgroundTrack;

				backgroundTrack.startNewSubPath ( startPoint );
				backgroundTrack.lineTo ( endPoint );
				g.setColour ( UI::getShade ( 0.3f ) );
				g.strokePath ( backgroundTrack, { trackHeight, juce::PathStrokeType::curved, juce::PathStrokeType::rounded } );

				// Slider with bi-polar range, draw tick at the 0 position
				if ( slider.getMinimum () < 0.0 )
				{
					juce::Path	middleLine;
					middleLine.startNewSubPath ( float ( width / 2 + x ), 0.0f );
					middleLine.lineTo ( float ( width / 2 + x ), startPoint.y - trackHeight );

					middleLine.startNewSubPath ( float ( width / 2 + x ), startPoint.y + trackHeight );
					middleLine.lineTo ( float ( width / 2 + x ), float ( height ) );

					g.strokePath ( middleLine, { trackHeight / 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded } );
				}
			}

			juce::Point<float>	maxPoint { sliderPos, ( y + height ) * 0.5f };

			// Value track
			{
				juce::Path valueTrack;

				valueTrack.startNewSubPath ( startPoint );
				valueTrack.lineTo ( maxPoint );

				g.setColour ( hover ? findColour ( UI::colors::accent ) : sliderCol );
				g.strokePath ( valueTrack, { trackHeight, juce::PathStrokeType::curved, juce::PathStrokeType::rounded } );
			}

			// Thumb
			if ( slider.isMouseOverOrDragging () )
			{
				const auto	thumbHeight = float ( getSliderThumbRadius ( slider ) );

				g.setColour ( sliderCol );
				g.fillRoundedRectangle ( juce::Rectangle<float> ( 3.0f, thumbHeight ).withCentre ( maxPoint ), 1.5f );
			}
		}
	}
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawRasterBars ( juce::Graphics& g, juce::Rectangle<float> b )
{
	static juce::Random	rand;
	static colodore	colo;
	static auto	c64Palette = colo.generateRGB ( 0, colo.generateYUV ( VIC2_Render::settings::colorStandard::PAL, 55.0f, 85.0f, 55.0f ) );

	static auto	colIdx = 0;
	auto	y = b.getY ();
	do
	{
		auto	h = juce::jmap ( rand.nextFloat (), 15.0f, 20.0f );
		if ( rand.nextFloat () < 0.05f )
			h *= 1.5f;

		g.setColour ( juce::Colour ( c64Palette[ colIdx ] ) );
		g.fillRect ( b.withY ( y ).withHeight ( h ) );

		// Do a break somewhere
		const auto	w = rand.nextFloat () * b.getWidth ();
		g.fillRect ( b.withY ( y ).withHeight ( 2.0f ).translated ( w, -1.5f ).withWidth ( b.getWidth () - w ) );

		y += h;

		colIdx = ( colIdx + 1 ) & 15;

	} while ( y < b.getBottom () );
}
//-------------------------------------------------------------------------------------------------

void SID_LookAndFeel::drawOutlinedRect ( juce::Graphics& g, const juce::Rectangle<float>& rect, const float radius, const float outline, const juce::Colour outlineCol )
{
	g.fillRoundedRectangle ( rect, radius );

	g.setColour ( outlineCol );
	g.drawRoundedRectangle ( rect.reduced ( outline / 2.0f ), radius - outline / 2.0f, outline );
}
//-------------------------------------------------------------------------------------------------
