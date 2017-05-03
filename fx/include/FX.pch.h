/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_FX__PCH_H__
#define __COLLADA_FX__PCH_H__

//These shouldn't be necessary.
//#include <assert.h>
//#include <stdio.h>
//#include <stdarg.h>
#include <stdlib.h>
//#include <string.h>
#include <math.h>
//#include <cstdlib>
//#include <iostream>
//#include <string>
#include <vector>

#ifdef _WIN32 
#define NOMINMAX
//#define WIN32_LEAN_AND_MEAN	
//glu.h wants APIENTRY defined.
#include <windows.h> 
//These conflict with ColladaDOM/the schemas.
#undef VOID
#undef RELATIVE
#undef CONST
#undef RGB
#endif

//2017: Trying to simplify Cg/OpenGL set up.

#include <Cg/cgGL.h> 
#ifdef NDEBUG
#error Remove GLU (gluPerspective/gluLookat)
#endif
#ifdef SN_TARGET_PS3
//Can't say what these all do without a PS3.
#define GL_GLEXT_PROTOTYPES
#include <PSGL/psgl.h>
#include <PSGL/psglu.h> //Can do without.
#include <GLES/gl.h>
#include <GLES/glext.h>
#elif defined(__APPLE__)
#define GL_GLEXT_PROTOTYPES
#include <OpenGL/glu.h> //Can do without.
#include <OpenGL/glext.h>
#else
#include <gl/glu.h> //Can do without.
#include <gl/glext.h>
#endif

//SCHEDULED FOR REMOVAL
//This is because the old Bullet-Physics library is compiled to use float.
#ifndef COLLADA_DOM_DOUBLE
#define COLLADA_DOM_DOUBLE float
#endif
#ifndef BUILDING_COLLADA_DOM
#define IMPORTING_COLLADA_DOM
#endif

#ifndef COLLADA_DOM
#define COLLADA_DOM 3
#endif
#define COLLADA_NOLEGACY
#include <ColladaDOM.inl>
#include "../../xmlns/http_www_collada_org_2005_11_COLLADASchema/config.h"

#if COLLADA_DOM_GENERATION!=1
#error The below inclusion guards expect COLLADA_DOM_GENERATION to be equal to 1.
#endif
#ifndef __COLLADA_h__http_www_collada_org_2005_11_COLLADASchema__ColladaDOM_g1__
//These lines suppress the inclusion of these rather large chunks of this schema.
//If including code uses them it must include them before this header is included.
#ifndef PRECOMPILING_COLLADA_FX
#define __profile_CG_h__http_www_collada_org_2005_11_COLLADASchema__ColladaDOM_g1__
#define __profile_GLSL_h__http_www_collada_org_2005_11_COLLADASchema__ColladaDOM_g1__
#define __profile_GLES_h__http_www_collada_org_2005_11_COLLADASchema__ColladaDOM_g1__
#endif
#define COLLADA_DOM_LITE
#include COLLADA_(http_www_collada_org_2005_11_COLLADASchema,(effect))
#include COLLADA_(http_www_collada_org_2005_11_COLLADASchema,(material))
#undef COLLADA_DOM_LITE
#endif

COLLADA_(namespace)
{
	namespace FX
	{
		namespace xs = ::COLLADA::DAEP::xs;
		namespace Collada05_XSD = ::COLLADA::http_www_collada_org_2005_11_COLLADASchema;
		namespace Collada05 = ::COLLADA::DAEP::http_www_collada_org_2005_11_COLLADASchema;
	}

	//EXPERIMENTAL (OpenGL is such a PITA.)
	extern struct GL
	{
		template<class F, F def()> struct Ext
		{
			F ptr;
			Ext():ptr(def()){}
			template<class S>
			void operator()(S x){ if(ptr!=nullptr) ptr(x); }
			template<class S, class T>
			void operator()(S x, T y){ if(ptr!=nullptr) ptr(x,y); }			
		};		
		#ifdef GL_GLEXT_PROTOTYPES		
		#define __(x,y) \
		static y _##x(){ return gl##x; }
		#elif defined(_WIN32)
		#define __(x,y) \
		static y _##x(){ return (y)wglGetProcAddress("gl"#x); } 
		#else
		//#error Is your system unrepresented?
		#endif		
		#define _(x,y,z) __(y,z) Ext<z,_##y> y;
		_(void,ActiveTexture,PFNGLACTIVETEXTUREPROC)
		_(void,ClientActiveTexture,PFNGLCLIENTACTIVETEXTUREPROC)
		_(void,GenerateMipmap,PFNGLGENERATEMIPMAPPROC)
		#ifdef COLLADA_GL_INCLUDE
		#include COLLADA_GL_INCLUDE
		#endif
		#undef __
		#undef _

		static void LoadMatrix(const float m[16]){ glLoadMatrixf(m); }
		static void LoadMatrix(const double m[16]){ glLoadMatrixd(m); }
		static void MultMatrix(const float m[16]){ glMultMatrixf(m); }
		static void MultMatrix(const double m[16]){ glMultMatrixd(m); }

	}GL; //See CrtRender.cpp
}

#endif //__COLLADA_FX__PCH_H__
/*C1071*/