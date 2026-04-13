#pragma once

#include <JuceHeader.h>

class yamlFile
{
public:
	virtual void load ()
	{
		result.clear ();
		parse ();
	}
	//-----------------------------------------------------------------------------

protected:
	juce::File	file;
	std::unordered_map<juce::String, juce::String>	result;

private:
	void parse ()
	{
		const auto	input = file.loadFileAsString ();

		juce::StringArray pathStack;
		juce::Array<int> indentStack;

		const auto lines = juce::StringArray::fromLines ( input );

		for ( auto i = 0; i < lines.size (); ++i )
		{
			const auto&	line = lines[ i ];

			if ( line.trim ().isEmpty () || line.trim ().startsWith ( "#" ) )
				continue;

			const auto	indent = line.initialSectionContainingOnly ( " \t\r\n" ).length ();

			// Adjust path based on indentation
			while ( ! indentStack.isEmpty () && indentStack.getLast () >= indent )
			{
				indentStack.removeLast ();
				pathStack.remove ( pathStack.size () - 1 );
			}

			if ( line.containsChar ( ':' ) )
			{
				const auto	key = line.upToFirstOccurrenceOf ( ":", false, false ).trim ().toLowerCase ();
				const auto	val = line.fromFirstOccurrenceOf ( ":", false, false ).trim ();

				pathStack.add ( key );
				indentStack.add ( indent );
				const auto fullPath = pathStack.joinIntoString ( "/" );

				if ( val == "|" )
				{
					juce::String block;
					while ( ++i < lines.size () )
					{
						const auto&	next = lines[ i ];

						if ( next.trim ().isEmpty () )
						{
							block << "\n";
							continue;
						}

						if ( next.initialSectionContainingOnly ( " \t" ).length () <= indent )
						{
							i--;
							break; // Backtrack for main loop
						}
						block << next.trim () << "\n";
					}
					result[ fullPath ] = block;
				}
				else if ( val.isNotEmpty () )
				{
					result[ fullPath ] = val.unquoted ();
				}
			}
		}
	}
};
//-----------------------------------------------------------------------------
