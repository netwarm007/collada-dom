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
#include "CrtAnimation.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'

#ifdef NDEBUG
#error sizeof(Collada05::const_float_array) is unknown.
#endif
//DEPENDENCY ON "float_array.h"
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

	Geometry_Semantic():Stride(),Offset(),Index(),Dimension(){}
	/**
	 * @c Index is derived from <input offset> although it's
	 * not the same. @c Offset is <accessor offset> verbatim.
	 */
	Collada05::accessor::stride_uint Stride, Offset:8, Index:8;
	/**
	 * @see @c RT::Geometry::Generate_Position(). This might
	 * be handy for textures and arbitrary vertex attributes.
	 */
	Collada05::accessor::stride_uint Dimension:8;	
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

	/**EXPERIMENTAL 
	 * It's trendy for COLLADA resources to try to be
	 * small by not having surface normal information. 
	 */
	bool Missing_Normals; Geometry():Missing_Normals(true){}
	void Generate_Normals();
	/**
	 * This generates 3-D POSITION data from 1 or 2-D.
	 * It must be called before the geometry is ready.
	 * @see @c RT::Spline::Genetrate_Positions() that
	 * is always called if @c this is a @c RT::Spline.
	 */
	void Generate_Positions(RT::Up);

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

COLLADA_(public) //Splines?

	//SCHEDULED FOR REMOVAL
	RT::Spline *ToSpline();
	RT::Spline *AsSpline(){ return (RT::Spline*)this; }
	inline bool IsSpline(){ return ToSpline()!=nullptr; }
};											   
class Spline : public RT::Geometry, public RT::Spline_Length
{
	#ifdef _DEBUG
	std::vector<RT::Float> *__SamplePoints;
	#endif

COLLADA_(public)

	Spline():
	#ifdef _DEBUG
	__SamplePoints((std::vector<RT::Float>*)&SamplePoints),
	#endif
	Open(-1),BSPLINE_Entry()
	{}
	/**
	 * The main reason this is retained (not discarded) is
	 * so the control-points (and graph) can be visualized.
	 * It may also make sense to apply a <skin> or <morph>
	 * to the splines. If so they should be handled by all
	 * new derivations of the @c RT::Controller base class.
	 */
	std::vector<RT::Spline_Point> SamplePoints;

	/**
	 * This is funky. Just do @c Points+Open where it can
	 * be -1 or 0. It wasn't present until BSPLINE had to
	 * have a phantom point added to the end, making this
	 * ambiguous.
	 * @c BSpline is used to check for the existence of a
	 * phantom point in what would be the 0th position if
	 * the curve did not begin with a BSPLINE entry point.
	 */
	signed Open:1; unsigned BSPLINE_Entry:1;

	/**
	 * This will be needed to rebuild the positions array. 
	 */
	RT::Up Sense; void Generate_Positions();
};
inline RT::Spline *RT::Geometry::ToSpline() //CIRCULAR-DEPENDENCY
{
	return dynamic_cast<RT::Spline*>(this); 
}

//-------.
	}//<-'
}

#endif //__COLLADA_RT__GEOMETRY_H__
/*C1071*/
