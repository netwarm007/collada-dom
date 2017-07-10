/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_FX__PCH_H__
#define __COLLADA_FX__PCH_H__

#include <math.h>
#include <float.h>
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

//This keeps some gl.h from including a
//glext.h file until the versions can be
//#undef below.
//glext.h doesn't define the GLtype types
//so it cannot be included before gl.h is.
#define GL_GLEXT_LEGACY

//2017: Trying to simplify Cg/OpenGL set up.
#include <Cg/cgGL.h>
#ifdef SN_TARGET_PS3
//Can't say what these all do without a PS3.
#include <PSGL/psgl.h>
#include <GLES/gl.h>
#endif

#ifndef _WIN32
#define GL_GLEXT_PROTOTYPES
#endif
//The Cygwin gl.h header is missing most of
//the function prototypes. If these macros
//are set glext.h doesn't emit their APIs.
#pragma push_macro("GL_VERSION_1_2")
#pragma push_macro("GL_VERSION_1_3")
#undef GL_VERSION_1_2
#undef GL_VERSION_1_3 //And possibly more?
#ifdef SN_TARGET_PS3
#include <GLES/glext.h> //Or GLES_VERSION?
#elif defined(__APPLE__)
#include <OpenGL/glext.h>
#else
#include <GL/glext.h>
#endif
#pragma pop_macro("GL_VERSION_1_2")
#pragma pop_macro("GL_VERSION_1_3")

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
   
#define COLLADA_DOM_LITE
#ifdef PRECOMPILING_COLLADA_FX
#if COLLADA_DOM_GENERATION!=1
#error The below inclusion guards expect COLLADA_DOM_GENERATION to be equal to 1.
#endif
//Blocking off to shave off of dog-slow *Nix world build-times/document support.
//#define __profile_GLES_h__http_www_collada_org_2005_11_COLLADASchema__ColladaDOM_g1__
//#define __profile_GLES_type_h__http_www_collada_org_2008_03_COLLADASchema__ColladaDOM_g1__
#define __profile_GLES2_type_h__http_www_collada_org_2008_03_COLLADASchema__ColladaDOM_g1__
//These can do (x(y)) but GCC won't accept degenerate token-pasting.
#include COLLADA_(http_www_collada_org_2005_11_COLLADASchema)(effect)
#include COLLADA_(http_www_collada_org_2005_11_COLLADASchema)(material)
#include COLLADA_(http_www_collada_org_2008_03_COLLADASchema)(effect_type)
#include COLLADA_(http_www_collada_org_2008_03_COLLADASchema)(image_type)
#include COLLADA_(http_www_collada_org_2008_03_COLLADASchema)(material_type)
#else 
COLLADA_(namespace)
{
	//FX::Collada05
	COLLADA_(http_www_collada_org_2005_11_COLLADASchema,namespace){}
	COLLADA_(http_www_collada_org_2008_03_COLLADASchema,namespace){}
	namespace DAEP
	{
	COLLADA_(http_www_collada_org_2005_11_COLLADASchema,namespace){}
	COLLADA_(http_www_collada_org_2008_03_COLLADASchema,namespace){}
	}
}
//FX::ColorSpace
#include COLLADA_(http_www_collada_org_2005_11_COLLADASchema)(http_www_collada_org_2005_11_COLLADASchema)
//__NB__ in CrtRender.h (it could be omitted, but may as well do this for both schemas.)
#include COLLADA_(http_www_collada_org_2008_03_COLLADASchema)(http_www_collada_org_2008_03_COLLADASchema)
#endif
#undef COLLADA_DOM_LITE

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
			#ifndef GL_GLEXT_PROTOTYPES		
			F ptr;
			Ext():ptr(def()){}			
			inline operator F(){ return ptr; }
			#else
			inline operator F(){ return def(); }
			#endif
		};		
		#ifdef GL_GLEXT_PROTOTYPES		
		#define __(t,u) \
		static u _##t(){ return gl##t; }
		#elif defined(_WIN32)
		#define __(t,u) \
		static u _##t(){ return (u)wglGetProcAddress("gl"#t); }
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
