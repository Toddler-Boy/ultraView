#pragma once
#include <JuceHeader.h>

//-----------------------------------------------------------------------------

class AsyncNetwork : private juce::Thread
{
public:
	using NetworkCallback = std::function<void ( const juce::var&, const int )>;

	AsyncNetwork ();
	~AsyncNetwork () override;

	void setBaseAddress ( const juce::String& url );

	void get ( const juce::String& endpoint, const juce::StringArray& params = {}, NetworkCallback cb = nullptr );
	void post ( const juce::String& endpoint, const juce::MemoryBlock& data, NetworkCallback cb = nullptr, const juce::StringArray& params = {} );
	void put ( const juce::String& endpoint, const juce::StringArray& params = {}, NetworkCallback cb = nullptr );

private:
	struct RequestData
	{
		juce::URL			url;
		juce::String		method;
		NetworkCallback		callback;
		int					statusCode;
	};

	void run () override;
	void enqueue ( const juce::String& endpoint, const juce::String& method, const juce::MemoryBlock& data, NetworkCallback cb, const juce::StringArray& params );

	juce::URL baseAddress;
	juce::CriticalSection queueLock;
	juce::Array<RequestData> queue;
	juce::WaitableEvent wakeUp;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( AsyncNetwork )
};
//-----------------------------------------------------------------------------
