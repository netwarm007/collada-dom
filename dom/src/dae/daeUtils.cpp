/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifdef NDEBUG
#error Remove. (Must implement ZAE first.)
#endif
//REFERENCE: NO LONGER RELEVANT
//THIS ENTIRE FILE/HEADER IS ON THE WAY OUT...
//AT THIS POINT THE HEADER IS GONE, AND ONLY daeZAEUncompressHandler.cpp
//REFERS TO IT. (IT'S THE LAST FILE ON THE TO-DO LIST.)
//SOME PARTS HAVE BEEN MOVED TO daeSIDResolver.cpp WHERE THEY'RE ISOLATED.
//(THAT FILE IS LARGELY UNTOUCHED AS WELL.)
//BITS AND PIECES OF THE "cdom" NAMESPACE HAVE BEEN PROGRESSIVELY REMOVED.
#if 0

#include <string> 
#include <cstdio> //for tmpnam	  
#ifndef NO_BOOST //THIS WASN'T COMMENTED.
#include <boost/filesystem/convenience.hpp> 
#endif

COLLADA_(namespace)
{//-.
//<-'

namespace cdom
{	
	//System type info. We only need to distinguish 
	//between Posix and Winodws for now.
	enum systemType{ Posix,Windows };	

	cdom::systemType getSystemType()
	{
		#ifdef _WIN32
		return Windows;
		#else
		return Posix;
		#endif
	}

	std::string replace
	(const std::string &s, const std::string &replace, const std::string &replaceWith)
	{
		if(replace.empty()) return s;

		std::string result;
		size_t pos1 = 0,pos2 = s.find(replace);
		while(pos2!=std::string::npos)
		{
			result += s.substr(pos1,pos2-pos1);
			result += replaceWith;
			pos1 = pos2+replace.length();
			pos2 = s.find(replace,pos1);
		}

		result += s.substr(pos1,s.length()-pos1);
		return result;
	} 

#ifndef NO_BOOST

	static const std::string &getSystemTmpDir()
	{
		static std::string tmpDir = 
		#ifdef _WIN32
		std::string(getenv("TMP"))+'/';
		#elif defined(__linux__) || defined(__linux)
		"/tmp/";
		#elif defined __APPLE_CC__
		getenv("TMPDIR");
		#elif defined __CELLOS_LV2__
		#error tmp dir for your system unknown
		#else
		#error tmp dir for your system unknown
		#endif
		return tmpDir;
	}

	std::string getRandomFileName()
	{
		char tmpbuffer[L_tmpnam*2+1];
		int i = strlen(tmpnam(tmpbuffer));
		while(i>=0&&tmpbuffer[i]!='\\'&&tmpbuffer[i]!='/') i--;
		return tmpbuffer+i+1;
	}

	const std::string &getSafeTmpDir()
	{
		//there is a race condition here is multiple collada-dom -enabled processes call getSafeTmpDir at the same time.
		//Therefore, have to check if directory already exists before using it. This still leaves the race
		//condition, but makes it more difficult to reproduce. A better alternative would be to stop relying on tmpnam!
		static std::string tmpDir;
		//2016: adding empty check! Assuming this is supposed to remain the same directory throughout.
		if(tmpDir.empty()) do 
		{
			tmpDir = cdom::getSystemTmpDir()+getRandomFileName()+'/';
		}while(boost::filesystem::is_directory(tmpDir));
		return tmpDir;
	}

#endif //NO_BOOST

	//This function takes a URI reference and converts it to an OS file path. Conversion
	//can fail if the URI reference is ill-formed, or if the URI contains a scheme other
	//than "file", in which case an empty string is returned. The 'type' parameter
	//indicates the format of the returned native path.
	//
	//Examples - Windows
	//   uriToNativePath("../folder/file.dae") --> "..\folder\file.dae"
	//   uriToNativePath("/folder/file.dae") --> "\folder\file.dae"
	//   uriToNativePath("file:/C:/folder/file.dae") --> "C:\folder\file.dae"
	//   uriToNativePath("file://otherComputer/file.dae") --> "\\otherComputer\file.dae"
	//   uriToNativePath("http://www.slashdot.org") --> "" (it's not a file scheme URI!)
	//
	//Examples - Linux/Mac
	//   uriToNativePath("../folder/file.dae") --> "../folder/file.dae"
	//   uriToNativePath("file:/folder/file.dae") --> "/folder/file.dae"
	//   uriToNativePath("http://www.slashdot.org") --> "" (it's not a file scheme URI!)
	std::string uriToNativePath(const std::string &uriRef, cdom::systemType type)
	{
		std::string scheme,authority,path,query,fragment;
		cdom::parseUriRef(uriRef,scheme,authority,path,query,fragment);

		//Make sure we have a file scheme URI, or that it doesn't have a scheme
		if(!scheme.empty()&&scheme!="file")
		return "";

		std::string filePath;

		if(type==Windows)
		{
			if(!authority.empty())
			filePath += string("\\\\")+authority; //UNC path

			//Replace two leading slashes with one leading slash, so that
			// ///otherComputer/file.dae becomes //otherComputer/file.dae and
			// //folder/file.dae becomes /folder/file.dae
			if(path.length()>=2&&path[0]=='/'&&path[1]=='/')
			path.erase(0,1);

			//Convert "/C:/" to "C:/"
			if(path.length()>=3&&path[0]=='/'&&path[2]==':')
			path.erase(0,1);

			//Convert forward slashes to back slashes
			path = std::replace(path,"/","\\");
		}

		filePath += path;

		//Replace %20 with space
		filePath = std::replace(filePath,"%20"," ");

		return filePath;
	}
}

//---.
}//<-'

#endif //0

/*C1071*/
