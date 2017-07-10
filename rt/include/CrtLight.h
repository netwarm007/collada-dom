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

	bool IsDirectional(){ return Type==RT::Light_Type::DIRECTIONAL; }

	//Reminder: 0.5 makes 1,1,1,1 models all
	//white/unlit under the default lighting. 
	//0.45 is for DEFAULT. (Kind of a HACK.)	
	Light():Type(RT::Light_Type::DEFAULT),Color(0.45,1)
	,FalloffAngle(180),FalloffExponent()
	,ConstantAttenuation(1),LinearAttenuation(),QuadraticAttenuation()
	{}
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__LIGHT_H__
/*C1071*/
