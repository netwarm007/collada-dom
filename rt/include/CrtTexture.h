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
	const void *Data; FX::ColorSpace sRGB;

	Texture():Width(),Height(),Format(),Data(),sRGB(){}

	~Texture(){ COLLADA_RT_array_delete(Data); }

	/**
	 * Set @c Data via @a URL. This is provided to nonvisual applications.
	 * Normally @c RT::Texture::Refresh() deletes @c Data. @c Load() sets
	 * it so it can be used to save a texture to a file.
	 * @see @c RT::Image::Load()
	 */
	bool Load(const daeIORequest &URL);

	/**COURTESY
	 */
	void Unload()
	{
		Width = Height = 0; COLLADA_RT_array_delete(Data);
	}
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
	 * This loads an embedded image. E.g. <hex> images.
	 */
	void Init(RT::Name format, const daeBinary<> &image)
	{
		//Just trying to build right now.
		#ifdef NDEBUG
		#error Implement this via DevIL.
		#endif
		daeEH::Error<<"Failed to initialize <hex> image because feature is not implemented. Sorry.";
	}

	/**
	 * Calls @c RT::Texture::Load() with this image's URL.
	 */
	bool Load();

	/**
	 * This will load the image from DocURI+URL.
	 * @note @c Refresh() generates an OpenGL texture ID and 
	 * updates its server side memory, and then deletes @c Data.
	 * Alternatively, @c Load() retains the Data and does not
	 * change the OpenGL memory. Refresh allocates a texture ID
	 * even if the OpenGL texture is not set up.
	 * @see @c RT::Frame::LoadImages.
	 *
	 * @param reload if @c false allows use of @c Load() and
	 * @c Refresh() together. If @c true @c Data is deleted.
	 */
	bool Refresh(bool reload=true);

	/**
	 * This called "CrtRender::DeleteTexture."
	 */
	void DeleteTexture();

	#ifdef PRECOMPILING_COLLADA_RT //INTERNAL
	template<class T> void _SetURL_or_hex_COLLADA_1_5_0(T &init_from)
	{
		if(init_from.empty()) return;

		if(!init_from->hex.empty())
		{
			Collada08::const_image::init_from::hex 
			hex = init_from->hex;
			if(!hex->value->empty())
			{
				if(hex->value->size()>1)
				daeEH::Warning<<"Using only the first data-block in multi-block <hex> image "<<Id;
				Init(hex->format,hex->value[0]);
			}
		}
		else URL = init_from->ref->value->*"";	
	}
	#endif
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
