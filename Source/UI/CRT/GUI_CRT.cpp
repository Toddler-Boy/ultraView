#include <JuceHeader.h>

#include "GUI_CRT.h"

#include "libSidplayEZ/src/stringutils.h"

#include "UI/Components/GUI_ComboBox.h"
#include "UI/Components/GUI_CRTSliderIcon.h"
#include "UI/Components/GUI_CRTSliderLabel.h"
#include "UI/Components/GUI_Disabler.h"
#include "UI/Components/GUI_Label.h"
#include "UI/Components/GUI_Line.h"
#include "UI/Components/GUI_SettingsBox.h"
#include "UI/Components/GUI_Slider.h"
#include "UI/Components/GUI_VIC2_Palette.h"
#include "UI/Components/GUI_XYPad.h"

//-----------------------------------------------------------------------------

GUI_CRT::GUI_CRT ()
{
	setName ( "crt" );
	addAndMakeVisible ( overlay );

	// Open/close settings
	{
		overlay.openSettings.onClick = [ this ]
		{
			showSettings ( overlay.openSettings.getStage () );
		};
	}

	settingsViewport.setScrollBarsShown ( true, false );
	settingsViewport.setViewedComponent ( &settingsContent, false );

	settingsWrapper.addAndMakeVisible ( settingsViewport );
	addChildComponent ( settingsWrapper );

	addMouseListener ( this, true );
}
//-----------------------------------------------------------------------------

void GUI_CRT::timerCallback ( int timerID )
{
	switch ( timerID )
	{
		case 'TV  ':
			stopTimer ( timerID );

			updateOverlayCRTSettings ();
			updateCRTsettingsUI ();
			break;

		case 'WCAM':
			stopTimer ( timerID );

			updateOverlayCRTSettings ();
			break;

		case 'CURS':
			if ( ! isBasicScreen )
			{
				stopTimer ( timerID );
				return;
			}
			cursorVisible = ! cursorVisible;
			renderCRT ( lastWasGenerated );
			break;
	}
}
//-----------------------------------------------------------------------------

void GUI_CRT::resized ()
{
	const auto	kioskMode = dynamic_cast<juce::DocumentWindow*> ( getTopLevelComponent () )->isKioskMode ();

	crtLayout.setConstant ( "fullscreen", kioskMode ? 1 : 0 );
	crtLayout.setConstant ( "windowed", kioskMode ? 0 : 1 );
	crtLayout.setConstant ( "showSettings", settingsVisible ? 1 : 0 );

	const auto	pos = settingsViewport.getViewPosition ();

	crtLayout.setLayout ( {	paths::getDataRoot ( "UI/layouts/constants.json" ),
							paths::getDataRoot ( "UI/layouts/crt.json" ) } );

	settingsViewport.setViewPosition ( pos );

	//
	// CRT settings
	//
	if ( crtSettingsComponentMap.empty () )
	{
		helpers::buildComponentMap ( crtSettingsComponentMap, &settingsContent );
		connectComponents ();
	}
}
//-----------------------------------------------------------------------------

void GUI_CRT::mouseWheelMove ( const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel )
{
	if ( event.eventComponent != &overlay )
		return;

	const auto	delta = wheel.deltaY * ( event.mods.isShiftDown () ? 20.0f : 50.0f );

	if ( event.mods.isCommandDown () )
	{
		auto	overscan = helpers::findComponent<juce::Slider> ( "tv/overscan/slider", crtSettingsComponentMap );

		const auto	newOverscan = std::clamp ( float ( overscan->getValue () ) + delta, 0.0f, 100.0f );
		overscan->setValue ( newOverscan, juce::dontSendNotification );
		preferences->set ( "TV", "overscan", newOverscan );
	}
	else
	{
		if ( ! preferences->get<bool> ( "Overlay", "enabled" ) )
			return;

 		auto	zoom = helpers::findComponent<juce::Slider> ( "overlay/disabler/zoom/slider", crtSettingsComponentMap );

 		const auto	newZoom = std::clamp ( zoom->getValue () + delta, 0.0, 100.0 );
 		zoom->setValue ( newZoom, juce::dontSendNotification );
 		preferences->set ( "Overlay", "zoom", int ( newZoom ) );
	}

	updateOverlayCRTSettings ();
	overlay.updateZoom ();
}
//-----------------------------------------------------------------------------

void GUI_CRT::visibilityChanged ()
{
	if ( isVisible () && isBasicScreen )
		startTimer ( 'CURS', 333 );
	else
		stopTimer ( 'CURS' );
}
//-----------------------------------------------------------------------------

void GUI_CRT::setRoot ( const juce::File& _root )
{
	screenshotRoot = _root.getChildFile ( "Screenshots/" );
}
//-----------------------------------------------------------------------------

void GUI_CRT::setStrings ( const SidTuneInfoEZ& src )
{
	sidInfoStr = src;
}
//-----------------------------------------------------------------------------

void GUI_CRT::loadGameArtwork ( const juce::String& sidName, const juce::String& index /* ="" */ )
{
	std::tie ( lastLoaded, tuneArtIndex ) = findArtwork ( sidName, index );

	lastFirstLuma = helpers::hintFromFilename ( lastLoaded.getFullPathName () ).firstLuma;

	sidname = sidName.fromLastOccurrenceOf ( "/", false, false );

	if ( vicRender.loadImage ( lastLoaded.getFullPathName ().toRawUTF8 () ) )
		renderCRT ();
	else
		renderCRT ( true );
}
//-----------------------------------------------------------------------------

void GUI_CRT::loadGameArtwork ( const int index )
{
	if ( index < 0 || index >= int ( tuneArtwork.size () ) )
	{
		lastLoaded = juce::File ();
		renderCRT ( true );
		return;
	}

	tuneArtIndex = index;
	lastLoaded = screenshotRoot.getChildFile ( tuneArtwork[ tuneArtIndex ] );

	lastFirstLuma = helpers::hintFromFilename ( lastLoaded.getFullPathName () ).firstLuma;

	if ( vicRender.loadImage ( lastLoaded.getFullPathName ().toRawUTF8 () ) )
		renderCRT ();
	else
		renderCRT ( true );
}
//-----------------------------------------------------------------------------

void GUI_CRT::showSettings ( const bool visible )
{
	settingsWrapper.setVisible ( visible );
	settingsVisible = visible;
	resized ();
}
//-----------------------------------------------------------------------------

void GUI_CRT::setBackgroundColour ( const juce::Colour& bckCol )
{
	overlay.setBackgroundColor ( bckCol );
}
//-----------------------------------------------------------------------------

void GUI_CRT::timerUpdate ( const float secondsPassed, const uint16_t cpuCycles )
{
	if ( ! isShowing () )
		return;

	// Update OpenGL iFrame & iTime
	overlay.setFrameAndTime ( 0, float ( juce::Time::highResolutionTicksToSeconds ( juce::Time::getHighResolutionTicks () ) ) );

	// This gets called once per V-BLANK (so may be higher than refresh rate of the C64)
	constexpr auto	frameMS = 1.0f / 60.0f - 0.01f;

	// Handle updates (skip if it happened faster than 65 Hz)
	timePassed += secondsPassed;
	if ( timePassed < frameMS )
		return;

	timePassed = 0.0f;

	// Draw raster lines
	if ( cpuCycles < 50 || cpuCycles > 10'000 )
		return;

	vicRender.restoreIndexBuffer ();

	auto&	img = vicRender.getCRT ();
	auto	dst = (uint8_t*)juce::Image::BitmapData ( img, juce::Image::BitmapData::ReadWriteMode::writeOnly ).data;

	// Draw raster lines
	auto fillLine = [ &dst ] ( int y, int width, const uint8_t color )
	{
		width = std::clamp ( width, 0, VIC2_Render::outerUnscaledWidth );
		if ( width <= 0 )
			return;

		y = y % VIC2_Render::outerUnscaledHeight;
		y = y + ( ( y >> 31 ) & VIC2_Render::outerUnscaledHeight );

		const auto	d = dst + y * VIC2_Render::outerUnscaledWidth;

		// In border?
		if ( y < VIC2_Render::unscaledBorderSizeY || y > ( VIC2_Render::unscaledBorderSizeY + VIC2_Render::innerUnscaledHeight ) )
		{
			std::fill_n ( d, width, color );
		}
		else
		{
			std::fill_n ( d, std::min ( width, VIC2_Render::unscaledBorderSizeX ), color );

			width -= VIC2_Render::unscaledBorderSizeX + VIC2_Render::innerUnscaledWidth;
			if ( width > 0 )
				std::fill_n ( d + VIC2_Render::unscaledBorderSizeX + VIC2_Render::innerUnscaledWidth, width, color );
		}
	};

	// Convert cycles to rasterlines
	const auto	crtSet = overlay.getSettings ();
	const auto	cyclesPerLine = crtSet.isNTSC ? 65 : 63;
	const auto	lineWidth = cyclesPerLine * 8;
	const auto	fullLines = cpuCycles / cyclesPerLine;
	const auto	remainder = ( cpuCycles - ( fullLines * cyclesPerLine ) ) * 8;

	// Draw raster lines
	for ( auto i = 0; i < int ( fullLines ); ++i )
		fillLine ( 100 + i, lineWidth, 13 );
	fillLine ( 100 + fullLines, remainder - ( lineWidth - VIC2_Render::outerUnscaledWidth ), 13 );

	overlay.triggerIndexTextureUpdate ();
}
//-----------------------------------------------------------------------------

void GUI_CRT::renderCRT ( const bool generate )
{
	lastWasGenerated = generate;

	auto	vic2Settings = getVIC2SettingsFromPreferences ();
	vicRender.setSettings ( vic2Settings );

	isBasicScreen = generate && sidname.isEmpty ();

	if ( generate )
	{
		static const char* resetText = "\n    **** COMMODORE 64 BASIC V2 ****\n\n 64K RAM SYSTEM  38911 BASIC BYTES FREE\n\nREADY.\n";

		char	sidText[ 40 * 25 * 2 + 1 ];		// double space needed for control characters

		if ( sidname.isNotEmpty () )
		{
			auto	sidUpper = sidname.replaceCharacter ( '_', '-' );
			auto	sid = sidUpper.toRawUTF8 ();
			auto	title = stringutils::utf8toExtendedASCII ( sidInfoStr.title );
			auto	author = stringutils::utf8toExtendedASCII ( sidInfoStr.author );
			auto	released = stringutils::utf8toExtendedASCII ( sidInfoStr.released );

			std::snprintf ( sidText, sizeof ( sidText ), "%sLOAD\"%.16s\",8,1\n\nSEARCHING FOR %.16s\nLOADING\nREADY.\nRUN\n\n\1"
						   " -------------------------------------- "
						   " TITLE    : %.27s\n"
						   " AUTHOR   : %.27s\n"
						   " RELEASED : %.27s\n"
						   " -------------------------------------- "
						   , resetText, sid, sid, title.c_str (), author.c_str (), released.c_str () );
		}
		else
		{
			std::strcpy ( sidText, resetText );
			if ( cursorVisible )
				std::strcat ( sidText, "`" );
		}
		vicRender.generateTextCRT ( vic2::light_blue << 4 | vic2::blue, vic2::light_blue, sidText );
	}

	auto	settings = overlay.getSettings ();
	settings.isNTSC = vic2Settings.standard == VIC2_Render::settings::NTSC;

	updateCRTPalette ( vic2Settings );

	overlay.setSettings ( settings );
	overlay.setIndexTextureSource ( vicRender.getCRT () );

	updateCRTsettingsUI ();
}
//-----------------------------------------------------------------------------

VIC2_Render::settings GUI_CRT::getVIC2SettingsFromPreferences () const
{
	auto	renderSettings = VIC2_Render::settings {};

	auto getChoiceInt = [ this ] ( const juce::StringArray& choices, const juce::String& name )
	{
		return std::max ( 0, choices.indexOf ( preferences->get<juce::String> ( name ) ) );
	};

	// TV standard (PAL/NTSC)
	{
		const static juce::StringArray	tvSystemChoices { "AUTO", "PAL", "NTSC" };

		auto	intChoiceSystem = getChoiceInt ( tvSystemChoices, "tv/system" );
		if ( intChoiceSystem == 0 )
			intChoiceSystem = tvSystemChoices.indexOf ( sidInfoStr.clock, true );

		renderSettings.standard = VIC2_Render::settings::colorStandard ( intChoiceSystem - 1 );
	}

	// First or revised Luma
	{
		const static juce::StringArray	firstLumaChoices { "AUTO", "FIRST", "REV" };

		auto	intChoiceLuma = getChoiceInt ( firstLumaChoices, "tv/first-luma" );
		if ( intChoiceLuma == 0 )
			intChoiceLuma = lastFirstLuma ? 1 : 2;

		renderSettings.firstLuma = intChoiceLuma == 1;
	}

	renderSettings.brightness = preferences->get<int> ( "tv/brightness" );
	renderSettings.contrast = preferences->get<int> ( "tv/contrast" );
	renderSettings.saturation = preferences->get<int> ( "tv/saturation"	);

	renderSettings.raw = ! preferences->get<bool> ( "crt/emulation" );

	return renderSettings;
}
//-----------------------------------------------------------------------------

lime::CRTEmulation::settings GUI_CRT::getCRTEmulationSettingsFromPreferences () const
{
	auto	set = lime::CRTEmulation::settings {};

	// Overlay settings
	set.overlay = preferences->get<bool> ( "overlay/enabled" );
	set.overlayProfile = preferences->get<juce::String> ( "overlay/bitmap" );

	set.overlayDaytime = preferences->get<int> ( "overlay/daytime" );
	set.overlayBezel = preferences->get<int> ( "overlay/bezel" );
	set.overlayShadow = preferences->get<int> ( "overlay/shadow" );
	set.overlayZoom = preferences->get<int> ( "overlay/zoom" );

	set.overlayDust = preferences->get<int> ( "overlay/dust" );
	set.overlayChromaticAberration = preferences->get<int> ( "overlay/chromatic-aberration" );
	set.overlayGrain = preferences->get<int> ( "overlay/grain" );

	// TV settings that affect CRT emulation
	set.brightness = preferences->get<int> ( "tv/brightness" );
	set.contrast = preferences->get<int> ( "tv/contrast" );
	set.saturation = preferences->get<int> ( "tv/saturation" );
	set.overscan = preferences->get<int> ( "tv/overscan" );

	// CRT emulation itself
	set.crtEmulation = preferences->get<bool> ( "crt/emulation" );
	set.encJailbars = preferences->get<int> ( "crt/jailbars" );
	set.decSharpening = preferences->get<int> ( "crt/sharpening" );
	set.decLumaBlur = preferences->get<int> ( "crt/luma-blur" );
	set.decChromaBlur = preferences->get<int> ( "crt/chroma-blur" );
	set.decInterference = preferences->get<int> ( "crt/interference" );
	set.decCrosstalk = preferences->get<int> ( "crt/crosstalk" );
	set.decSubcarrier = preferences->get<int> ( "crt/sub-carrier" );
	set.decNoise = preferences->get<int> ( "crt/noise" );

	set.crtCurve = preferences->get<int> ( "crt/curve" );
	set.crtBleed = preferences->get<int> ( "crt/bleed" );
	set.crtBleedRed = preferences->get<iniFile::vec2i> ( "crt/bleed-red" );
	set.crtBleedGreen = preferences->get<iniFile::vec2i> ( "crt/bleed-green" );
	set.crtBleedBlue = preferences->get<iniFile::vec2i> ( "crt/bleed-blue" );
	set.crtHwave = preferences->get<int> ( "crt/h-wave" );
	set.crtScanlines = preferences->get<int> ( "crt/scanlines" );
	set.crtMask = preferences->get<int> ( "crt/mask" );
	set.crtMaskBitmap = preferences->get<juce::String> ( "crt/mask-bitmap" );

	set.crtGlow = preferences->get<int> ( "crt/glow" );
	set.crtPhosphorDecay = preferences->get<int> ( "crt/phosphor-decay" );
	set.crtAmbient = preferences->get<int> ( "crt/ambient" );
	set.crtVignette = preferences->get<int> ( "crt/vignette" );
	set.crtReflections = preferences->get<int> ( "crt/reflection" );

	set.webcam = preferences->get<bool> ( "webcam/enabled" );
	set.webcamBrightness = preferences->get<int> ( "webcam/brightness" );
	set.webcamContrast = preferences->get<int> ( "webcam/contrast" );
	set.webcamSaturation = preferences->get<int> ( "webcam/saturation" );

	return set;
}
//-----------------------------------------------------------------------------

void GUI_CRT::updateOverlayCRTSettings ()
{
	auto	settings = getCRTEmulationSettingsFromPreferences ();

	// VIC2 settings that affect CRT emulation
	{
		const auto	vic2Settings = getVIC2SettingsFromPreferences ();

		settings.isNTSC = vic2Settings.standard == VIC2_Render::settings::NTSC;
		settings.crtEmulation = ! vic2Settings.raw;

		updateCRTPalette ( vic2Settings );
	}

	overlay.setSettings ( settings );
}
//-----------------------------------------------------------------------------

void GUI_CRT::updateCRTPalette ( const VIC2_Render::settings& vic2Settings )
{
	if (	vic2Settings.needsNewPalette ( curVicSettings )
		 ||	yuvE_yuvO_yiq.empty () )
	{
		yuvE_yuvO_yiq = colo.generateYUV_YIQ ( vic2Settings.raw ? 0.0f : 22.5f, vic2Settings.firstLuma );
		overlay.setLumaChromaPalette ( yuvE_yuvO_yiq );

		curVicSettings = vic2Settings;
	}
}
//-----------------------------------------------------------------------------

void GUI_CRT::updateCRTsettingsUI ()
{
	if ( crtSettingsComponentMap.empty () )
		return;

	const auto	vic2Settings = getVIC2SettingsFromPreferences ();

	auto highlightChoice = [ this ] ( const juce::String& compName, const int choice )
	{
		auto	sld = helpers::findComponent<juce::Slider> ( compName, crtSettingsComponentMap );
		if ( sld->getProperties ().set ( "highlight", choice ) )
			sld->repaint ();
	};

	highlightChoice ( "tv/system", vic2Settings.standard + 1 );
	highlightChoice ( "tv/first-luma", vic2Settings.firstLuma ? 1 : 2 );

	// Set palette
	auto	palette = helpers::findComponent<GUI_VIC2_Palette> ( "tv/palette", crtSettingsComponentMap );
	palette->setSettings ( vic2Settings.standard, vic2Settings.brightness, vic2Settings.contrast, vic2Settings.saturation, vic2Settings.firstLuma );
}
//-----------------------------------------------------------------------------

std::pair<juce::File, const int> GUI_CRT::findArtwork ( const juce::String& _sidname, const juce::String& indexStr )
{
	tuneArtwork = scrshot->getScreenshots ( _sidname.toStdString () );

	overlay.setNumCRTpages ( int ( tuneArtwork.size () ) );

	if ( tuneArtwork.empty () )
		return {};

	auto	index = ScreenshotLookup::getDefaultScreenshotIndex ( tuneArtwork );
	if ( indexStr.isNotEmpty () )
	{
		const auto	idxStr = indexStr.toStdString ();
		auto findWithHint = [ &idxStr ] (const std::string& str) -> bool
		{
			if ( idxStr == str )
				return true;

			const auto	hint = helpers::hintFromFilename ( str );
			return idxStr == hint.name + hint.extension;
		};

		auto	it = std::ranges::find_if ( tuneArtwork, findWithHint );
		index = int ( std::distance ( tuneArtwork.begin (), it ) );
	}

	overlay.setCRTPage ( index );

	return { screenshotRoot.getChildFile ( tuneArtwork[ index ] ), index };
}
//-----------------------------------------------------------------------------

void GUI_CRT::updateDisablers ()
{
	auto updateDisabler = [ this ] ( const juce::String& parentName, const juce::String& toggleName )
	{
		auto	disabler = helpers::findComponent<GUI_Disabler> ( parentName + "/disabler", crtSettingsComponentMap );
		auto	toggle = helpers::findComponent<juce::ToggleButton> ( parentName + "/" + toggleName, crtSettingsComponentMap );

		disabler->setEnabled ( toggle->getToggleState () );
	};

	updateDisabler ( "overlay", "enabled" );
	updateDisabler ( "crt", "emulation" );
	updateDisabler ( "crt/disabler", "webcam" );
}
//-----------------------------------------------------------------------------

void GUI_CRT::connectComponents ()
{
	auto sliderConnect = [ this ] ( const juce::String& sldName )
	{
		helpers::findComponent<GUI_CRTSliderLabel> ( sldName, crtSettingsComponentMap )->onValueChange = [ this ]
		{
			updateOverlayCRTSettings ();
		};
	};

	//
	// Overlay bitmap toggle
	//
	auto	bitmapEnabled = helpers::findComponent<GUI_Toggle> ( "overlay/enabled", crtSettingsComponentMap );
	bitmapEnabled->onClick = [ bitmapEnabled, this ]
	{
		preferences->set ( "overlay/enabled", bitmapEnabled->getToggleState () );
		updateDisablers ();
		updateOverlayCRTSettings ();
		overlay.updateOverlay ();
	};

	//
	// Overlay selector
	//
	{
		auto	overlayBrowser = helpers::findComponent<juce::ComboBox> ( "overlay/disabler/bitmap", crtSettingsComponentMap );

		overlayBrowser->onChange = [ this, overlayBrowser ]
		{
			preferences->set ( "overlay/bitmap", overlayBrowser->getText () );
			updateOverlayCRTSettings ();
			overlay.updateOverlay ();
		};

		overlayBrowser->addItemList ( overlay.getOverlays (), 1 );
		overlayBrowser->setText ( preferences->get<juce::String> ( "overlay/bitmap" ), juce::sendNotification );
	}

	sliderConnect ( "overlay/disabler/daytime" );
	sliderConnect ( "overlay/disabler/bezel" );
	sliderConnect ( "overlay/disabler/shadow" );

	//
	// Zoom
	//
	helpers::findComponent<GUI_CRTSliderLabel> ( "overlay/disabler/zoom", crtSettingsComponentMap )->onValueChange = [ this ]
	{
		updateOverlayCRTSettings ();
		overlay.updateZoom ();
	};

	sliderConnect ( "overlay/disabler/dust" );
	sliderConnect ( "overlay/disabler/chromatic" );
	sliderConnect ( "overlay/disabler/grain" );

	//
	// Colodore colors
	//
	{
		auto timedSlider = [ this ]
		{
			if ( ! isTimerRunning ( 'TV  ' ) )
			{
				auto choiceToPreference = [ this ] ( const juce::String& pref )
				{
					auto	sld = helpers::findComponent<juce::Slider> ( pref, crtSettingsComponentMap );
					const juce::String	choice = sld->getProperties ()[ "choice" + juce::String ( int ( sld->getValue () ) ) ];
					preferences->set ( pref, choice );
				};

				auto sliderToPreference = [ this ] ( const juce::String& pref )
				{
					auto	sld = helpers::findComponent<juce::Slider> ( pref + "/slider", crtSettingsComponentMap );
					preferences->set<int> ( pref, int ( sld->getValue () ) );
				};

				// TV settings
				choiceToPreference ( "tv/system" );
				choiceToPreference ( "tv/first-luma" );

				// Brightness/contrast/saturation
				sliderToPreference ( "tv/brightness" );
				sliderToPreference ( "tv/contrast" );
				sliderToPreference ( "tv/saturation" );

				// Start timer to update CRT values
				startTimer ( 'TV  ', 1000 / 100 );
			}
		};

		auto connectSlider = [ this, &timedSlider ] ( const juce::String& compName )
		{
			helpers::findComponent<juce::Slider> ( compName, crtSettingsComponentMap )->onValueChange = timedSlider;
		};

		connectSlider ( "tv/system" );
		connectSlider ( "tv/first-luma" );
		connectSlider ( "tv/brightness/slider" );
		connectSlider ( "tv/contrast/slider" );
		connectSlider ( "tv/saturation/slider" );

		updateCRTsettingsUI ();
	}

	//
	// TV overscan
	//
	{
		helpers::findComponent<GUI_CRTSliderIcon> ( "tv/overscan", crtSettingsComponentMap )->onValueChange = [ this ]
		{
			updateOverlayCRTSettings ();
		};
	}

	//
	// CRT emulation toggle
	//
	{
		auto	emulation = helpers::findComponent<GUI_Toggle> ( "crt/emulation", crtSettingsComponentMap );
		emulation->onClick = [ this, emulation ]
		{
			preferences->set ( "crt/emulation", emulation->getToggleState () );

			updateDisablers ();

			updateOverlayCRTSettings ();
		};
	}

	//
	// CRT emulation parameters
	//
	sliderConnect ( "crt/disabler/jailbars" );
	sliderConnect ( "crt/disabler/sharpening" );
	sliderConnect ( "crt/disabler/luma-blur" );
	sliderConnect ( "crt/disabler/chroma-blur" );
	sliderConnect ( "crt/disabler/interference" );
	sliderConnect ( "crt/disabler/crosstalk" );
	sliderConnect ( "crt/disabler/sub-carrier" );
	sliderConnect ( "crt/disabler/noise" );

	sliderConnect ( "crt/disabler/curve" );
	sliderConnect ( "crt/disabler/bleed" );
	sliderConnect ( "crt/disabler/h-wave" );
	sliderConnect ( "crt/disabler/scanlines" );
	sliderConnect ( "crt/disabler/mask" );
	sliderConnect ( "crt/disabler/glow" );
	sliderConnect ( "crt/disabler/ambient" );
	sliderConnect ( "crt/disabler/phosphor" );
	sliderConnect ( "crt/disabler/vignette" );

	sliderConnect ( "crt/disabler/reflection" );

	// CRT Bleed
	{
		auto updateBleed = [ this ]
		{
			updateOverlayCRTSettings ();
		};

		auto	bleedRed = helpers::findComponent<GUI_XYPad> ( "crt/disabler/bleed-red", crtSettingsComponentMap );
		auto	bleedGreen = helpers::findComponent<GUI_XYPad> ( "crt/disabler/bleed-green", crtSettingsComponentMap );
		auto	bleedBlue = helpers::findComponent<GUI_XYPad> ( "crt/disabler/bleed-blue", crtSettingsComponentMap );

		bleedRed->onValueChange = updateBleed;
		bleedGreen->onValueChange = updateBleed;
		bleedBlue->onValueChange = updateBleed;
	}

	//
	// Reflections
	//
	{
		auto	reflectionSource = helpers::findComponent<GUI_Toggle> ( "crt/disabler/webcam", crtSettingsComponentMap );
		reflectionSource->onClick = [ this, reflectionSource ]
		{
			preferences->set ( "webcam/enabled", reflectionSource->getToggleState () );

			updateDisablers ();
			updateOverlayCRTSettings ();
		};
	}

	//
	// Webcam settings
	//
	{
		auto timedSlider60 = [ this ]
		{
			if ( ! isTimerRunning ( 'WCAM' ) )
				startTimer ( 'WCAM', 1000 / 60 );
		};

		helpers::findComponent<GUI_CRTSliderIcon> ( "crt/disabler/disabler/brightness", crtSettingsComponentMap )->onValueChange = timedSlider60;
		helpers::findComponent<GUI_CRTSliderIcon> ( "crt/disabler/disabler/contrast", crtSettingsComponentMap )->onValueChange = timedSlider60;
		helpers::findComponent<GUI_CRTSliderIcon> ( "crt/disabler/disabler/saturation", crtSettingsComponentMap )->onValueChange = timedSlider60;
	}

	//
	// CRT-Mask-bitmap selector
	//
	{
		auto	crtMaskBrowser = helpers::findComponent<juce::ComboBox> ( "crt/disabler/mask-bitmap", crtSettingsComponentMap );

		crtMaskBrowser->onChange = [ this, crtMaskBrowser ]
		{
			preferences->set ( "crt/mask-bitmap", crtMaskBrowser->getText () );
			overlay.updateOverlay ();
		};

		crtMaskBrowser->addItemList ( overlay.getCRTMasks (), 1 );
		crtMaskBrowser->setText ( preferences->get<juce::String> ( "crt/mask-bitmap" ), juce::sendNotification );
	}

	updateOverlayCRTSettings ();

	//
	// Set initial values
	//
	overlay.updateZoom ();

	updateDisablers ();
}
//-----------------------------------------------------------------------------

bool GUI_CRT::isInterestedInFileDrag ( const juce::StringArray& files )
{
	if ( ! UI::isDeveloperMode () )
		return false;

	if ( helpers::getFilteredStrings ( files, { ".png" } ).size () )
		return true;

	return false;
}
//-----------------------------------------------------------------------------

void GUI_CRT::filesDropped ( const juce::StringArray& files, int /*x*/, int /*y*/ )
{
	if ( ! UI::isDeveloperMode () )
		return;

	const auto	s = helpers::createActionMessage ( "addScreenshots", helpers::getFilteredStrings ( files, { ".png" } ) );

	if ( s.empty () )
		return;

	UI::sendGlobalMessage ( s );
}
//-----------------------------------------------------------------------------

bool GUI_CRT::isInterestedInTextDrag ( const juce::String& text )
{
	const auto	trimmed = text.trim ().toLowerCase ();

	if ( trimmed.startsWith ( "http://" ) || trimmed.startsWith ( "https://" ) )
	{
		const auto	url = juce::URL ( trimmed );
		const auto	domain = url.getDomain ().toLowerCase ();

		const auto	fname = url.getFileName ();
		if ( UI::isDeveloperMode () && fname.endsWithIgnoreCase ( ".png" ) )
			return true;
	}

	return false;
}
//-----------------------------------------------------------------------------

void GUI_CRT::textDropped ( const juce::String& text, int /*x*/, int /*y*/ )
{
	UI::sendGlobalMessage ( "downloadScreenshot \"{}\"", text.trim () );
}
//-----------------------------------------------------------------------------
