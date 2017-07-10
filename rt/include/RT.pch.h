/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__PCH_H__
#define __COLLADA_RT__PCH_H__

#ifdef PRECOMPILING_COLLADA_RT
//NOTE: devIL is an awful library
//without meaningful documentation.
//That said, it lines up with OpenGL,
//-so you can feel your way around it.
#ifndef NO_IL
#include <IL/il.h>
#endif
#ifdef SN_TARGET_PS3
#include <sys/raw_spu.h>
#include <sys/sys_time.h>
#elif !defined(_WIN32)
#include <sys/time.h>
#endif
#endif //PRECOMPILING_COLLADA_RT

//Not including in this revision.
#if 0
//EXPERIMENTAL
//lab, lab::vector & lab::matrix
//This is a new 3-D math library.
//It appears as lab in the files.
#include <ColladaLAB.inl>
#endif

//HACK: cfxLoader.h defines stuff this file would
//have to otherwise.
#include "../../fx/include/cfxLoader.h"
#include "../../fx/include/cfxData.h"
#if COLLADA_DOM!=3
#error This is relying cfxLoader.h to set things up.
#endif
#ifdef PRECOMPILING_COLLADA_RT
#if COLLADA_DOM_GENERATION!=1
#error The below inclusion guards expect COLLADA_DOM_GENERATION to be equal to 1.
#endif
//These lines suppress the inclusion of these rather large chunks of this schema.
//If including code uses them it must include them before this header is included.
#define __profile_CG_h__http_www_collada_org_2005_11_COLLADASchema__ColladaDOM_g1__
#define __profile_GLSL_h__http_www_collada_org_2005_11_COLLADASchema__ColladaDOM_g1__
#define __profile_GLES_h__http_www_collada_org_2005_11_COLLADASchema__ColladaDOM_g1__
#define __profile_CG_type_h__http_www_collada_org_2008_03_COLLADASchema__ColladaDOM_g1__
#define __profile_GLSL_type_h__http_www_collada_org_2008_03_COLLADASchema__ColladaDOM_g1__
#define __profile_GLES_type_h__http_www_collada_org_2008_03_COLLADASchema__ColladaDOM_g1__
#define __profile_GLES2_type_h__http_www_collada_org_2008_03_COLLADASchema__ColladaDOM_g1__
//Blocking off some more to shave off of dog-slow *Nix world build-times/document support.
#define __brep_type_h__http_www_collada_org_2008_03_COLLADASchema__ColladaDOM_g1__
#define __library_formulas_type_h__http_www_collada_org_2008_03_COLLADASchema__ColladaDOM_g1__
#define __library_kinematics_scenes_type_h__http_www_collada_org_2008_03_COLLADASchema__ColladaDOM_g1__
#define __library_kinematics_models_type_h__http_www_collada_org_2008_03_COLLADASchema__ColladaDOM_g1__
#define COLLADA_DOM_LITE
//These can do (x(y)) but GCC won't accept degenerate token-pasting.
#include COLLADA_(http_www_collada_org_2005_11_COLLADASchema)(COLLADA)
#include COLLADA_(http_www_collada_org_2008_03_COLLADASchema)(COLLADA)
#undef COLLADA_DOM_LITE
#endif //PRECOMPILING_COLLADA_RT

COLLADA_(namespace)
{
	namespace RT
	{	
		#ifdef __COLLADA_LAB_INL__ //Not including in this revision.
		typedef COLLADA::LAB lab;
		#endif
		namespace xs = ::COLLADA::DAEP::xs;
		namespace Collada05_XSD = ::COLLADA::http_www_collada_org_2005_11_COLLADASchema;
		namespace Collada05 = ::COLLADA::DAEP::http_www_collada_org_2005_11_COLLADASchema;
		namespace Collada08_XSD = ::COLLADA::http_www_collada_org_2008_03_COLLADASchema;
		namespace Collada08 = ::COLLADA::DAEP::http_www_collada_org_2008_03_COLLADASchema;
	}
}

#endif //__COLLADA_RT__PCH_H__
/*C1071*/
