#include "DataSource.h"

#include "Config/FilePaths.h"
#include "Config/PakFile.h"

//-----------------------------------------------------------------------------

namespace
{
	struct State
	{
		bool		pak = false;
		bool		valid = false;
		PakFile		pakFile;
		juce::File	folder;		// The naked data root, or the pak's parent
	};
	//-----------------------------------------------------------------------------

	// The UI's explicit $USER$ picks; the loader lambdas read these to decide
	// which variant of a shared name to serve (message thread writes, loads may
	// come from the GL thread)
	struct UserCRTSelection
	{
		juce::CriticalSection	lock;

		juce::String	overlay;	// Empty = factory variant selected
		juce::String	mask;		// Stem without ".png"
	};
	//-----------------------------------------------------------------------------

	[[ nodiscard ]] UserCRTSelection& userSelection ()
	{
		static UserCRTSelection	s;

		return s;
	}
	//-----------------------------------------------------------------------------

	[[ nodiscard ]] State fromPak ( const juce::File& pakFile )
	{
		State	s;

		s.pak = true;
		s.folder = pakFile.getParentDirectory ();
		s.valid = s.pakFile.open ( pakFile )
			   && ! s.pakFile.listFiles ( "CRTEmulation/Shaders/", true ).isEmpty ();

		return s;
	}
	//-----------------------------------------------------------------------------

	// The lime shader stack reads the CRT data through its content hook: the
	// nominal files it builds under getCRTRoot () map straight onto pak
	// entries; anything outside the data folder falls through to the disk.
	// User CRT content merges in here: a folder in user/Overlays shadows the
	// factory overlay of the same name whole, a png in user/CRT Masks shadows
	// (or adds) a single mask
	void installLimeContentLoader ( const juce::File& dataFolder )
	{
		auto relative = [ dataFolder ] ( const juce::File& file ) -> juce::String
		{
			if ( ! file.isAChildOf ( dataFolder ) )
				return {};

			return file.getRelativePathFrom ( dataFolder ).replaceCharacter ( '\\', '/' );
		};

		// The real user file behind a nominal path, or invalid when factory
		// content applies. A name owned by both sides serves the user variant
		// only when it is the explicit $USER$ pick; a name only the user has
		// always serves from the user folder. User overlays shadow per-folder:
		// files missing inside stay missing (no factory fallback)
		auto userOverride = [] ( const juce::String& rel ) -> juce::File
		{
			if ( rel.startsWithIgnoreCase ( "CRTEmulation/Overlays/" ) )
			{
				const auto	sub = rel.fromFirstOccurrenceOf ( "Overlays/", false, true );
				const auto	name = sub.upToFirstOccurrenceOf ( "/", false, false );

				if ( const auto userRoot = filepaths::getUserOverlaysPath (); userRoot != juce::File () && name.isNotEmpty () )
					if ( userRoot.getChildFile ( name ).isDirectory () )
					{
						const auto	selected = [ &name ]
						{
							const juce::ScopedLock	sl ( userSelection ().lock );
							return userSelection ().overlay.equalsIgnoreCase ( name );
						} ();

						if ( selected || ! datasource::exists ( "CRTEmulation/Overlays/" + name + "/profile.yml" ) )
							return userRoot.getChildFile ( sub );
					}
			}
			else if ( rel.startsWithIgnoreCase ( "CRTEmulation/CRT Masks/" ) )
			{
				const auto	sub = rel.fromFirstOccurrenceOf ( "CRT Masks/", false, true );

				if ( const auto userRoot = filepaths::getUserCRTMasksPath (); userRoot != juce::File () && sub.isNotEmpty () )
					if ( const auto f = userRoot.getChildFile ( sub ); f.existsAsFile () )
					{
						const auto	selected = [ &f ]
						{
							const juce::ScopedLock	sl ( userSelection ().lock );
							return userSelection ().mask.equalsIgnoreCase ( f.getFileNameWithoutExtension () );
						} ();

						if ( selected || ! datasource::exists ( rel ) )
							return f;
					}
			}

			return {};
		};

		lime::content::Loader	loader;

		loader.load = [ relative, userOverride ] ( const juce::File& file ) -> juce::MemoryBlock
		{
			juce::MemoryBlock	mb;

			if ( const auto rel = relative ( file ); rel.isNotEmpty () )
			{
				if ( const auto user = userOverride ( rel ); user != juce::File () )
					user.loadFileAsData ( mb );
				else
					mb = datasource::loadData ( rel );
			}
			else
			{
				file.loadFileAsData ( mb );
			}

			return mb;
		};

		loader.exists = [ relative, userOverride ] ( const juce::File& file )
		{
			if ( const auto rel = relative ( file ); rel.isNotEmpty () )
			{
				if ( const auto user = userOverride ( rel ); user != juce::File () )
					return user.existsAsFile ();

				return datasource::exists ( rel );
			}

			return file.existsAsFile ();
		};

		loader.list = [ relative ] ( const juce::File& dir, const bool recursive, const juce::String& wildcard, const bool folders )
		{
			const auto	rel = relative ( dir );

			if ( rel.isEmpty () )
				return dir.findChildFiles ( ( folders ? juce::File::findDirectories : juce::File::findFiles ) | juce::File::ignoreHiddenFiles,
											recursive, wildcard.isEmpty () ? "*" : wildcard );

			juce::Array<juce::File>	ret;
			for ( const auto& name : folders ? datasource::listFolders ( rel ) : datasource::listFiles ( rel, recursive, wildcard ) )
				ret.add ( dir.getChildFile ( name ) );

			// Merge user CRT entries as nominal children of dir, so later loads
			// route back through userOverride; user names shadow factory ones
			auto merge = [ &ret, &dir, &wildcard ] ( const juce::File& userRoot, const int whatToLookFor )
			{
				if ( userRoot == juce::File () )
					return;

				for ( const auto& f : userRoot.findChildFiles ( whatToLookFor | juce::File::ignoreHiddenFiles, false, wildcard.isEmpty () ? "*" : wildcard ) )
				{
					const auto	name = f.getFileName ();

					auto	known = false;
					for ( const auto& existing : ret )
						known = known || existing.getFileName ().equalsIgnoreCase ( name );

					if ( ! known )
						ret.add ( dir.getChildFile ( name ) );
				}
			};

			if ( folders && rel.equalsIgnoreCase ( "CRTEmulation/Overlays" ) )
				merge ( filepaths::getUserOverlaysPath (), juce::File::findDirectories );
			else if ( ! folders && rel.equalsIgnoreCase ( "CRTEmulation/CRT Masks" ) )
				merge ( filepaths::getUserCRTMasksPath (), juce::File::findFiles );

			return ret;
		};

		lime::content::setLoader ( std::move ( loader ) );
	}
	//-----------------------------------------------------------------------------

	[[ nodiscard ]] State fromFolder ( const juce::File& folder )
	{
		State	s;

		s.folder = folder;
		s.valid = filepaths::hasDataContent ( folder );

		return s;
	}
	//-----------------------------------------------------------------------------

	[[ nodiscard ]] State resolve ()
	{
		#if JUCE_MAC
			// Everything ships inside the bundle: files can't go missing without
			// breaking the code signature, which stops the app from launching at
			// all, so there is nothing to probe
			return fromPak ( juce::File::getSpecialLocation ( juce::File::currentApplicationFile ).getChildFile ( "Contents/Resources/Data.pak" ) );
		#elif JUCE_WINDOWS
			// A pak next to the exe is the one user layout, and it wins over
			// everything: dropping one beside a dev build tests the release path.
			// A pak that exists but doesn't parse fails hard on purpose
			if ( const auto pak = juce::File::getSpecialLocation ( juce::File::currentExecutableFile ).getSiblingFile ( "Data.pak" ); pak.existsAsFile () )
				return fromPak ( pak );

			// Developer: naked Data in the working directory (the repo checkout)
			if ( filepaths::isDeveloperMode () )
				if ( auto d = juce::File::getCurrentWorkingDirectory ().getChildFile ( "Data" ); filepaths::hasDataContent ( d ) )
					return fromFolder ( d );

			// Installed
			if ( auto pak = juce::File::getSpecialLocation ( juce::File::globalApplicationsDirectory ).getChildFile ( "ultraView/Data.pak" ); pak.existsAsFile () )
				return fromPak ( pak );

			// No data anywhere: keep the portable pak as the nominal source, a
			// valid absolute path that fails the content check
			return fromPak ( juce::File::getSpecialLocation ( juce::File::currentExecutableFile ).getSiblingFile ( "Data.pak" ) );
		#elif JUCE_LINUX
			return fromFolder ( juce::File ( "/usr/share/ultraView" ) );
		#else
			jassertfalse;	// Unsupported platform
			return {};
		#endif
	}
	//-----------------------------------------------------------------------------

	// Resolved once, on first use. The lime hook installs here so it exists
	// before any CRT component constructs; its lambdas only run later, when
	// this state is long complete
	[[ nodiscard ]] const State& state ()
	{
		static const State	s = [] ()
		{
			auto	st = resolve ();

			if ( st.pak && st.valid )
				installLimeContentLoader ( st.folder );

			return st;
		} ();

		return s;
	}
	//-----------------------------------------------------------------------------

	[[ nodiscard ]] juce::File nakedFile ( const juce::String& path )
	{
		return path.isEmpty () ? state ().folder : state ().folder.getChildFile ( path );
	}
}
//-----------------------------------------------------------------------------

bool datasource::isPak ()
{
	return state ().pak;
}
//-----------------------------------------------------------------------------

bool datasource::isValid ()
{
	return state ().valid;
}
//-----------------------------------------------------------------------------

juce::String datasource::describe ()
{
	const auto&	s = state ();

	if ( s.pak )
		return s.pakFile.getFile ().getFullPathName () + " (" + juce::String ( s.pakFile.getNumEntries () ) + " entries)";

	return s.folder.getFullPathName ();
}
//-----------------------------------------------------------------------------

bool datasource::exists ( const juce::String& path )
{
	const auto&	s = state ();

	if ( s.pak )
		return s.pakFile.exists ( path );

	return nakedFile ( path ).existsAsFile ();
}
//-----------------------------------------------------------------------------

juce::String datasource::loadText ( const juce::String& path )
{
	const auto&	s = state ();

	if ( ! s.pak )
		return nakedFile ( path ).loadFileAsString ();

	const auto	mb = s.pakFile.load ( path );

	return juce::String::createStringFromData ( mb.getData (), int ( mb.getSize () ) );
}
//-----------------------------------------------------------------------------

juce::MemoryBlock datasource::loadData ( const juce::String& path )
{
	const auto&	s = state ();

	if ( s.pak )
		return s.pakFile.load ( path );

	juce::MemoryBlock	mb;
	nakedFile ( path ).loadFileAsData ( mb );

	return mb;
}
//-----------------------------------------------------------------------------

juce::Image datasource::loadImage ( const juce::String& path )
{
	const auto	mb = loadData ( path );
	if ( mb.getSize () == 0 )
		return {};

	return juce::ImageFileFormat::loadFrom ( mb.getData (), mb.getSize () );
}
//-----------------------------------------------------------------------------

std::unique_ptr<juce::InputStream> datasource::openStream ( const juce::String& path )
{
	const auto&	s = state ();

	if ( s.pak )
		return s.pakFile.createStream ( path );

	auto	in = std::make_unique<juce::FileInputStream> ( nakedFile ( path ) );

	return in->openedOk () ? std::move ( in ) : nullptr;
}
//-----------------------------------------------------------------------------

juce::StringArray datasource::listFiles ( const juce::String& prefix, const bool recursive, const juce::String& wildcard )
{
	const auto&	s = state ();

	if ( s.pak )
		return s.pakFile.listFiles ( prefix, recursive, wildcard );

	const auto	root = nakedFile ( prefix );
	const auto	files = root.findChildFiles ( juce::File::findFiles | juce::File::ignoreHiddenFiles, recursive, wildcard.isEmpty () ? "*" : wildcard );

	juce::StringArray	ret;
	for ( const auto& f : files )
		ret.add ( f.getRelativePathFrom ( root ).replaceCharacter ( '\\', '/' ) );

	return ret;
}
//-----------------------------------------------------------------------------

juce::StringArray datasource::listFolders ( const juce::String& prefix )
{
	const auto&	s = state ();

	if ( s.pak )
		return s.pakFile.listFolders ( prefix );

	const auto	folders = nakedFile ( prefix ).findChildFiles ( juce::File::findDirectories | juce::File::ignoreHiddenFiles, false, "*" );

	juce::StringArray	ret;
	for ( const auto& f : folders )
		ret.add ( f.getFileName () );

	return ret;
}
//-----------------------------------------------------------------------------

juce::File datasource::getCRTRoot ()
{
	return state ().folder.getChildFile ( "CRTEmulation" );
}
//-----------------------------------------------------------------------------

juce::File datasource::getDevFile ( const juce::String& path )
{
	if ( state ().pak )
	{
		Z_ERR ( "Developer file access in pak mode: " << ( path.isEmpty () ? "<root>" : path ) );
		jassertfalse;
		return {};
	}

	return nakedFile ( path );
}
//-----------------------------------------------------------------------------

void datasource::setActiveUserOverlay ( const juce::String& name )
{
	const juce::ScopedLock	sl ( userSelection ().lock );

	userSelection ().overlay = name;
}
//-----------------------------------------------------------------------------

void datasource::setActiveUserCRTMask ( const juce::String& name )
{
	const juce::ScopedLock	sl ( userSelection ().lock );

	userSelection ().mask = name;
}
//-----------------------------------------------------------------------------
