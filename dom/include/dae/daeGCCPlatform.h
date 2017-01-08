/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_GCC_PLATFORM_H__
#define __COLLADA_DOM__DAE_GCC_PLATFORM_H__
		 								
#define COLLADA_DOM_EXPORT __attribute__((visibility("default")))
#define COLLADA_DOM_IMPORT __attribute__((visibility("default")))

#define COLLADA_ALIGN(X) __attribute__((aligned(X)))

/** Ask politley for a CALL instruction. */
#define COLLADA_NOINLINE __attribute__((noinline))

#define COLLADA_DEPRECATED(hint) __attribute__((deprecated(hint)))

#endif //__COLLADA_DOM__DAE_GCC_PLATFORM_H__
/*C1071*/