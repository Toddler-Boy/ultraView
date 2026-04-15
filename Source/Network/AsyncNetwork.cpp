#include "AsyncNetwork.h"

//-----------------------------------------------------------------------------

AsyncNetwork::AsyncNetwork ()
	: juce::Thread ( "NetworkUploader" )
{
	startThread ( Priority::low );
}
//-----------------------------------------------------------------------------

AsyncNetwork::~AsyncNetwork ()
{
	signalThreadShouldExit ();
	wakeUp.signal ();
	waitForThreadToExit ( 5000 );
}
//-----------------------------------------------------------------------------

void AsyncNetwork::setBaseAddress ( const juce::String& url )
{
	baseAddress = juce::URL ( url );
}
//-----------------------------------------------------------------------------

void AsyncNetwork::get ( const juce::String& ep, NetworkCallback cb, const juce::StringArray& p )
{
	enqueue ( ep, "GET", {}, std::move ( cb ), p );
}
//-----------------------------------------------------------------------------

void AsyncNetwork::post ( const juce::String& ep, const juce::MemoryBlock& d, NetworkCallback cb, const juce::StringArray& p )
{
	enqueue ( ep, "POST", d, std::move ( cb ), p );
}
//-----------------------------------------------------------------------------

void AsyncNetwork::put ( const juce::String& ep, const juce::MemoryBlock& d, NetworkCallback cb, const juce::StringArray& p )
{
	enqueue ( ep, "PUT", d, std::move ( cb ), p );
}
//-----------------------------------------------------------------------------

void AsyncNetwork::enqueue ( const juce::String& endpoint, const juce::String& method, const juce::MemoryBlock& data, NetworkCallback cb, const juce::StringArray& params )
{
	auto	requestUrl = baseAddress.getChildURL ( endpoint );

	if ( ! data.isEmpty () )
		requestUrl = requestUrl.withPOSTData ( data );

	for ( auto i = 0; i < params.size (); i += 2 )
		requestUrl = requestUrl.withParameter ( params[ i ], params[ i + 1 ] );

	const juce::ScopedLock	sl ( queueLock );
	queue.add ( { requestUrl, method, std::move ( cb ) } );
	wakeUp.signal ();
}
//-----------------------------------------------------------------------------

void AsyncNetwork::run ()
{
	while ( ! threadShouldExit () )
	{
		queueLock.enter ();
		if ( queue.isEmpty () )
		{
			queueLock.exit ();
			wakeUp.wait ();
			continue;
		}

		auto	req = queue.removeAndReturn ( 0 );
		queueLock.exit ();

		auto	options = juce::URL::InputStreamOptions ( juce::URL::ParameterHandling::inAddress )
			.withHttpRequestCmd ( req.method )
			.withStatusCode ( &req.statusCode )
			.withConnectionTimeoutMs ( 200 )
			.withNumRedirectsToFollow ( 0 );

		if ( auto stream = req.url.createInputStream ( options ) )
		{
			if ( req.callback )
			{
				const auto	jsonString = stream->readEntireStreamAsString ();
				const auto	json = juce::JSON::parse ( jsonString );

				req.callback ( json, req.statusCode );
			}
		}
		else
		{
			if ( req.callback )
				req.callback ( {}, 0 );
		}
	}
}
//-----------------------------------------------------------------------------
