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

namespace
{

void flattenJsonToPairArray ( const juce::var& v, juce::String currentPath, juce::StringPairArray& results )
{
	if ( v.isArray () )
	{
		if ( auto* arr = v.getArray (); arr->isEmpty () )
			results.set ( currentPath, {} );
		else
		{
			for ( auto i = 0; i < arr->size (); ++i )
			{
				juce::String nextPath = currentPath + "/" + juce::String ( i );
				flattenJsonToPairArray ( arr->getReference ( i ), nextPath, results );
			}
		}
	}
	else if ( v.isObject () )
	{
		if ( auto& props = v.getDynamicObject ()->getProperties (); props.isEmpty () )
			results.set ( currentPath, {} );
		else
		{
			for ( auto& prop : props )
			{
				juce::String nextPath = currentPath.isEmpty () ? prop.name.toString () : currentPath + "/" + prop.name.toString ();
				flattenJsonToPairArray ( prop.value, nextPath, results );
			}
		}
	}
	else
	{
		results.set ( currentPath, v.toString () );
	}
}

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
			.withConnectionTimeoutMs ( 1000 )
			.withNumRedirectsToFollow ( 0 );

		if ( auto stream = req.url.createInputStream ( options ) )
		{
			if ( req.callback )
			{
				const auto	jsonString = stream->readEntireStreamAsString ();

				juce::StringPairArray	results;
				flattenJsonToPairArray ( juce::JSON::parse ( jsonString ), "", results );

				req.callback ( results, req.statusCode );
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
