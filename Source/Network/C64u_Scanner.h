#pragma once

#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class C64uScanner : private juce::Thread
{
public:
	using ScannerCallback = std::function<void ( const juce::StringArray& )>;

	C64uScanner ();
	~C64uScanner () override;

	void scan ( ScannerCallback _callback );

private:
	void run () override;

	static juce::String isActualC64u ( juce::StreamingSocket& socket );

	ScannerCallback			callback;

	juce::CriticalSection	resultsLock;
	juce::StringArray		foundDevices;
};
//-----------------------------------------------------------------------------
