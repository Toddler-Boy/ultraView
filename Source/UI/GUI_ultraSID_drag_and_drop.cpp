#include "GUI_ultraSID.h"

//-----------------------------------------------------------------------------

bool GUI_ultraSID::isInterestedInFileDrag ( const juce::StringArray& files )
{
	for ( const auto& f : files )
		if ( f.endsWithIgnoreCase ( ".sid" ) )
			 return true;

	return false;
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::filesDropped ( const juce::StringArray& files, int /*x*/, int /*y*/ )
{
	juce::StringArray	sidFiles;

	for ( const auto& f : files )
		if ( f.endsWithIgnoreCase ( ".sid" ) )
			sidFiles.add ( f );

	addSidTunes ( sidFiles );
}
//-----------------------------------------------------------------------------

bool GUI_ultraSID::isInterestedInTextDrag ( const juce::String& text )
{
	const auto	trimmed = text.trim ().toLowerCase ();

	if ( trimmed.startsWith ( "http://" ) || trimmed.startsWith ( "https://" ) )
	{
		const auto	url = juce::URL ( trimmed );
		const auto	domain = url.getDomain ().toLowerCase ();

		if ( domain == "csdb.dk" )
			return true;

		const auto	fname = url.getFileName ().toLowerCase ();
		if ( fname.endsWith ( ".sid" ) )
			return true;
	}

	return false;
}
//-----------------------------------------------------------------------------

void GUI_ultraSID::textDropped ( const juce::String& text, int /*x*/, int /*y*/ )
{
	const auto	dlURL = juce::URL ( text.trim () );

	if ( ! dlURL.getDomain ().equalsIgnoreCase ( "csdb.dk" ) || ! dlURL.getSubPath ().endsWithIgnoreCase ( ".sid" ) )
		return;

	downloader.startAsyncDownload ( dlURL, [ this ] ( gin::DownloadManager::DownloadResult res )
	{
		if ( res.ok )
		{
			auto	filename = res.url.getSubPath ().fromLastOccurrenceOf ( "/", false, false );

			if ( res.url.getDomain ().equalsIgnoreCase ( "csdb.dk" ) )
				filename = "";

			if ( filename.isEmpty () )
			{
				// Parse content-disposition to get filename
				auto	dispo = juce::StringArray::fromTokens ( res.responseHeaders.getValue ( "content-disposition", "" ), ";", "" );
				dispo.trim ();
				dispo.removeEmptyStrings ();

				for ( const auto& d : dispo )
				{
					if ( d.startsWithIgnoreCase ( "filename=" ) )
					{
						filename = d.fromFirstOccurrenceOf ( "=", false, false ).unquoted ().trim ();
						break;
					}
				}
			}

			if ( filename.isNotEmpty () )
			{
				auto	tempFile = juce::File::getSpecialLocation ( juce::File::tempDirectory ).getChildFile ( filename );

				gin::overwriteWithData ( tempFile, res.data );

				addSidTunes ( { tempFile.getFullPathName () } );
			}
		}
		else
		{
			Z_ERR ( "Download failed HTTP/" << res.httpCode << " for " << res.url.toString ( true ) );
		}
	} );
}
//-----------------------------------------------------------------------------
