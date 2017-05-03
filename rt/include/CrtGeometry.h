/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__GEOMETRY_H__
#define __COLLADA_RT__GEOMETRY_H__  

#include "CrtNode.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'

/**
 * @c Geometry_Elements uses this to fill the array.
 */
struct Geometry_Semantic : Collada05::const_float_array
{
	//using Collada05::const_float_array::operator=;
	//MSVC2015 isn't generating a default operator=.
	//Probably because of the using directive above.
	RT::Geometry_Semantic &operator=(const Geometry_Semantic &cp) 
	{
		this->~Geometry_Semantic();
		new(this) RT::Geometry_Semantic(cp); return *this;
	}
	void operator=(const Collada05_XSD::float_array *WTF) 
	{
		const_float_array::operator=(WTF);
	}

	Geometry_Semantic():Offset(),Stride(),Index(){}
	/**
	 * @c Index is derived from <input offset> although it's
	 * not the same. @c Offset is <accessor offset> verbatim.
	 */
	Collada05::accessor::offset_uint Offset, Stride, Index;	
};

//SCHEDULED FOR REMOVAL (DOESN'T SCALE)
/**
 * @c RT::Geometry_Elements & @c RT::Geometry base class.
 */
struct Geometry_GetFormat
{
	inline int GetFormat()const
	{
		int n = Normals==nullptr?0:1;
		int t0 = TexCoords==nullptr?0:2; return n+t0;
	}
	/**
	 * E.g. NORMAL, TEXCOORD.
	 * These will have to go into @c RT::Geometry if
	 * more semantics are added. 
	 * @see @c Begin() and @c End() (ESPECIALLY WHEN
	 * ADDING TO THIS LIST.)
	 */
	RT::Geometry_Semantic Normals,TexCoords;
	RT::Geometry_Semantic *Begin(){ return &Normals; }
	RT::Geometry_Semantic *End(){ return &TexCoords+1; }	
};

/**
 * These store the element arrays and vertex-buffer
 * views that can index into the <vertices> buffers.
 */
struct Geometry_Elements : RT::Geometry_GetFormat
{	
	//Currently triangles or lines via glDrawArrays.
	#ifdef NDEBUG	
	#error Try fan/stripper using glMultiDrawArrays.
	#endif
	GLenum Mode; size_t Region, Width;
	
	xs::string Material_Instance_Symbol;

	inline bool operator<(const RT::Geometry_Elements &e)const
	{
		//Mode doen't matter, but can't hurt.
		daeOffset m, f = GetFormat()-e.GetFormat();
		m = Material_Instance_Symbol-e.Material_Instance_Symbol;
		return m!=0?m<0:f!=0?f<0:Mode<e.Mode;
	}
};

class Geometry : public RT::Base, public RT::Geometry_GetFormat
{								 
COLLADA_(public)

	RT::RangeFinder SetRange;

	RT::Geometry_Semantic Positions;	

	Collada05::accessor::count_uint Vertices;

	/**
	 * These aren't necessarily for OpenGL's indexing
	 * APIs. But under certain conditions they may be.
	 */
	std::vector<GLuint> ElementBuffer;

	std::vector<RT::Geometry_Elements> Elements;	

COLLADA_(public) //These are for RT::Stack to use. Not clients.
	/**
	 * @c RT::Stack passes VBuffer2's memory pointer when there
	 * are <controller> interpolants. Both buffers are involved.
	 */
	size_t Size_VBuffer(),Fill_VBuffer(float*,xs::string=nullptr);
	size_t OverrideWith_ControllerData(float*,xs::string=nullptr);

	//SCHEDULED FOR REMOVAL
	//This old logic doesn't belong in Geometry/CrtGeometry.cpp.
	void Draw_VBuffer(float*,xs::string);
	void Draw_VBuffer(float*,std::vector<RT::Material_Instance>&);
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__GEOMETRY_H__
/*C1071*/