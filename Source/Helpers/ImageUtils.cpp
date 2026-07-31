#include <JuceHeader.h>

#include "ImageUtils.h"

//-----------------------------------------------------------------------------

// The border-color hint in a screenshot filename is a single hex digit
constexpr auto	hexDigits = "0123456789ABCDEF";
//-----------------------------------------------------------------------------

imageutils::imageHint imageutils::hintFromFilename ( const juce::String& in )
{
	if ( in.isEmpty () )
		return {};

	// name[#hints].ext - only the last '#' and '.' separate, dots and hashes
	// inside the name stay part of it
	jassert ( in.containsChar ( '.' ) );

	const auto	stem = in.upToLastOccurrenceOf ( ".", false, false );

	imageHint	hint;

	hint.name = stem.upToLastOccurrenceOf ( "#", false, false );
	hint.extension = "." + in.fromLastOccurrenceOf ( ".", false, false );

	auto	hintStr = stem.containsChar ( '#' ) ? stem.fromLastOccurrenceOf ( "#", false, false ).toUpperCase () : juce::String ();

	hint.firstLuma = hintStr.containsChar ( 'Y' );
	hint.isGameScreen = hintStr.containsChar ( 'G' );

	hintStr = hintStr.removeCharacters ( "YG" );

	// Last parameter left over is always border color
	if ( hintStr.isNotEmpty () )
		hint.borderColor = int8_t ( juce::String ( hexDigits ).indexOfChar ( hintStr[ 0 ] ) );

	return hint;
}
//-----------------------------------------------------------------------------

juce::String imageutils::filenameFromHint ( const imageutils::imageHint& hint )
{
	if ( hint.name.isEmpty () )
		return {};

	auto	hintStr = juce::String ();

	if ( hint.isGameScreen )		hintStr += "G";
	if ( hint.firstLuma )			hintStr += "Y";
	if ( hint.borderColor >= 0 && hint.borderColor < 16 )
		hintStr += hexDigits[ hint.borderColor ];

	if ( hintStr.isNotEmpty () )
		hintStr = "#" + hintStr;

	return hint.name + hintStr + hint.extension;
}
//-----------------------------------------------------------------------------

bool imageutils::imagesAreEqual ( const juce::Image& a, const juce::Image& b )
{
	if ( a.getWidth () != b.getWidth () || a.getHeight () != b.getHeight () || a.getFormat () != b.getFormat () )
		return false;

	const auto	dataA = juce::Image::BitmapData ( a, juce::Image::BitmapData::readOnly );
	const auto	dataB = juce::Image::BitmapData ( b, juce::Image::BitmapData::readOnly );

	const auto	h = a.getHeight ();
	const auto rowBytes = dataA.width * dataA.pixelStride;

	for ( auto y = 0; y < h; ++y )
		if ( std::memcmp ( dataA.getLinePointer ( y ), dataB.getLinePointer ( y ), rowBytes ) != 0 )
			return false;

	return true;
}
//-----------------------------------------------------------------------------

bool imageutils::imagesAreNotEqual ( const juce::Image& a, const juce::Image& b )
{
	return ! imagesAreEqual ( a, b );
}
//-----------------------------------------------------------------------------
