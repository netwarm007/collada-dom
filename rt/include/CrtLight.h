/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__LIGHT_H__
#define __COLLADA_RT__LIGHT_H__

#include "CrtData.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
	
struct Light_Type
{
	enum
	{
	DEFAULT=0,
	AMBIENT,
	DIRECTIONAL,
	POINT,
	SPOT
	};
};

class Light : public RT::Base
{
COLLADA_(public)
	/**
	 * This is an @c RT::Light_Type @c enum.
	 */
	int Type;

	//4 for glLight??
	FX::Float4 Color;/**< The color of the light (all types)*/
	float FalloffAngle;/**< Falloff angle for spot light (spot) */
	float FalloffExponent;/**< Falloff exponent for spot light (spot) */
	float ConstantAttenuation;/**< Constant attenuation factor (point and spot)*/
	float LinearAttenuation;/**< Linear attenuation factor (point and spot)*/
	float QuadraticAttenuation;/**< Quadratic attenuation factor (point and spot) */

COLLADA_(public)

	Light()
	:Color(0.5f,1)
	//!!!GAC are these good values for defaults?
	,FalloffAngle(180)
	,FalloffExponent()
	,ConstantAttenuation(1)
	,LinearAttenuation()
	,QuadraticAttenuation()
	,Type(RT::Light_Type::DEFAULT){}
	~Light(){}
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__LIGHT_H__
/*C1071*/