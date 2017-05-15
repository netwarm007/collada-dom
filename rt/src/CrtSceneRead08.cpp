/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <RT.pch.h> //PCH

#include "CrtTypes.h"
COLLADA_(namespace)
{
	namespace RT
	{	
		#define YY 8 //MSVC2015 wants RT::??
		namespace ColladaYY = RT::Collada08;
		namespace ColladaYY_XSD = Collada08_XSD;
	}
}
#define _if_YY(x,y) y
#include "CrtSceneRead.cpp"

/*C1071*/