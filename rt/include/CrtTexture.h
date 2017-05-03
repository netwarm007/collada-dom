/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__TEXTURE_H__
#define __COLLADA_RT__TEXTURE_H__

#include "CrtData.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
	
//SCHEDULED FOR REMOVAL?
/**
 * Previously "TgaFile."
 */
struct Texture
{
	/**
	 * Format is now GL_RGB, etc.
	 */
	int Width, Height, Format;
	/**2017
	 * @c RT::Image::Refresh() deletes @c Data after calling glTexImage2D.
	 */
	const void *Data;		

	Texture():Data(),Width(),Height(),Format(){}

	~Texture(){ COLLADA_RT_array_delete(Data); }
};

RT::Texture *LoadTargaFromURI(const daeURI &file);
RT::Texture *LoadTargaFromMemory(const void*,size_t,RT::Texture*_=nullptr);

class Image : public RT::Base, public RT::Texture
{
COLLADA_(public)

	xs::string URL;

	GLuint TexId;

COLLADA_(public)

	Image(xs::string URI=nullptr):URL(URI),TexId()
	{}
	~Image(){ DeleteTexture(); }
		
	/**
	 * This will load the image from DocURI+URL.
	 */
	bool Refresh();

	/**
	 * This called "CrtRender::DeleteTexture."
	 */
	void DeleteTexture();
};

//SCHEDULED FOR REMOVAL?
/**
 * Previously "CrtFile."
 */
class Resource
{	
	Resource(const Resource&);

	void operator=(const Resource&);

COLLADA_(public)
	/**
	 * A 0-terminator is always added to data.
	 * @c Data may be binary, but it's type is
	 * @c char* since it's usually textual data.
	 */
	const char *Data; size_t Size;

COLLADA_(public)

	Resource():Data((char*)&Size),Size(){}	
	Resource(const xs::anyURI &URI){ new(this) Resource(); Locate(URI); }
	~Resource(){ Locate(); }

	bool Locate(const xs::anyURI &URI);

	inline void Locate()
	{
		COLLADA_RT_array_delete(Data); new(this) Resource();
	}
};
  
//-------.
	}//<-'
}

#endif //__COLLADA_RT__TEXTURE_H__
/*C1071*/