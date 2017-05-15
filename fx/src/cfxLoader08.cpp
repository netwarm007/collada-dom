/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <FX.pch.h> //PCH

COLLADA_(namespace)
{
	namespace FX
	{	
		#define YY 8 //MSVC2015 wants FX::??
		namespace ColladaYY = FX::Collada08;
		namespace ColladaYY_XSD = Collada08_XSD;
	}
}
#define _if_YY(x,y) y
#include "cfxLoader.cpp"

/*C1071*/