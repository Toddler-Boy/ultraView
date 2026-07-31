#pragma once

#include <JuceHeader.h>

#include <atomic>
#include <functional>
#include <map>
#include <type_traits>
#include <utility>

//-----------------------------------------------------------------------------

// The transport of the global message bus: a single broadcaster (the main
// window) that everything sends through, the wire-format parser, and the
// receiving-side router. Senders don't use this directly, they send typed
// structs via their send () members (Helpers/Messages.h).

namespace UI
{
	// Set/cleared by the message thread, read by senders on any thread
	extern std::atomic<juce::ActionBroadcaster*>	ab;
	void setActionBroadCaster ( juce::ActionBroadcaster* _ab ) noexcept;
}

namespace msg
{
	// Wire-safe quoting for payload strings: embedded '"' and '%' are escaped
	// (%22 / %25), then the whole field is quoted. parseActionMessage decodes
	[[ nodiscard ]] juce::String quoted ( const juce::String& text );

	[[ nodiscard ]] std::pair<juce::String, juce::StringArray> parseActionMessage ( const juce::String& message );

	// Verb -> handler dispatch table. Handlers register per typed message;
	// dispatch () parses the wire string, decodes the payload and calls the
	// handler. Messages with a static `sub` route on "verb sub", so verbs
	// shared by several messages (download, playlist) fan out here instead
	// of inside one big handler.
	class Router
	{
	public:
		// Handler is either f ( const M& ) or, for payload-free reactions, f ()
		template<typename M, typename Fn>
		void on ( Fn handler )
		{
			auto&	slot = routes[ key<M> () ];
			jassert ( ! slot );	// duplicate registration for this verb

			slot = [ handler ] ( const juce::StringArray& params )
			{
				if constexpr ( std::is_invocable_v<Fn> )
					handler ();
				else
					handler ( M::decode ( params ) );
			};
		}

		// Returns false if no handler matched
		[[ nodiscard ]] bool dispatch ( const juce::String& message ) const;

	private:
		template<typename M>
		[[ nodiscard ]] static juce::String key ()
		{
			if constexpr ( requires { M::sub; } )
				return juce::String ( M::verb ) + " " + M::sub;
			else
				return M::verb;
		}

		std::map<juce::String, std::function<void ( const juce::StringArray& )>>	routes;
	};
}
//-----------------------------------------------------------------------------
