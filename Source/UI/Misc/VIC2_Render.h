#pragma once

#include <JuceHeader.h>

#include "colodore.h"
#include "Globals/constants.h"

//-----------------------------------------------------------------------------

class VIC2_Render_Data final
{
public:
	VIC2_Render_Data ();

	uint8_t		chargen[ 4096 ];
	uint64_t	bitsToBytes[ 256 ];
};
//-----------------------------------------------------------------------------

class VIC2_Render final
{
public:
	VIC2_Render ( const bool withBackup );

	// Load image from file (*.png, *.gif)
	bool loadImage ( const char* filename );

	// Image is pure PETSCII, 40x25 characters, plus color buffer, border and screen colors and a control-byte (upper- or lower-case)
	bool loadPETSCII ( const char* filename );

	// Convert screen- and color-buffers, etc. to an index buffer
	void renderScreen ();

	struct settings
	{
		enum colorStandard : int8_t { PAL, NTSC };
		colorStandard	standard = PAL;

		bool	firstLuma = false;

		// All three values between 0.0 and 100.0
		float	brightness = 50.0f;
		float	contrast = 100.0f;
		float	saturation = 50.0f;

		bool	raw = false;

		[[ nodiscard ]] bool needsNewPalette ( const settings& other ) const
		{
			return firstLuma != other.firstLuma || raw != other.raw;
		}
	};

	// Image gets generated from text
	void generateTextCRT ( const uint8_t bckColors, uint8_t textColor, const char* text );

	// Create images for CRT-emulation and thumbnail
	void setSettings ( const settings& set );
	void renderCRT ();

	[[ nodiscard ]] juce::Image& getCRT () { return indexBuffer; }
	[[ nodiscard ]] juce::Image getThumbnail ();
	[[ nodiscard ]] bool wasBorderFilled () const;

	void restoreIndexBuffer ();

	// C64 image size (without borders)
	static constexpr auto	innerUnscaledWidth = 320;
	static constexpr auto	innerUnscaledHeight = 200;

	// C64 border size
	static constexpr auto	unscaledBorderSizeX = 32;
	static constexpr auto	unscaledBorderSizeY = 36;

	static constexpr auto	outerUnscaledWidth = innerUnscaledWidth + unscaledBorderSizeX * 2;
	static constexpr auto	outerUnscaledHeight = innerUnscaledHeight + unscaledBorderSizeY * 2;

	static constexpr auto	outerUnscaledLength = outerUnscaledWidth * outerUnscaledHeight;

	uint8_t		screenBuffer[ 40 * 25 ];
	uint8_t		colorBuffer[ 40 * 25 ];
	uint8_t		screenCol = vic2::black;
	uint8_t		borderCol = vic2::black;
	uint8_t		controlByte = 0x15;

private:
	// Image has to be 32-bit per pixel, alpha is ignored
	bool convertTrueColor ( const char* filename, const uint32_t* rawData, const int width, const int height );

	juce::SharedResourcePointer<VIC2_Render_Data>	characterData;

	void findBorderColor ( const char* filename );
	void fillBorder ();
	void fillAll ( const uint8_t innerCol );

	colodore				colo;
	colodore::yuvPalette	lineYUV[ 2 ];

	void generateLineYUV ();
	void convertToYUVBuffers ();
	void chromaProcessing ();
	void convertToRGB ();

	settings	set;

	int						indexBufferWidth = 0;
	juce::Image				indexBuffer;
	juce::Image				yuvBuffer;
	juce::Image				rgbBuffer;

	void backupIndexBuffer ();
	std::vector<uint8_t>	indexBufferBackup;

	bool		borderInFilename = false;
};
//-----------------------------------------------------------------------------
