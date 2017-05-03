/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__PCH_H__
#define __COLLADA_RT__PCH_H__

#include <assert.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>
#include <set>
#include <map>
#include <vector>

#ifdef SN_TARGET_PS3
#include <sys/raw_spu.h>
#include <sys/sys_time.h>
#elif !defined(_WIN32)
#include <sys/time.h>
#endif

//NOTE: devIL is an awful library
//without meaningful documentation.
//That said, it lines up with OpenGL,
//-so you can feel your way around it.
#ifndef NO_DEVIL
#include <IL/il.h>
#endif

//Not including in this revision.
#if 0
//EXPERIMENTAL
//lab, lab::vector & lab::matrix
//This is a new 3-D math library.
//It appears as lab in the files.
#include <ColladaLAB.inl>
#endif

#include "cfxLoader.h"
#if COLLADA_DOM!=3
#error This is relying cfxLoader.h to set things up.
#endif
#define COLLADA_DOM_LITE
#include COLLADA_(http_www_collada_org_2005_11_COLLADASchema,(COLLADA))
#undef COLLADA_DOM_LITE

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
	}
}

#endif //__COLLADA_RT__PCH_H__
/*C1071*/