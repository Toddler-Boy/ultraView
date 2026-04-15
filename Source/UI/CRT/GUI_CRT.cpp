#include <JuceHeader.h>

#include "GUI_CRT.h"

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
							paths::getDataRoot ( "UI/layouts/pages/crt.json" ) } );

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

void GUI_CRT::update ( const float secondsPassed )
{
	if ( ! isShowing () )
		return;

	// Update OpenGL iFrame & iTime
	overlay.setFrameAndTime ( 0, float ( juce::Time::highResolutionTicksToSeconds ( juce::Time::getHighResolutionTicks () ) ) );

/*	// This gets called once per V-BLANK (so may be higher than refresh rate of the C64)
	constexpr auto	frameMS = 1.0f / 60.0f - 0.01f;

	// Handle updates (skip if it happened faster than 65 Hz)
	timePassed += secondsPassed;
	if ( timePassed < frameMS )
		return;

	timePassed = 0.0f;
*/
}
//-----------------------------------------------------------------------------

void GUI_CRT::showSettings ( const bool visible )
{
	settingsWrapper.setVisible ( visible );
	settingsVisible = visible;

	overlay.openSettings.setStage ( settingsVisible ? 1 : 0	);

	resized ();
}
//-----------------------------------------------------------------------------

void GUI_CRT::setBackgroundColour ( const juce::Colour& bckCol )
{
	overlay.setBackgroundColor ( bckCol );
}
//-----------------------------------------------------------------------------

void GUI_CRT::renderCRT ()
{
	auto	vic2Settings = getVIC2SettingsFromPreferences ();
	auto	settings = overlay.getSettings ();
	settings.isNTSC = vic2Settings.standard == VIC2_Render::settings::NTSC;

	updateCRTPalette ( vic2Settings );

	overlay.setSettings ( settings );

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
			intChoiceSystem = tvSystemChoices.indexOf ( "PAL", true );

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
