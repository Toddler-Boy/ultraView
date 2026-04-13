#pragma once

#include <JuceHeader.h>

class MipMap final
{
public:
	MipMap () = default;
	MipMap ( const juce::Image& src )	{	setImage ( src );	}

	void setImage ( const juce::Image& src );
	void setImage ( const juce::File& f );
	void setImage ( const void* rawData, size_t numBytesOfData );

	void draw ( juce::Graphics& g, juce::Rectangle<float> rc, juce::RectanglePlacement placement = 0 );

	[[ nodiscard ]] juce::Image getImageFor ( const int width, const int height );
	[[ nodiscard ]] juce::Image getImage () { return images[ 0 ]; }

	[[ nodiscard ]] juce::Rectangle<int> getBounds ();

	[[ nodiscard ]] int numMipMaps () const		{	return int ( images.size () );	}
	[[ nodiscard ]] bool isValid () const		{	return numMipMaps ();			}
	[[ nodiscard ]] bool isNull () const		{	return ! isValid ();			}
	[[ nodiscard ]] int getWidth () const		{	return isValid () ? images[ 0 ].getWidth () : 0; }
	[[ nodiscard ]] int getHeight () const		{	return isValid () ? images[ 0 ].getHeight () : 0; }

	[[ nodiscard ]] int getNumBytesOfData () const;

private:
	std::vector<juce::Image>	images;
};
//-------------------------------------------------------------------------------------------------
