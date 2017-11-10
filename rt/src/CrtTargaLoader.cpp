/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <RT.pch.h> //PCH

#include "CrtTexture.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
	
RT::Texture * /*C4138*//*RT::*/LoadTargaFromMemory(const void *buf, size_t size, RT::Texture *out)
{		
	//2016: Getting these out of "CrtTypes.h"
	daeCTC<CHAR_BIT==8>();
	typedef unsigned char UInt8;
	typedef int Int32;
	typedef unsigned int UInt32;
	typedef unsigned short UInt16;									
	struct //h
	{	
	UInt8 IDLength;
	UInt8 ColormapType;
	UInt8 ImageType;
	UInt8 ColormapSpecification[5];
	UInt16 XOrigin;
	UInt16 YOrigin;
	UInt16 ImageWidth;
	UInt16 ImageHeight;
	UInt8 PixelDepth;
	UInt8 ImageDescriptor;
	}h;

	if(buf==nullptr||size<=sizeof(h)) 
	return nullptr; //LEGACY
		
	memcpy(&h,buf,sizeof(h));
	daeIOController::BigEndian<16>(h.XOrigin);
	daeIOController::BigEndian<16>(h.YOrigin);
	daeIOController::BigEndian<16>(h.ImageWidth);
	daeIOController::BigEndian<16>(h.ImageHeight);
	daeEH::Verbose<<"TGA texture loaded to memory "<<
	h.ImageWidth<<" by "<<h.ImageHeight<<", pixeldepth is "<<h.PixelDepth;
	#ifndef COLLADA_RT_BIG_ENDIAN 
	daeEH::Verbose<<"with no endian swapping.";
	#else
	daeEH::Verbose<<"with big endian swapping.";
	#endif 
	//This swapped the RGB data on PlayStation 3 according to Endianness.
	//But color data isn't words??? And the data is going to OpenGL. Does
	//it need to be GL_BGRA?
	/*REFERENCE
	//2017: This looks like bunk. This can be done below if need be.
	RT::SwapImageData(u8,
	h.PixelDepth*h.ImageWidth*h.ImageHeight/8,
	h.PixelDepth/8);*/
	
	//Only true color, non-mapped or compressed images are supported.
	if(0!=h.ColormapType||h.ImageType!=10&&h.ImageType!=2)
	{
		daeEH::Error<<"TGA texture format isn't supported by the limited built-in loader.\n"
		"(Please convert to 24 or 32 bit no REL compressed Targa.)";		
		return nullptr;
	}

	const int iw = h.ImageWidth;
	const int ih = h.ImageHeight;		
	const bool alpha = 32==h.PixelDepth;	

	//2017: "FORCE_RGB_TO_RGBA" was making glTexImage2D's 
	//4-byte alignment a non-issue.
	int tga_Width = ih*(alpha?4:3)/**(CHAR_BIT/8)*/;
	int gl_Width = tga_Width;
	while(0!=(gl_Width&3)) gl_Width++;

	const UInt8 *p = (UInt8*)buf;
	//2017: Adding bound test incase the file isn't a tga.
	if(sizeof(h)+h.IDLength+tga_Width*ih<=size)
	{
		daeCTC<CHAR_BIT==8>();
		//Skip the ID field. 
		//The first byte of the h is the length of this field.	
		p+=sizeof(h); p+=h.IDLength;
	}
	else return nullptr;

	//2017: There had been a bunch of maddening code here.
	char *d = COLLADA_RT_array_new(char,gl_Width*ih);
	//Flipping the Tga data before passing to OpenGL.
	for(int i=ih;i-->0;p+=tga_Width)
	memcpy(d+i*gl_Width,p,tga_Width);

	if(out==nullptr) out = COLLADA_RT_new(RT::Texture);
	else COLLADA_RT_array_delete(out->Data);
	
	out->Data = d; out->Format = alpha?GL_RGBA:GL_RGB;

	out->Width = iw; out->Height = ih; return out;
}

//-------.
	}//<-'
}

/*C1071*/
