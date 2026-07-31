#include "PakFile.h"

//-----------------------------------------------------------------------------

namespace
{
	// The little-endian readers work on any alignment
	[[ nodiscard ]] uint32_t u16 ( const uint8_t* p )	{	return uint32_t ( p[ 0 ] ) | uint32_t ( p[ 1 ] ) << 8;	}
	[[ nodiscard ]] uint32_t u32 ( const uint8_t* p )	{	return uint32_t ( p[ 0 ] ) | uint32_t ( p[ 1 ] ) << 8 | uint32_t ( p[ 2 ] ) << 16 | uint32_t ( p[ 3 ] ) << 24;	}

	[[ nodiscard ]] std::string lowerKey ( const juce::String& path )
	{
		return path.replaceCharacter ( '\\', '/' ).toLowerCase ().toStdString ();
	}

	// "UI/themes" and "UI/themes/" both mean the folder
	[[ nodiscard ]] juce::String normalizedPrefix ( const juce::String& prefix )
	{
		if ( prefix.isEmpty () || prefix.endsWithChar ( '/' ) )
			return prefix;

		return prefix + "/";
	}
}
//-----------------------------------------------------------------------------

bool PakFile::open ( const juce::File& _pakFile )
{
	pakFile = _pakFile;
	entries.clear ();
	lookup.clear ();

	juce::FileInputStream	in ( pakFile );
	if ( ! in.openedOk () )
		return false;

	const auto	fileSize = in.getTotalLength ();

	//
	// End-of-central-directory record: 22 fixed bytes plus an optional comment
	// of up to 64K, found by scanning backwards for its signature
	//
	const auto	tailSize = juce::jmin ( fileSize, juce::int64 ( 65557 ) );
	if ( tailSize < 22 )
		return false;

	juce::MemoryBlock	tail;
	in.setPosition ( fileSize - tailSize );
	if ( juce::int64 ( in.readIntoMemoryBlock ( tail, tailSize ) ) != tailSize )
		return false;

	const auto	t = static_cast<const uint8_t*> ( tail.getData () );

	auto	eocd = -1;
	for ( auto i = int ( tailSize ) - 22; i >= 0; --i )
		if ( u32 ( t + i ) == 0x06054b50 )
		{
			eocd = i;
			break;
		}

	if ( eocd < 0 )
	{
		Z_ERR ( "No end-of-central-directory in " << pakFile.getFullPathName () );
		return false;
	}

	const auto	numEntries = int ( u16 ( t + eocd + 10 ) );
	const auto	cdSize = juce::int64 ( u32 ( t + eocd + 12 ) );
	const auto	cdOffset = juce::int64 ( u32 ( t + eocd + 16 ) );

	// 0xFFFF/0xFFFFFFFF mean zip64, which nothing on our side produces
	if ( numEntries == 0xFFFF || cdOffset == 0xFFFFFFFF || cdOffset + cdSize > fileSize )
	{
		Z_ERR ( "Unsupported central directory in " << pakFile.getFullPathName () );
		return false;
	}

	//
	// Central directory: 46 fixed bytes per entry plus name/extra/comment
	//
	juce::MemoryBlock	cd;
	in.setPosition ( cdOffset );
	if ( juce::int64 ( in.readIntoMemoryBlock ( cd, cdSize ) ) != cdSize )
		return false;

	const auto	d = static_cast<const uint8_t*> ( cd.getData () );

	entries.reserve ( size_t ( numEntries ) );
	lookup.reserve ( size_t ( numEntries ) );

	juce::int64	pos = 0;
	for ( auto i = 0; i < numEntries; ++i )
	{
		if ( pos + 46 > cdSize || u32 ( d + pos ) != 0x02014b50 )
		{
			Z_ERR ( "Central directory truncated in " << pakFile.getFullPathName () );
			entries.clear ();
			lookup.clear ();
			return false;
		}

		const auto	method = int ( u16 ( d + pos + 10 ) );
		const auto	compressedSize = juce::int64 ( u32 ( d + pos + 20 ) );
		const auto	uncompressedSize = juce::int64 ( u32 ( d + pos + 24 ) );
		const auto	nameLen = int ( u16 ( d + pos + 28 ) );
		const auto	extraLen = int ( u16 ( d + pos + 30 ) );
		const auto	commentLen = int ( u16 ( d + pos + 32 ) );
		const auto	headerOffset = juce::int64 ( u32 ( d + pos + 42 ) );

		const auto	path = juce::String::fromUTF8 ( reinterpret_cast<const char*> ( d + pos + 46 ), nameLen );

		pos += 46 + nameLen + extraLen + commentLen;

		// Folders carry no data; anything but stored/deflate can't be read
		if ( path.endsWithChar ( '/' ) )
			continue;

		if ( method != 0 && method != 8 )
		{
			Z_ERR ( "Unsupported compression on " << path << " in " << pakFile.getFullPathName () );
			continue;
		}

		lookup[ lowerKey ( path ) ] = entries.size ();
		entries.push_back ( { path, headerOffset, compressedSize, uncompressedSize, method == 8 } );
	}

	return isValid ();
}
//-----------------------------------------------------------------------------

const PakFile::Entry* PakFile::find ( const juce::String& path ) const
{
	const auto	it = lookup.find ( lowerKey ( path ) );

	return it == lookup.end () ? nullptr : &entries[ it->second ];
}
//-----------------------------------------------------------------------------

bool PakFile::exists ( const juce::String& path ) const
{
	return find ( path ) != nullptr;
}
//-----------------------------------------------------------------------------

std::unique_ptr<juce::InputStream> PakFile::createStream ( const juce::String& path ) const
{
	const auto	e = find ( path );
	if ( e == nullptr )
		return nullptr;

	auto	in = std::make_unique<juce::FileInputStream> ( pakFile );
	if ( ! in->openedOk () )
	{
		Z_ERR ( "Cannot open " << pakFile.getFullPathName () );
		return nullptr;
	}

	// The local header repeats name/extra with its own lengths, so the data
	// offset comes from it, not from the central directory
	uint8_t	lh[ 30 ];
	in->setPosition ( e->headerOffset );
	if ( in->read ( lh, 30 ) != 30 || u32 ( lh ) != 0x04034b50 )
	{
		Z_ERR ( "Bad local header for " << e->path << " in " << pakFile.getFullPathName () );
		return nullptr;
	}

	const auto	dataOffset = e->headerOffset + 30 + juce::int64 ( u16 ( lh + 26 ) ) + juce::int64 ( u16 ( lh + 28 ) );

	auto	sub = std::make_unique<juce::SubregionStream> ( in.release (), dataOffset, e->compressedSize, true );

	if ( ! e->deflated )
		return sub;

	return std::make_unique<juce::GZIPDecompressorInputStream> ( sub.release (), true, juce::GZIPDecompressorInputStream::deflateFormat, e->uncompressedSize );
}
//-----------------------------------------------------------------------------

juce::MemoryBlock PakFile::load ( const juce::String& path ) const
{
	const auto	e = find ( path );

	auto	stream = createStream ( path );
	if ( stream == nullptr )
		return {};

	juce::MemoryBlock	mb;
	stream->readIntoMemoryBlock ( mb );

	if ( juce::int64 ( mb.getSize () ) != e->uncompressedSize )
	{
		Z_ERR ( "Short read of " << path << " from " << pakFile.getFullPathName () );
		return {};
	}

	return mb;
}
//-----------------------------------------------------------------------------

juce::StringArray PakFile::listFiles ( const juce::String& prefix, const bool recursive, const juce::String& wildcard ) const
{
	const auto	pre = normalizedPrefix ( prefix );
	const auto	preLen = pre.length ();

	juce::StringArray	ret;

	for ( const auto& e : entries )
	{
		if ( ! e.path.startsWithIgnoreCase ( pre ) )
			continue;

		const auto	relative = e.path.substring ( preLen );

		if ( ! recursive && relative.containsChar ( '/' ) )
			continue;

		if ( wildcard.isNotEmpty () && ! relative.fromLastOccurrenceOf ( "/", false, false ).matchesWildcard ( wildcard, true ) )
			continue;

		ret.add ( relative );
	}

	return ret;
}
//-----------------------------------------------------------------------------

juce::StringArray PakFile::listFolders ( const juce::String& prefix ) const
{
	const auto	pre = normalizedPrefix ( prefix );
	const auto	preLen = pre.length ();

	juce::StringArray	ret;

	for ( const auto& e : entries )
	{
		if ( ! e.path.startsWithIgnoreCase ( pre ) )
			continue;

		const auto	relative = e.path.substring ( preLen );

		if ( ! relative.containsChar ( '/' ) )
			continue;

		ret.addIfNotAlreadyThere ( relative.upToFirstOccurrenceOf ( "/", false, false ) );
	}

	return ret;
}
//-----------------------------------------------------------------------------
