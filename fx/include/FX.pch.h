/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_FX__PCH_H__
#define __COLLADA_FX__PCH_H__

#include <vector>

#ifdef _WIN32 
#define NOMINMAX
//glu.h wanted APIENTRY defined.
#include <windows.h> 
//These conflict with ColladaDOM/the schemas.
#undef VOID
#undef RELATIVE
#undef CONST
#undef RGB
#endif

//2017: Trying to simplify Cg/OpenGL set up.
#include <Cg/cgGL.h> 
#ifdef SN_TARGET_PS3
//Can't say what these all do without a PS3.
#define GL_GLEXT_PROTOTYPES
#include <PSGL/psgl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#elif defined(__APPLE__)
#define GL_GLEXT_PROTOTYPES
#include <OpenGL/glext.h>
#else
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
#include "../../xmlns/http_www_collada_org_2008_03_COLLADASchema/config.h"
   
#ifdef PRECOMPILING_COLLADA_FX
#if COLLADA_DOM_GENERATION!=1
#error The below inclusion guards expect COLLADA_DOM_GENERATION to be equal to 1.
#endif
#define COLLADA_DOM_LITE
#include COLLADA_(http_www_collada_org_2005_11_COLLADASchema,(effect))
#include COLLADA_(http_www_collada_org_2005_11_COLLADASchema,(material))
#include COLLADA_(http_www_collada_org_2008_03_COLLADASchema,(effect_type))
#include COLLADA_(http_www_collada_org_2008_03_COLLADASchema,(image_type))
#include COLLADA_(http_www_collada_org_2008_03_COLLADASchema,(material_type))
#undef COLLADA_DOM_LITE
#else 
COLLADA_(namespace)
{
	COLLADA_(http_www_collada_org_2005_11_COLLADASchema,namespace){}
	COLLADA_(http_www_collada_org_2008_03_COLLADASchema,namespace){}
	namespace DAEP
	{
	COLLADA_(http_www_collada_org_2005_11_COLLADASchema,namespace){}
	COLLADA_(http_www_collada_org_2008_03_COLLADASchema,namespace){}
	}
}
#endif

COLLADA_(namespace)
{
	namespace FX
	{
		namespace xs = ::COLLADA::DAEP::xs;
		namespace Collada05_XSD = ::COLLADA::http_www_collada_org_2005_11_COLLADASchema;
		namespace Collada05 = ::COLLADA::DAEP::http_www_collada_org_2005_11_COLLADASchema;
		namespace Collada08_XSD = ::COLLADA::http_www_collada_org_2008_03_COLLADASchema;
		namespace Collada08 = ::COLLADA::DAEP::http_www_collada_org_2008_03_COLLADASchema;
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