/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#define COLLADA_DOM_MAKER \
"ColladaDOM 3 reference implementation"

//This must match RT & FX.
#define IMPORTING_COLLADA_DOM
//This is because the old Bullet-Physics library is compiled to use float.
#ifndef COLLADA_DOM_DOUBLE
#define COLLADA_DOM_DOUBLE float
#endif
//libbulletmath.lib wants _printf
#if defined(_MSC_VER) && _MSC_VER>=1900 //MSVC2015
#pragma comment(lib,"legacy_stdio_definitions.lib")
#endif

#undef VOID
#define COLLADA_DOM 3
#include <ColladaDOM.inl>
#include "../../xmlns/http_www_collada_org_2005_11_COLLADASchema/config.h"
#undef RELATIVE
#undef CONST
#undef RGB
#define COLLADA__inline__
#include COLLADA_(http_www_collada_org_2005_11_COLLADASchema,(COLLADA))

extern COLLADA::daePShare COLLADA::DOM_process_share = 0;
extern COLLADA::daeClientString ColladaAgent(COLLADA::XS::Schema *xs)
{
	return COLLADA_DOM_MAKER;
}