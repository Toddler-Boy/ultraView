#pragma once

#include <JuceHeader.h>

#include <variant>

//-----------------------------------------------------------------------------

class iniFile
{
public:
	using vec2i = std::pair<int, int>;
	using vec2f = std::pair<float, float>;

	struct value
	{
		std::string	section;
		std::string	key;
		std::variant<bool, float, double, int, std::string, vec2i, vec2f>	defaultValue;
	};

	iniFile ( const std::vector<value>& defaultValues, const juce::File& _file = {}, const bool _readOnly = false )
		: readOnly ( _readOnly )
		, values ( defaultValues )
	{
		if ( _file != juce::File () )
			load ( _file );
	}
	//-------------------------------------------------------------------------

	~iniFile ()
	{
		save ();
	}
	//-------------------------------------------------------------------------

	[[ nodiscard ]] bool hasData () const
	{
		return ! data.empty ();
	}
	//-------------------------------------------------------------------------

	void load ( const juce::File& _file )
	{
		data.clear ();
		dirty = false;

		file = _file;
		file.create ();

		juce::StringArray	lines;
		file.readLines ( lines );

		lines.removeEmptyStrings ();
		lines.trim ();

		loading = true;
		for ( juce::String section = ""; const auto& line : lines )
		{
			if ( line.startsWithChar ( '[' ) )
			{
				section = line.removeCharacters ( "[] " );
				continue;
			}

			if ( line.startsWithChar ( ';' ) )
				continue;

			auto	key = line.upToFirstOccurrenceOf ( "=", false, false ).trim ();
 			auto	value = line.fromFirstOccurrenceOf ( "=", false, false ).upToFirstOccurrenceOf ( ";", false, false ).trim ();

			setString ( section, key, value );
		}
		loading = false;

		dirty = false;
	}
	//-----------------------------------------------------------------------------

	void save ()
	{
		if ( readOnly || ! dirty || file == juce::File () || ! hasData () )
			return;

		dirty = false;

		juce::String	out;

		for ( const auto& [ section, collection ] : data )
		{
			if ( section.isNotEmpty () )
				out += juce::String ( "\n[" + section + "]\n" );

			for ( const auto& key : collection.getAllKeys () )
				out += juce::String ( key + " = " + collection[ key ].trim () + "\n" );
		}
		out = out.trim () + "\n";

		file.replaceWithText ( out );
	}
	//-----------------------------------------------------------------------------

	template<typename T>
	[[ nodiscard ]] T get ( const juce::String& section, const juce::String& key )
	{
		jassert ( hasData () );

		T	def = getDefault<T> ( section, key );

		std::function<juce::StringArray()>	splitStr;

		if constexpr ( std::is_same_v<T, vec2i> || std::is_same_v<T, vec2f> )
		{
			splitStr = [ & ]
			{
				auto	str = getString ( section, key, juce::String ( def.first ) + ", " + juce::String ( def.second ) );
				auto	parts = juce::StringArray::fromTokens ( str, ",", "" );

				parts.trim ();
				parts.removeEmptyStrings ();
				return parts;
			};
		}

		if constexpr ( std::is_same_v<T, bool> )
			return getString ( section, key, juce::String ( def ? "on" : "off" ) ).equalsIgnoreCase ( "on" );
		else if constexpr ( std::is_same_v<T, float> )
			return getString ( section, key, juce::String ( def ) ).getFloatValue ();
		else if constexpr ( std::is_same_v<T, double> )
			return getString ( section, key, juce::String ( def ) ).getDoubleValue ();
		else if constexpr ( std::is_same_v<T, int> )
			return getString ( section, key, juce::String ( def ) ).getIntValue ();
		else if constexpr ( std::is_same_v<T, vec2i> )
		{
			auto	parts = splitStr ();

			if ( parts.size () != 2 )
				return def;

			return { parts[ 0 ].getIntValue (), parts[ 1 ].getIntValue () };
		}
		else if constexpr ( std::is_same_v<T, vec2f> )
		{
			auto	parts = splitStr ();

			if ( parts.size () != 2 )
				return def;

			return { parts[ 0 ].getFloatValue (), parts[ 1 ].getFloatValue () };
		}
		else if constexpr ( std::is_same_v<T, juce::String> )
			return getString ( section, key, def );
		else if constexpr ( std::is_same_v<T, std::string> )
			return getString ( section, key, def ).toStdString ();
		else
			static_assert ( false, "Unsupported type in iniFile::get" );
	}
	//-----------------------------------------------------------------------------

	template<typename T>
	[[ nodiscard ]] T getDefault ( const juce::String& section, const juce::String& key )
	{
		jassert ( hasData () );

		if constexpr ( std::is_same_v<T, juce::String> )
			return T ( std::get<std::string> ( findEntry ( section, key ).defaultValue ) );
		else
		{
			// Ensure the type matches the default value type
			jassert ( std::holds_alternative<T> ( findEntry ( section, key ).defaultValue ) );
			return std::get<T> ( findEntry ( section, key ).defaultValue );
		}
	}
	//-----------------------------------------------------------------------------

	template<typename T>
	void set ( const juce::String& section, const juce::String& key, const T value )
	{
		jassert ( hasData () );

		if constexpr ( std::is_same_v<T, bool> )
			setString ( section, key, juce::String ( value ? "on" : "off" ) );
		else if constexpr ( std::is_same_v<T, float> || std::is_same_v<T, double> )
			setString ( section, key, juce::String ( value, 0 ) );
		else if constexpr ( std::is_same_v<T, int> )
			setString ( section, key, juce::String ( value ) );
		else if constexpr ( std::is_same_v<T, vec2i> )
			setString ( section, key, juce::String ( value.first ) + ", " + juce::String ( value.second ) );
		else if constexpr ( std::is_same_v<T, juce::String> || std::is_same_v<T, std::string> )
			setString ( section, key, value );
		else
			static_assert ( false, "Unsupported type in iniFile::set" );
	}
	//-----------------------------------------------------------------------------

	template<typename T>
	[[ nodiscard ]] T get ( const juce::String& sectionKey )
	{
		jassert ( sectionKey.containsChar ( '/' ) ); // Must be in "section/key" format
		return get<T> ( sectionKey.upToFirstOccurrenceOf ( "/", false, false ), sectionKey.fromFirstOccurrenceOf ( "/", false, false ) );
	}
	//-----------------------------------------------------------------------------

	template<typename T>
	[[ nodiscard ]] T getDefault ( const juce::String& sectionKey )
	{
		jassert ( sectionKey.containsChar ( '/' ) ); // Must be in "section/key" format
		return getDefault<T> ( sectionKey.upToFirstOccurrenceOf ( "/", false, false ), sectionKey.fromFirstOccurrenceOf ( "/", false, false ) );
	}
	//-----------------------------------------------------------------------------

	template<typename T>
	void set ( const juce::String& sectionKey, const T value )
	{
		jassert ( sectionKey.containsChar ( '/' ) ); // Must be in "section/key" format
		set<T> ( sectionKey.upToFirstOccurrenceOf ( "/", false, false ), sectionKey.fromFirstOccurrenceOf ( "/", false, false ), value );
	}
	//-----------------------------------------------------------------------------

private:
	[[ nodiscard ]] const value& findEntry ( const juce::String& section, const juce::String& key )
	{
		jassert ( hasData () );

		for ( const auto& val : values )
		{
			if ( section.equalsIgnoreCase ( juce::String ( val.section ) )
				 && key.equalsIgnoreCase ( juce::String ( val.key ) ) )
			{
				return val;
			}
		}
		// That section/key doesn't exist! Typo?
		jassertfalse;

		static const value	dummy = { "", "", false };
		return dummy;
	}
	//-----------------------------------------------------------------------------

	[[ nodiscard ]] juce::String getString ( const juce::String& section, const juce::String& key, const juce::String& def )
	{
		jassert ( hasData () );

		juce::StringPairArray	strPair;

		// Find value
		auto&	sec = findSection ( section, strPair );

		if ( sec.containsKey ( key ) )
			return sec[ key ];

		if ( def.isNotEmpty () && ! readOnly )
			setString ( section, key, def );

		return def;
	}
	//-----------------------------------------------------------------------------

	void setString ( const juce::String& section, const juce::String& key, const juce::String& value )
	{
		jassert ( hasData () || loading );

		juce::StringPairArray	strPair;

		// Add/replace value
		auto&	sec = findSection ( section, strPair );

		if ( ! loading )
			dirty = dirty || ! sec.containsKey ( key ) || sec[ key ] != value;

		sec.set ( key, value );

		// If findSection returned the default object, then push new section into vector
		if ( sec == strPair )
			data.push_back ( { section, strPair } );
	}
	//-----------------------------------------------------------------------------

	[[ nodiscard ]] std::optional<juce::StringPairArray> getGlobalSection ()
	{
		jassert ( hasData () );

		if ( ! data.empty () )
			return data[ 0 ].second;

		return {};
	}
	//-----------------------------------------------------------------------------

	[[ nodiscard ]] juce::StringPairArray& findSection ( const juce::String& section, juce::StringPairArray& newArr )
	{
		jassert ( hasData () || loading );

		for ( auto& [ secName, secData ] : data )
			if ( section.equalsIgnoreCase ( secName ) )
				return secData;

		return newArr;
	}

	bool	dirty = false;
	bool	loading = false;
	bool	readOnly = false;

	juce::File	file;
	std::vector<std::pair<juce::String, juce::StringPairArray>>	data;
	const std::vector<value>	values;
};
//-----------------------------------------------------------------------------
