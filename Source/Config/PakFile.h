#pragma once

#include <JuceHeader.h>

#include <unordered_map>
#include <vector>

//-----------------------------------------------------------------------------

// Read-only access to Data.pak (a plain zip by another name). The central
// directory is parsed once up front; after that every access opens its own
// stream, so readers on any thread share no mutable state and need no locks

class PakFile final
{
public:
	// Parse the central directory. Any error leaves the pak invalid
	bool open ( const juce::File& _pakFile );

	[[ nodiscard ]] bool isValid () const				{	return ! entries.empty ();	}
	[[ nodiscard ]] const juce::File& getFile () const	{	return pakFile;	}
	[[ nodiscard ]] int getNumEntries () const			{	return int ( entries.size () );	}

	[[ nodiscard ]] bool exists ( const juce::String& path ) const;

	// nullptr when the entry is missing or the pak is unreadable. A deflated
	// entry's stream reads front-to-back; rewinding one re-inflates from the
	// start, so anything seek-heavy belongs in the pak stored, not deflated
	[[ nodiscard ]] std::unique_ptr<juce::InputStream> createStream ( const juce::String& path ) const;

	// The whole entry; empty on a miss or a short read
	[[ nodiscard ]] juce::MemoryBlock load ( const juce::String& path ) const;

	// Paths relative to prefix of the files under it; non-recursive stops at
	// the next '/'. An empty wildcard matches everything
	[[ nodiscard ]] juce::StringArray listFiles ( const juce::String& prefix, const bool recursive, const juce::String& wildcard = {} ) const;

	// Names of the immediate sub-folders under prefix
	[[ nodiscard ]] juce::StringArray listFolders ( const juce::String& prefix ) const;

private:
	struct Entry
	{
		juce::String	path;
		juce::int64		headerOffset;
		juce::int64		compressedSize;
		juce::int64		uncompressedSize;
		bool			deflated;
	};

	[[ nodiscard ]] const Entry* find ( const juce::String& path ) const;

	juce::File			pakFile;
	std::vector<Entry>	entries;

	// Case-insensitive, like the file systems the naked layout lives on
	std::unordered_map<std::string, size_t>	lookup;
};
//-----------------------------------------------------------------------------
