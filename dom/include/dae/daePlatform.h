/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_PLATFORM_H__
#define __COLLADA_DOM__DAE_PLATFORM_H__

//Definitions from here down to COLLADA_DOM_INCLUDE
//are made available for use by COLLADA_DOM_INCLUDE.
//
//These macros are meta to the C Preprocessor.
#define COLLADA_TOKENIZE2(x,y) x##y
#define COLLADA_TOKENIZE(x,y) COLLADA_TOKENIZE2(x,y)
#define COLLADA_STRINGIZE2(...) #__VA_ARGS__
#define COLLADA_STRINGIZE(...) COLLADA_STRINGIZE2(__VA_ARGS__)
//
//////This injects a configuration file. It's mainly
	//of use when dependent libraries must all agree.
	//This will require an environment-variable or a
	//configuration script. #if 0 helps in that case.
	#if 0!=COLLADA_DOM_INCLUDE
	#include COLLADA_DOM_INCLUDE
	#endif

//This macro is expected to be 2 or 3. If 3 the old 
//2.x APIs are hidden to users. The runtime package
//defines it to be 3 so the 3 folder is used to get
//its headers. Essentially 2 is backward compatible.
#ifndef COLLADA_DOM
#define COLLADA_DOM 2
#endif

/**
 * @namespace ColladaDOM_3 
 * @c ColladaDOM_3 signifies version 2.5 and later.
 * @c COLLADA is its alias. @c COLLADA_(namespace) 
 * is needed since aliases can't extend namespaces.
 * The linker uses this namespace. @c COLLADA is a
 * logical namespace, so it can be changed without
 * breaking existing code bases.
 */
namespace ColladaDOM_3
{
	//These namespaces are documented below
	//so that the Doxygentation will appear
	//in tooltips in IDEs.  

	/**namespace COLLADA::DAEP
	 *
	 * DAEP is an initialism of:
	 *
	 * Digital-Artifact-Exchange-Protocol.
	 *
	 * Artifact can be shortened to "Art."
	 *
	 * DAEP is an abstract portable API definition.
	 * (This just means that conforming code can be
	 * compiled against a conforming implementation.)
	 *
	 * It's used by COLLADA-DOM to hide implementation
	 * details. Primarily so that generated classes may
	 * use the same names as the XSD files they are from.
	 *
	 * DAEP classes should have data members and virtual
	 * method layouts, and no more (names don't matter.)
	 */
	namespace DAEP{}

	/**namespace COLLADA::XS
	 * 
	 * This namespace contains classes that correspond
	 * to elements in the XML Schema's (XSD) namespace.
	 *
	 * Classes in this namespace are used to configure
	 * the code-generation step's XML element metadata.
	 */
	namespace XS{}
}
/**
 * @namespace COLLADA
 * COLLADA is an alias. It's preferred that
 * clients/users use COLLADA over the exact
 * namespace. Unfortunately C++ forbids the
 * use of aliases as definitions/extensions. 
 */
namespace COLLADA = ColladaDOM_3;
/** @see the @c COLLADA_ preprocessor macro. */
#define COLLADA__namespace__ namespace ColladaDOM_3

/**
 * This relates incompatibility due to opposing objectives.
 * It's kind of a major version. 2 was to imply version 2.x,
 * -but maybe this is not useful.
 *
 * Major philosophies for example, are: multi-threading, with 
 * locking; and non-local C++ exceptions. In practice this has
 * to be a bitwise-combination of interdisciplinary philosophy.
 */
#define COLLADA_DOM_PHILOSOPHY 0 //COLLADA_DOM //2
/**
 * This relates the ABI's version. It's kind of a minor version.
 * 5 was to imply version 2.5. This number is the module version.
 */
#define COLLADA_DOM_PRODUCTION 5

/**2.5 Linkage Macros EXPLAINED*********************************

*COLLADA_DOM_INCLUDE
if defined is the body of a #include directive to be included at
the top of the core library headers. The included header can use
any macros that are defined before it is included.
(THIS IS FOR WHEN LIBRARIES MUST BUILD AGAINST A CONFIGURATION.)

*BUILDING_COLLADA_DOM
includes all of the data, etc. in the build. Whereas
normally parts are not included by clients when including headers.

*EXPORTING_COLLADA_DOM
adds __declspec(dllexport) and permits -fvisibility=hidden on GCC.

*IMPORTING_COLLADA_DOM 
adds __declspec(dllimport) and "noinline". noinline is experimental. 
NOTE: see LINKAGE.HPP and the SNIPPET macro, that distrusts noinline.

*BUILDING_IN_LIBXML
builds in the libxml I/O plugin. (You need one. Unless you have one?)

*BUILDING_IN_TINYXML
builds in the TinyXML I/O plugin. (Histories suggest PS3 required this.)

*COLLADA_DOM_OMIT_ZAE (formerly	BUILDING_IN_MINIZIP)
disables built-in ZAE features.

NOTE: These libraries will be replaced with equivalents once obsolete.
 
***********************/ /**Ancillary Build Macros EXPLAINED**

*COLLADA_NODEPRECATED
hides deprecated features.

*COLLADA_NOLEGACY
hides additional features. (Deprecations are a subset of legacy.)

NOTE: These two are disabled in release builds. Be careful enabling
them where there are other parties' clients in the build environment.

*COLLADA_DOM_TOSLASH 
concerns backslashes (\) in URLs. All clients get the same behavior.

*COLLADA_DOM_INT8,16,32,64,FLOAT,DOUBLE
are really xs:byte, short, int, long, float & double. Clients don't
have to agree, but they probably should. If clients are sharing the
library, and do not agree, then the shared-library should use every
type that the system architecture provides. Problems can arise if a
type that a client uses is not built into the shared-library's code.

*COLLADA_DOM_UNORDERED_MAP
enables the C++11 unordered_map. (C++11 is undetectable.)
Also enables COLLADA_DOM_UNORDERED_SET. It's a master build option.
*COLLADA_DOM_UNORDERED_SET
enables the C++11 unordered_set. (C++11 is undetectable.)
COLLADA_DOM_UNORDERED_MAP enables both.

*COLLADA_DOM_MAP
includes the C++98 map. daeStringMap, etc. require one or the other.
(most likely something in the library will require/enable it anyway.)
*COLLADA_DOM_SET
includes the C++98 set. daeStringSet, etc. require one or the other.
(most likely something in the library will require/enable it anyway.)

 ***********************/

//imply BUILDING_COLLADA_DOM
#ifdef EXPORTING_COLLADA_DOM
#ifndef BUILDING_COLLADA_DOM //suppress warning
#define BUILDING_COLLADA_DOM
#endif
#ifdef IMPORTING_COLLADA_DOM
#error IMPORTING_COLLADA_DOM && EXPORTING_COLLADA_DOM
#endif 
#endif 

//imply COLLADA_NODEPRECATED
#ifdef COLLADA_NOLEGACY
#define COLLADA_NODEPRECATED
#endif
//"Legacy" is old--"deprecated" is old AND funky.
//This check lets these policies be as aggressive 
//as possible, by neglecting the ABI in debug mode.
//If you need debug-and-release to use the same ABI,
//then (if that's even possible) do not define these.
#if defined(NDEBUG) && defined(COLLADA_NODEPRECATED)
#error Releasing with COLLADA_NODEPRECATED (or COLLADA_NOLEGACY)
#endif

//COLLADA DOM 2.5 is system agnostic.
//Backslashes are not compiler things.
//See daeURI_base::toslash in daeURI.h.
#ifndef COLLADA_DOM_TOSLASH
#define COLLADA_DOM_TOSLASH(CP) (CP=='\\')
#endif

//This is detecting compilers. Not platforms.
//#if defined(_WIN32)
#if defined(_MSC_VER) 
//Read this as Visual Studio.
#include "daeWin32Platform.h"
#elif defined(__GNUC__) //These are defined???
//Read this as GNU compilers.
#include "daeGCCPlatform.h"
#else
//GCC forbids ' in #error.
#ifndef COLLADA_DOM_EXPORT //__declspec(dllexport)
#error "exotic platform hasn't #define COLLADA_DOM_EXPORT in daePlatform.h"
#endif
#ifndef COLLADA_DOM_IMPORT //__declspec(dllimport)
#error "exotic platform hasn't #define COLLADA_DOM_IMPORT in daePlatform.h"
#endif
#ifndef COLLADA_DEPRECATED //__declspec(deprecated)
#error "exotic platform hasn't @define COLLADA_DEPRECATED in daePlatform.h"
#endif
#ifndef COLLADA_ALIGN //__declspec(align)
#error "exotic platform hasn't @define COLLADA_ALIGN in daePlatform.h"
#endif
#endif

#ifndef COLLADA_ASSUME //__assume
#define COLLADA_ASSUME(x) assert(x);
#endif
#ifndef COLLADA_NOALIAS //__declspec(noalias)
#define COLLADA_NOALIAS
#endif
#ifndef COLLADA_NOINLINE //__declspec(noinline)
#define COLLADA_NOINLINE
#endif

#ifndef COLLADA_H
#define COLLADA_H(__FILE__)
#endif

#ifdef IMPORTING_COLLADA_DOM
#if defined(NDEBUG) && defined(BUILDING_COLLADA_DOM)
#error Importing with BUILDING_COLLADA_DOM in Release mode
#endif //Clients may want to see hidden data in their debugger.
#define COLLADA_DOM_LINKAGE COLLADA_DOM_IMPORT COLLADA_NOINLINE
#elif defined(EXPORTING_COLLADA_DOM)
#define COLLADA_DOM_LINKAGE COLLADA_DOM_EXPORT
#else 
#define COLLADA_DOM_LINKAGE
#endif

//See LINKAGE.HPP shorthand
#ifdef BUILDING_COLLADA_DOM
//Note {} could be omitted, but writing ({ }) seems busy-work
#define COLLADA_DOM_SNIPPET(...) { __VA_ARGS__ }
#else
#define COLLADA_DOM_SNIPPET(...) ;
#endif

#include <typeinfo> //typeid
#include <stdio.h>
#include <stddef.h> //ptrdiff_t
#include <stdlib.h>
#include <stdarg.h>
#include <iostream>
#include <iomanip>
#include <assert.h>
#include <wchar.h>
#include <limits.h>
#include <string.h>
#include <string>
#include <map> //ref caches
#include <algorithm>

#ifdef BUILDING_COLLADA_DOM
#include <limits>
#include <vector>
#endif //BUILDING_COLLADA_DOM

//Assuming this is set by the CMake build script.
#ifdef COLLADA_DOM_NEED_NULLPTR
#ifndef nullptr
#define nullptr 0 //C++98 support.
#endif
#endif

//daeStringMap will use one or the other or will not be emitted.
#if defined(COLLADA_DOM_UNORDERED_MAP) || defined(BUILDING_COLLADA_DOM) && __cplusplus >= 201103L
#include <unordered_map>
#ifndef COLLADA_DOM_UNORDERED_MAP //suppressing warning (needed if equivalent?)
#define COLLADA_DOM_UNORDERED_MAP
#endif
#ifndef COLLADA_DOM_UNORDERED_SET //go ahead and use it.
#define COLLADA_DOM_UNORDERED_SET
#endif
#elif defined(COLLADA_DOM_MAP) || defined(BUILDING_COLLADA_DOM) 
#include <map>
#ifndef COLLADA_DOM_MAP //suppressing warning (needed if equivalent?)
#define COLLADA_DOM_MAP
#endif
#endif
//SAME DEAL, but for std::set. This is for daeSmallStringTable.
#if defined(COLLADA_DOM_UNORDERED_SET)
#include <unordered_set>
#elif defined(COLLADA_DOM_SET) || defined(BUILDING_COLLADA_DOM) 
#include <set>
#ifndef COLLADA_DOM_SET //suppressing warning (needed if equivalent?)
#define COLLADA_DOM_SET
#endif
#endif

//PHASING OUT? APPARENTLY WRITING () AFTER IS NEEDED.
//(PUTTING THE ARRAY IN THE PARAMETERS DOESN'T HELP.)
//(ADDING () MAY BE "MOST-VEXING-PARSE." CAN'T TELL.)
//This is a non-C++11 alternative to "static_assert".
//Compile-Time-Check with number of letters as assert.
//Replaces "int compile[N]; (void)compile; //quiet the warning"
template<int N> struct daeCTC{ char compile[N>0?1:/*-1*/N-1]; daeCTC(const void*_=0){} };

//Disable access-control in Release builds, or if they are causing problems.
//This is because The C++ Standard allows implementations to reorder layouts.
//This permits librarians to not have to think in terms of an extra dimension!
#if defined(COLLADA_NOACCESS) || defined(NDEBUG)
#define COLLADA__public__ public:
#define COLLADA__private__ public:
#define COLLADA__protected__ public:
#else
#define COLLADA__public__ public:
#define COLLADA__private__ private:
#define COLLADA__protected__ protected:
#endif
#ifdef NDEBUG
#error Setup a helper macro to glue COLLADA_ on last thing.
#endif
//__VA_ARGS__ is added so generated namespace can use this
//macro. MSVC2013's compiler fails those macros if keyword 
//is namespace. It must be a bug. This may not be portable.
//FOR WHATEVER REASON MSVC WILL WORK WHEN ... IS namespace.
//UPDATE: 
//What the compiler is actually doing is converting the ## 
//into a further macro at every step along the way. So the
//COLLADA__namespace__ is produced, and ... is added to it.
//(For the record, Intellisense is busy doing the opposite.)
//https://connect.microsoft.com/VisualStudio/feedback/details/3101668
#define COLLADA_(keyword,...) COLLADA__##keyword##__##__VA_ARGS__

#ifndef COLLADA__extern__
/**GCC STFU
 * This silences warning: initialized and declared 'extern'
 * 'extern' variables should be used sparingly and so need
 * to be marked.
 */
#define COLLADA__extern__
#endif

//SCHEDULED FOR REMOVAL?
#ifndef COLLADA__nullptr__
/**COLLADA_(nullptr)
 * It's not clear if this is a bug or portable, but MSVC2010
 * often runs into problems with the @c nullptr literal that
 * are resolved by replacing it with 0. Perhaps 0 is not the
 * best definition. 
 * Two known problem areas are:
 * 1) Pointer-to-member defaults in template parameter lists.
 * 2) Pointer conversion operators, where there are multiple
 * types of pointers considered. These could be resolved by
 * an explicit @c std::nullptr_t overload/specialization, if
 * it wasn't necessary to support compilers that are missing
 * support for @c nullptr. (Which must "#define nullptr 0.")
 */
#define COLLADA__nullptr__ ((void*)0)
#endif

//The generator will do:
/**warning: multi-line comment [-Wcomment]
 * COLLADA_(http_www_collada_org_2008_03_COLLADASchema,namespace) 
 * The client must have done:
 * #define COLLADA__http_www_collada_org_2008_03_COLLADASchema__namespace \
 * COLLADA_DOM_NICKNAME(COLLADA_1_5_0,http_www_collada_org_2008_03_COLLADASchema)
 */
//Or:
//namespace http_www_collada_org_2008_03_COLLADASchema
#define COLLADA_DOM_NICKNAME(nick,name) \
namespace nick{} namespace name = nick; namespace nick

#ifndef COLLADA_SUPPRESS_C
#define COLLADA_SUPPRESS_C(xxxx) //suppress MSVC++ warning
#endif	

//moving out of compiler specific files
//and enabling the ability for clients to 
//define these ahead of including daeTypes.h
#ifndef COLLADA_DOM_INT64
#if LONG_MAX <= 2147483647L
#define COLLADA_DOM_INT64 long long
#else
#define	COLLADA_DOM_INT64 long
#endif
#endif //end
#ifndef COLLADA_DOM_INT32
#if SHRT_MAX >= 2147483647
#define COLLADA_DOM_INT32 short
#elif INT_MAX >= 2147483647
#define	COLLADA_DOM_INT32 int
#else
#define	COLLADA_DOM_INT32 COLLADA_DOM_INT64
#endif
#endif //end
#ifndef COLLADA_DOM_INT16
#if SHRT_MAX <= 32767
#define COLLADA_DOM_INT16 short
#else
#define	COLLADA_DOM_INT16 COLLADA_DOM_INT32
#endif
#endif //end
#ifndef COLLADA_DOM_INT8
#if CHAR_MAX <= 127
#define COLLADA_DOM_INT8 char
#else
#define	COLLADA_DOM_INT8 COLLADA_DOM_INT16
#endif
#endif //end
#ifndef COLLADA_DOM_FLOAT
#define COLLADA_DOM_FLOAT float
#endif //end
#ifndef COLLADA_DOM_DOUBLE
#define COLLADA_DOM_DOUBLE double
#endif

COLLADA_(namespace)
{
	//THESE FEED INTO daeDomTypes.h.
	//In the past they were used like stdint types.
	//Don't do that.
	//Note: signed char and char are different types.
	typedef COLLADA_DOM_INT8 daeByte; //Was "daeChar."
	typedef COLLADA_DOM_INT16 daeShort;
	typedef COLLADA_DOM_INT32 daeInt;
	typedef COLLADA_DOM_INT64 daeLong;
	typedef unsigned COLLADA_DOM_INT8 daeUByte; //Was "daeUChar."
	typedef unsigned COLLADA_DOM_INT16 daeUShort;
	typedef unsigned COLLADA_DOM_INT32 daeUInt;
	typedef unsigned COLLADA_DOM_INT64 daeULong;
	typedef COLLADA_DOM_FLOAT daeFloat;
	typedef COLLADA_DOM_DOUBLE daeDouble;

	#define COLLADA__float__precision 1
	#define COLLADA__double__precision 2 
	//GCC won't use ##__precision nor )__precision.
	/**C-PREPROCESSOR MACRO
	 * This macro is for external package configuration.
	 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.C++}
	 * #if 2==COLLADA_DOM_PRECISION
	 * #define BT_USE_DOUBLE_PRECISION
	 * #endif 
	 */
	#define COLLADA_DOM_PRECISION \
	COLLADA_TOKENIZE(COLLADA_TOKENIZE(COLLADA__,COLLADA_DOM_DOUBLE),__precision)

	//Check that char is signed and types meet XML Schema widths.
	//daeByte should be signed char if char is an unsigned value.
	static daeCTC<(daeByte)-1==-1> daeByte_check;
	static daeCTC<(daeUByte)-1!=-1> daeUByte_check;
	static daeCTC<sizeof(daeShort)>=16/CHAR_BIT> daeShort_check;
	static daeCTC<sizeof(daeInt)>=32/CHAR_BIT> daeInt_check;
	static daeCTC<sizeof(daeLong)>=64/CHAR_BIT> daeLong_check;
	static daeCTC<sizeof(daeFloat)>=32/CHAR_BIT> daeFloat_check;
	#if 1!=COLLADA_DOM_PRECISION
	static daeCTC<sizeof(daeDouble)>=64/CHAR_BIT> daeDouble_check;
	#endif
	  
	#ifndef COLLADA_UPTR_MAX
	#ifndef SIZE_MAX
	#error Please define COLLADA_UPTR_MAX for 32 or 64bit system.
	#endif
	#define COLLADA_UPTR_MAX SIZE_MAX
	//If this fails, define	COLLADA_UPTR_MAX manually.
	static daeCTC<sizeof(size_t)==sizeof(void*)> dae64_check;
	#endif
}

#endif //__COLLADA_DOM__DAE_PLATFORM_H__
/*C1071*/
