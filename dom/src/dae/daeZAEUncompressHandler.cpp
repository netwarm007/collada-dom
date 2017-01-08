///////NOTE: Not the usual 2006 Sony (c).
/*
 * Copyright 2008 Netallied Systems GmbH.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
//2016 NOTE: The above notice is copied from daeZAEUncrompressHandler.h.
//(It could conceivably be false attribution.)

//2.5 WON'T EXPORT THIS
#ifdef BUILDING_COLLADA_DOM///////////////////////////////////////////////////
#ifdef BUILDING_IN_LIBXML/////////////////////////////////////////////////
#ifdef BUILDING_IN_MINIZIP////////////////////////////////////////////////

//SCHEDULED FOR REMOVAL
#ifdef NDEBUG
#error Remove <fstream>
#endif 
#include <fstream>

//SCHEDULED FOR REMOVAL
//Previously from daePlatform.h
#ifdef NDEBUG
#error REMOVE BOOST
#endif //SCHEDULED FOR REMOVAL
//We use the boost filesystem library for cross-platform file system support. You'll need
//to have boost on your machine for this to work. For the Windows build boost is provided
//in the external-libs folder, but for Linux it's expected that you'll install a boost
//obtained via your distro's package manager. For example on Debian/Ubuntu, you can run
//  apt-get install libboost-filesystem-dev
//to install the boost filesystem library on your machine.
//
//Disable the warnings we get from Boost
//warning C4180: qualifier applied to function type has no meaning; ignored
//warning C4245: 'argument' : conversion from 'int' to 'boost::filesystem::system_error_type',
//  signed/unsigned mismatch
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4180 4245)
#endif
#ifndef NO_BOOST
#include <boost/filesystem/convenience.hpp>
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif	
namespace fs = boost::filesystem;

#include "../../include/dae/daeErrorHandler.h"
#include "../../include/dae/daeZAEUncompressHandler.h"

COLLADA_(namespace)
{//-.
//<-'

//-----------------------------------------------------------------
static const std::string daeZAEUncompressHandler::MANIFEST_FILE_NAME("manifest.xml");
static const std::string daeZAEUncompressHandler::MANIFEST_FILE_ROOT_ELEMENT_NAME("dae_root");
static const int daeZAEUncompressHandler::CASE_INSENSITIVE = 2;
static const int daeZAEUncompressHandler::BUFFER_SIZE = 1024;
static const std::string daeZAEUncompressHandler::EMPTY_STRING = "";

//-----------------------------------------------------------------

daeZAEUncompressHandler::daeZAEUncompressHandler(const daeURI &zaeFile)
: mZipFile(nullptr)
,mZipFileURI(zaeFile)
,mValidZipFile(false)
,mRootFilePath("")
{
	std::string zipFilePath = cdom::uriToNativePath(zaeFile.getURI());
	mZipFile = unzOpen(zipFilePath.c_str());

	mValidZipFile = mZipFile!=nullptr;

	#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
	mTmpDir = (fs::temp_directory_path()/fs::unique_path()).string();
	#else
	mTmpDir = cdom::getSafeTmpDir()+cdom::getRandomFileName()+'/'+mZipFileURI.pathFile()+'/';
	#endif

}

//-----------------------------------------------------------------

daeZAEUncompressHandler::~daeZAEUncompressHandler()
{
	if(mZipFile!=nullptr)
		unzClose(mZipFile);
}

//-----------------------------------------------------------------

const std::string &daeZAEUncompressHandler::obtainRootFilePath()
{

	if(!isZipFile())
		return EMPTY_STRING;
	if(boost::filesystem::create_directories(mTmpDir))
	{
		if(extractArchive(mZipFile,mTmpDir))
		{
			if(retrieveRootURIFromManifest(mTmpDir))
			{
				return mRootFilePath;
			}
			else
			{
				//TODO find root file without manifest
			}
		}
		else
		{
			daeErrorHandler::get()->handleError("Error extracting archive in daeZAEUncompressHandler::obtainRootFilePath\n");
		}
	}
	else
	{
		daeErrorHandler::get()->handleError("Error creating tmp dir in daeZAEUncompressHandler::obtainRootFilePath\n");
	}

	boost::filesystem::remove_all(this->getTmpDir());
	return EMPTY_STRING;
}

//-----------------------------------------------------------------

bool daeZAEUncompressHandler::retrieveRootURIFromManifest(const std::string &tmpDir)
{
	//extract via libxml.
	bool error = false;
	std::string manifest_path = (fs::path(tmpDir)/MANIFEST_FILE_NAME).string();
	xmlTextReaderPtr xmlReader = xmlReaderForFile(manifest_path.c_str(),nullptr,0);

	if(xmlReader)
	{
		if(findManifestRootElement(xmlReader))
		{
			if(xmlTextReaderRead(xmlReader))
			{
				if(xmlTextReaderNodeType(xmlReader)==XML_READER_TYPE_TEXT)
				{
					const xmlChar *xmlText = xmlTextReaderConstValue(xmlReader);

					//copy xmlText.
					std::string rootFilePath((daeString)xmlText);

					//destroy xmlText.
					xmlTextReaderRead(xmlReader);

					//cdom::trimWhitespaces(rootFilePath);
					mRootFilePath = (fs::path(tmpDir)/rootFilePath).string();
				}
				else
				{
					error = true;
				}
			}
			else
			{
				error = true;
			}
		}
		else
		{
			error = true;
		}
	}
	else
	{
		error = true;
	}

	if(xmlReader)
		xmlFreeTextReader(xmlReader);
	if(error)
	{
		daeErrorHandler::get()->handleError("Error parsing manifest.xml in daeZAEUncompressHandler::retrieveRootURIFromManifest\n");
		return false;
	}

	return true;
}

//-----------------------------------------------------------------

bool daeZAEUncompressHandler::findManifestRootElement(xmlTextReaderPtr xmlReader)
{
	while(xmlTextReaderNodeType(xmlReader)!=XML_READER_TYPE_ELEMENT)
	{
		if(xmlTextReaderRead(xmlReader)!=1)
		{
			return false;
		}
	}

	daeString elementName = (daeString)xmlTextReaderConstName(xmlReader);
	if(strcmp(elementName,MANIFEST_FILE_ROOT_ELEMENT_NAME.c_str())==0)
	{
		return true;
	}
	return findManifestRootElement(xmlReader);
}

//-----------------------------------------------------------------

bool daeZAEUncompressHandler::extractArchive(unzFile zipFile,const std::string &destDir)
{
	bool error = false;
	unz_global_info globalZipInfo;

	if(unzGetGlobalInfo(zipFile,&globalZipInfo)==UNZ_OK)
	{
		for(unsigned i=0;i<globalZipInfo.number_entry;++i)
		{
			if(!extractFile(zipFile,destDir))
			{
				error = true;
				break;
			}

			if((i+1)<globalZipInfo.number_entry)
			{
				if(unzGoToNextFile(zipFile)!=UNZ_OK)
				{
					daeErrorHandler::get()->handleError("Error moving to next file in zip archive in daeZAEUncompressHandler::extractArchive\n");
					error = true;
					break;
				}
			}
		}
	}
	else
	{
		daeErrorHandler::get()->handleError("Error getting info for zip archive in daeZAEUncompressHandler::extractArchive\n");
		error = true;
	}
	return !error;
}

//-----------------------------------------------------------------

bool daeZAEUncompressHandler::extractFile(unzFile zipFile,const std::string &destDir)
{
	bool error = false;

	unz_file_info fileInfo;
	char currentFileName[256]; //ARGH !!!
	int fileInfoResult = unzGetCurrentFileInfo(zipFile,&fileInfo,currentFileName,sizeof(currentFileName),0,0,0,0);
	if(fileInfoResult==UNZ_OK)
	{
		if(currentFileName[ strlen(currentFileName)-1 ]=='/')
		{
			if(!boost::filesystem::create_directories(fs::path(destDir)/currentFileName))
			{
				daeErrorHandler::get()->handleError("Error creating dir from zip archive in daeZAEUncompressHandler::extractFile\n");
				error = true;
			}
		}
		else
		{
			if(unzOpenCurrentFile(zipFile)==UNZ_OK)
			{

				char *buffer = 0;
				int readBytes = 1;
				buffer = new char[ BUFFER_SIZE ];
				fs::path currentOutFilePath = fs::path(destDir)/std::string(currentFileName);
				std::ofstream outFile(currentOutFilePath.string().c_str(),std::ios::binary);

				while(readBytes>0)
				{
					readBytes = unzReadCurrentFile(zipFile,buffer,BUFFER_SIZE);
					outFile.write(buffer,readBytes);
				}
				delete[] buffer;
				outFile.close();

				if(readBytes>=0)
				{
					if(unzCloseCurrentFile(zipFile)==UNZ_CRCERROR)
					{
						daeErrorHandler::get()->handleError("CRC error while opening file in zip archive in daeZAEUncompressHandler::extractFile\n");
						error = true;
					}
					else
					{
						if(!checkAndExtractInternalArchive(currentOutFilePath.string()))
						{
							error = true;
						}
					}
				}
				else
				{
					daeErrorHandler::get()->handleError("Error reading file in zip archive in daeZAEUncompressHandler::extractFile\n");
					error = true;
				}

			}
			else
			{
				daeErrorHandler::get()->handleError("Error opening file in zip archive in daeZAEUncompressHandler::extractFile\n");
				error = true;
			}
		}
	}
	else
	{
		daeErrorHandler::get()->handleError("Error getting info for file in zip archive in daeZAEUncompressHandler::extractFile\n");
		error = true;
	}

	return !error;
}

//-----------------------------------------------------------------

bool daeZAEUncompressHandler::checkAndExtractInternalArchive(const std::string &filePath)
{
	unzFile zipFile = unzOpen(filePath.c_str());
	if(zipFile==nullptr)
	{
		//TODO check for other compression formats.
		return true;
	}

	bool error = false;

	boost::filesystem::path archivePath(filePath);
	std::string dir = archivePath.branch_path().string();

	const std::string &randomSegment = cdom::getRandomFileName();
	std::string tmpDir = dir+'/'+randomSegment+cdom::getFileSeparator();
	if(boost::filesystem::create_directory(tmpDir))
	{
		if(!extractArchive(zipFile,tmpDir))
		{
			daeErrorHandler::get()->handleError("Could not extract internal zip archive in daeZAEUncompressHandler::checkAndExtractInternalArchive\n");
			error = true;
		}
	}
	else
	{
		daeErrorHandler::get()->handleError("Could not create temporary directory for extracting internal zip archive in daeZAEUncompressHandler::checkAndExtractInternalArchive\n");
		error = true;
	}

	unzClose(zipFile);

	if(!error)
	{
		if(boost::filesystem::remove(archivePath))
		{
			boost::filesystem::rename(tmpDir,archivePath);
		}
		else
		{
			daeErrorHandler::get()->handleError("Could not remove internal zip archive in daeZAEUncompressHandler::checkAndExtractInternalArchive\n");
			error = true;
		}
	}

	return !error;
}

//---.
}//<-'

#endif //BUILDING_IN_MINIZIP////////////////////////////////////////////////
#endif //BUILDING_IN_LIBXML/////////////////////////////////////////////////
#endif //BUILDING_COLLADA_DOM///////////////////////////////////////////////

/*C1071*/