	
/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_RAII_HPP__
#define __COLLADA_DOM__DAE_RAII_HPP__

//This header is not included in ColladaDOM.inl.
//It houses small, informal RAII helper classes.

#include "daeIOPluginCommon.h"

COLLADA_(namespace)
{
	namespace daeRAII
	{//-.
//<-----'

template<class T> struct Reset
{
	T &t; T reset;

	Reset(T &t):t(t){} ~Reset(){ t = reset; } 	
};

struct UnplugIO
{
	daePlatform &p; daeIOPlugin *IO;

	daeIOPlugin *operator=(daeIOPlugin *io)
	{
		assert(IO==nullptr); return IO = io; 
	}

	UnplugIO(daePlatform &p):p(p),IO(nullptr){}

	~UnplugIO(){ if(IO!=nullptr) p.unplugIO(IO); } 	
};

struct CloseIO
{
	daePlatform &p; daeIO *IO;

	daeIO *operator=(daeIO *io)
	{
		assert(IO==nullptr);
		if((IO=io)==nullptr)
		daeEH::Error<<"No platform provided daeIO.\n""Nor built-in I/O.";
		return io;
	}

	CloseIO(daePlatform &p):p(p),IO(nullptr){}

	~CloseIO(){ if(IO!=nullptr) p.closeIO(IO); } 	
};

struct InstantLegacyIO
{
	bool instantiated;
	char IO[sizeof(daeLegacyIOPlugins)];

	InstantLegacyIO():instantiated(){}

	//Suggestive of a delayed constructor.
	inline daeIOPlugin &operator()(int legacy, const daeURI &URI)
	{
		assert(!instantiated); instantiated = true;
		return *new(IO) daeLegacyIOPlugins(legacy,URI);
	}

	~InstantLegacyIO()
	{
		if(instantiated) ((daeIOPlugin&)*IO).~daeIOPlugin(); 
	} 	
};

//-------.
	}//<-'
}

#endif //__COLLADA_DOM__DAE_RAII_HPP__
/*C1071*/