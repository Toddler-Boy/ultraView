#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class C64uScanner : private juce::Thread
{
public:
	using ScannerCallback = std::function<void ( const juce::String& )>;

	C64uScanner ();
	~C64uScanner () override;

	void scan ( ScannerCallback _callback, juce::String& _lastIP );

private:
	void run () override;

	juce::String isActualC64u ( juce::StreamingSocket& socket );

	ScannerCallback	callback;
	juce::String	lastIP;

	juce::String	request;
};
//-----------------------------------------------------------------------------
