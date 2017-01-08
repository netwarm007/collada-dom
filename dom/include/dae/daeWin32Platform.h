/*

 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_WIN32_PLATFORM_H__
#define __COLLADA_DOM__DAE_WIN32_PLATFORM_H__
							  
#if _MSC_VER <= 1200
#error Sorry, MSVC6 is too old for COLLADA-DOM 2.5.
typedef int intptr_t;
#endif

#define COLLADA_DOM_EXPORT __declspec(dllexport)
#define COLLADA_DOM_IMPORT __declspec(dllimport)

#define COLLADA_ALIGN(X) __declspec(align(X))

/** Ask politley for a CALL instruction. */
#define COLLADA_NOINLINE __declspec(noinline)

/** Compiler optimization hint. */
#define COLLADA_ASSUME(x) assert(x); __assume(x)

/**WARNING
 * @warning the GCC profile doesn't have this macro setup. if it can do
 * anything meaningful in this vein, then that would be a good addition.
 *
 * In theory this lets compilers hold onto/reorder exported API results. 
 * Anyway, how it's being used, is to mark COLLADA_DOM_LINKAGE APIs that
 * don't really alter anything.
 *
 * @remarks It's very hard to tell what "noalias" is meant to address, but
 * "I've" at least seen instances of it in examples that decorate external
 * declarations and no-argument functions. A document called "Optimization
 * Best Practices" seems more clear than noalias's own document. Except it
 * warns about referencing globals, as if touching them invalidates memory.
 * It doesn't say if "this" or references meet its definition of a pointer.
 */
#define COLLADA_NOALIAS __declspec(noalias)

//let the warnings fly when doing Release builds
#ifdef _DEBUG 
/** Suppresses Visual Studio's warnings. */
#define COLLADA_SUPPRESS_C(xxxx) __pragma(warning(suppress:xxxx))
/** C4351 array 0 initialization going back to MVSC2005. */
__pragma(warning(disable:4351))
//warning C4520: "inherits via dominance" There's no spot fix.
#pragma warning(disable: 4250)
#define COLLADA_DEPRECATED(hint) __declspec(deprecated(hint))
#endif //_DEBUG

#ifndef COLLADA_H
#define COLLADA_H__FILE__(h) #h
#define COLLADA_H(h) __pragma(message("COLLADA-DOM: In " COLLADA_H__FILE__(h) " ..."))
#endif

//Microsoft prefixes these nonstandard APIs
#ifdef BUILDING_COLLADA_DOM
#define COLLADA__itoa__ COLLADA_SUPPRESS_C(4996) _itoa
#endif

/**
 * Define the system fopen hook for @c daeCRT. 
 */
#ifndef COLLADA_DOM_FOPEN
#define COLLADA_DOM_FOPEN(dae_fopen)\
typedef FILE*(*const dae_fopen##_f)(const wchar_t*,const wchar_t*,int);\
static dae_fopen##_f dae_fopen = _wfsopen;
#endif
#ifndef COLLADA_DOM_FILENO
#define COLLADA_DOM_FILENO _fileno
#endif

#endif //__COLLADA_DOM__DAE_WIN32_PLATFORM_H__
/*C1071*/
