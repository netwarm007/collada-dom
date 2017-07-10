/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_GCC_PLATFORM_H__
#define __COLLADA_DOM__DAE_GCC_PLATFORM_H__
		 							
#ifdef __CYGWIN__
//Do all Cygwin apps really do this??
//https://gcc.gnu.org/wiki/Visibility
#define COLLADA_DOM_EXPORT __attribute__((__dllexport__))
#define COLLADA_DOM_IMPORT __attribute__((__dllimport__))
#else
#define COLLADA_DOM_EXPORT __attribute__((__visibility__("default")))
#define COLLADA_DOM_IMPORT __attribute__((__visibility__("default")))
#endif

#define COLLADA_ALIGN(X) __attribute__((__aligned__(X)))

/** Ask politley for a CALL instruction. */
#define COLLADA_NOINLINE __attribute__((__noinline__))

#define COLLADA_DEPRECATED(hint) __attribute__((__deprecated__(hint)))

#endif //__COLLADA_DOM__DAE_GCC_PLATFORM_H__
/*C1071*/
