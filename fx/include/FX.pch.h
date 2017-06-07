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
		template<class R, class F, F def()> struct Ext
		{
			F ptr;
			Ext():ptr(def()){}			
			R operator()(){ return ptr!=nullptr?ptr():(R)0; }
			template<class S>
			R operator()(S s){ return ptr!=nullptr?ptr(s):(R)0; }
			template<class S, class T>
			R operator()(S s, T t){ return ptr!=nullptr?ptr(s,t):(R)0; }
			template<class S, class T, class U>
			R operator()(S s, T t, U u){ return ptr!=nullptr?ptr(s,t,u):(R)0; }
			template<class S, class T, class U, class V>
			R operator()(S s, T t, U u, V v){ return ptr!=nullptr?ptr(s,t,u,v):(R)0; }
			template<class S, class T, class U, class V, class W>
			R operator()(S s, T t, U u, V v, W w){ return ptr!=nullptr?ptr(s,t,u,v,w):(R)0; }
			template<class S,class T,class U,class V,class W,class X,class Y>
			R operator()(S s,T t,U u,V v,W w, X x,Y y){ return ptr!=nullptr?ptr(s,t,u,v,w,x,y):(R)0; }
		};		
		#ifdef GL_GLEXT_PROTOTYPES		
		#define __(s,t) \
		static t _##s(){ return gl##s; }
		#elif defined(_WIN32)
		#define __(s,t) \
		static t _##s(){ return (t)wglGetProcAddress("gl"#s); } 
		#else
		//#error Is your system unrepresented?
		#endif		
		#define _(s,t,u) __(t,u) Ext<s,u,_##t> t;
		_(void,ActiveTexture,PFNGLACTIVETEXTUREPROC)
		_(void,ClientActiveTexture,PFNGLCLIENTACTIVETEXTUREPROC)
		_(void,GenerateMipmap,PFNGLGENERATEMIPMAPPROC)
		_(void,DebugMessageCallbackARB,PFNGLDEBUGMESSAGECALLBACKARBPROC)
		//GLSL
		_(void,AttachShader,PFNGLATTACHSHADERPROC)
		_(void,BindAttribLocation,PFNGLBINDATTRIBLOCATIONPROC)
		_(void,CompileShader,PFNGLCOMPILESHADERPROC)
		_(GLuint,CreateProgram,PFNGLCREATEPROGRAMPROC)
		_(GLuint,CreateShader,PFNGLCREATESHADERPROC)
		_(void,DeleteShader,PFNGLDELETESHADERPROC)
		_(void,DeleteProgram,PFNGLDELETEPROGRAMPROC)
		_(void,GetActiveUniform,PFNGLGETACTIVEUNIFORMPROC)
		_(void,GetAttachedShaders,PFNGLGETATTACHEDSHADERSPROC)
		_(void,GetShaderiv,PFNGLGETSHADERIVPROC)
		_(void,GetShaderInfoLog,PFNGLGETSHADERINFOLOGPROC)
		_(void,GetProgramiv,PFNGLGETPROGRAMIVPROC)
		_(GLint,GetUniformLocation,PFNGLGETUNIFORMLOCATIONPROC)
		_(void,LinkProgram,PFNGLLINKPROGRAMPROC)
		_(void,ShaderSource,PFNGLSHADERSOURCEPROC)
		_(void,UseProgram,PFNGLUSEPROGRAMPROC)
		//GLSL uniform (cfxData.cpp)		
		_(void,Uniform1i,PFNGLUNIFORM1IPROC)
		_(void,Uniform1f,PFNGLUNIFORM1FPROC)		
		_(void,Uniform2f,PFNGLUNIFORM2FPROC)
		_(void,Uniform3f,PFNGLUNIFORM3FPROC)
		_(void,Uniform4f,PFNGLUNIFORM4FPROC)		
		//Cg cannot use these.
		_(void,UniformMatrix2fv,PFNGLUNIFORMMATRIX2FVPROC)		
		_(void,UniformMatrix3fv,PFNGLUNIFORMMATRIX3FVPROC)		
		_(void,UniformMatrix4fv,PFNGLUNIFORMMATRIX4FVPROC)
		//Cg uses these instead.
		_(void,Uniform2fv,PFNGLUNIFORM2FVPROC)		
		_(void,Uniform3fv,PFNGLUNIFORM3FVPROC)		
		_(void,Uniform4fv,PFNGLUNIFORM4FVPROC)		
		//RT client-data support.		
		_(void,Uniform1d,PFNGLUNIFORM1DPROC)
		_(void,UniformMatrix4dv,PFNGLUNIFORMMATRIX4DVPROC)
		_(void,Uniform4dv,PFNGLUNIFORM4DVPROC)		
		#ifdef COLLADA_GL_INCLUDE
		#include COLLADA_GL_INCLUDE
		#endif
		#undef __
		#undef _

		static void LoadMatrix(const float m[16]){ glLoadMatrixf(m); }
		static void LoadMatrix(const double m[16]){ glLoadMatrixd(m); }
		static void MultMatrix(const float m[16]){ glMultMatrixf(m); }
		static void MultMatrix(const double m[16]){ glMultMatrixd(m); }

	}GL;
}

#endif //__COLLADA_FX__PCH_H__
/*C1071*/