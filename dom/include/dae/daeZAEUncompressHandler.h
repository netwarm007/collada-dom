///////NOTE: Not the usual 2006 Sony (c).
/*
 * Copyright 2008 Netallied Systems GmbH.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

//2.5 WON'T EXPORT THIS
#ifdef BUILDING_COLLADA_DOM///////////////////////////////////////////////
#ifdef BUILDING_IN_LIBXML/////////////////////////////////////////////////
#ifdef BUILDING_IN_MINIZIP////////////////////////////////////////////////

//hack: __COLLADA_DOM__DAE_ZAE_UNCOMPRESS_HANDLER_H__ is used as a sanifty check.
#ifndef __COLLADA_DOM__DAE_ZAE_UNCOMPRESS_HANDLER_H__
#define __COLLADA_DOM__DAE_ZAE_UNCOMPRESS_HANDLER_H__

#include <unzip.h>
#include <libxml/xmlreader.h>
#include "../dae.h"

COLLADA_(namespace)
{//-.
//<-'

/**
 * Takes an URI to a ZAE file and extracts it to a temporary directory.
 * Use obtainRootFilePath() to accomplish this.
 *
 * The whole ZAE archive gets extracted because it is not specified
 * how an URL pointing inside a ZAE archive should look like.
 * By extracting the whole archive we can use the 'file' scheme.
 */
class daeZAEUncompressHandler
{
COLLADA_(private)
	//zip file this object operates on.
	unzFile mZipFile;

	//URI th zip file this object operates on.
	const daeURI &mZipFileURI;

	//indicates if the passed URI is a valid zip file.
	bool mValidZipFile;

	//path to root file in archive this object handles.
	std::string mRootFilePath;

	//tmp dir where this archive is extracted.
	std::string mTmpDir;

	//disable copy c-tor and assignment operator.
	daeZAEUncompressHandler(const daeZAEUncompressHandler &copy);
	daeZAEUncompressHandler &operator=(const daeZAEUncompressHandler &copy);

COLLADA_(public)
	//Name of manifest file inside ZAE.
	static const std::string MANIFEST_FILE_NAME;
	//Root xml element inside manifest file.
	static const std::string MANIFEST_FILE_ROOT_ELEMENT_NAME;
	//Case insensitivity constant from minizip.
	static const int CASE_INSENSITIVE;
	//Buffer size for extracting files from zip archive.
	static const int BUFFER_SIZE;
	//Empty string to be returned in case of error.
	static const std::string EMPTY_STRING;

	/**
	 * C-Tor.
	 * @param zaeFile URI to the ZAE file to open.
	 */
	daeZAEUncompressHandler(const daeURI &zaeFile);

	/**
	 * D-Tor.
	 */
	virtual ~daeZAEUncompressHandler();

	/**
	 * Returns true if this object has been initialized
	 * with a zip file.
	 */
	bool isZipFile(){ return mValidZipFile;	}

	/**
	 * Extracts ZAE file and returns resulting path to the root daeDOM file.
	 */
	std::string obtainRootFilePath();

	/**
	 * Returns currently known path to root daeDOM of ZAE file.
	 * Only valid after obtainRootFilePath() has been called.
	 */
	std::string getRootFilePath(){ return mRootFilePath; }

	/**
	 * Returns used temp dir.
	 */
	std::string getTmpDir(){ return mTmpDir; }

COLLADA_(private)
	/**
	 * Tries to open manifest.xml inside tmpDir. On success
	 * it parses the XML file to find URI of root daeDOM.
	 */
	bool retrieveRootURIFromManifest(const std::string &tmpDir);

	/**
	 * Iterates over zip archive and extracts each file.
	 */
	bool extractArchive(unzFile zipFile, const std::string &destDir);

	/**
	 * Extracts the current file inside zip archive.
	 */
	bool extractFile(unzFile zipFile, const std::string &destDir);

	/**
	 * Finds <dae_root> element in manifest.xml. Used by retrieveRootURIFromManifest().
	 */
	bool findManifestRootElement(xmlTextReaderPtr xmlReader);

	/**
	 * Checks if an extracted file is a zip archive itself and extracts it.
	 */
	bool checkAndExtractInternalArchive(const std::string &filePath);
};

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_ZAE_UNCOMPRESS_HANDLER_H__

#endif //BUILDING_IN_MINIZIP////////////////////////////////////////////////
#endif //BUILDING_IN_LIBXML/////////////////////////////////////////////////
#endif //BUILDING_COLLADA_DOM///////////////////////////////////////////////

/*C1071*/