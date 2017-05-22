/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__EFFECT_H__
#define __COLLADA_RT__EFFECT_H__ 

#include "CrtData.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'

#ifdef NDEBUG
#error This requires work. This is just a simplification.
#endif
/**
 * This is the old packages crude representation of <setparam>.
 * It seems like the CrtMaterial class hadn't actually used it.
 */
struct Material_Data
{
	//this is part of the Phong lighting definition 
	//and will override the FF material setting  
	//Again this is a first implementation and should
	//be fleshed out further later. 
		
	#ifdef NDEBUG
	#error Transparency/Transparent should be ignored if 0. (Unless animated?)
	#endif
	//https://forums.khronos.org/showthread.php/11108-Transparent?p=36874#post36874
	FX::Float4 Emission, Ambient, Diffuse;
	FX::Float4 Specular, Reflective, Transparent;
	float Shininess, Transparency, Reflectivity, RefractiveIndex;

	//These are some historically defined defaults.
	//Note that by default they are overriden by 0
	//so that they can be filled out in <material>.
	Material_Data(float def=0):Shininess(def*40),Transparency(1)
	,Reflectivity(),RefractiveIndex()
	,Emission(0,1),Ambient(def*0.25,1),Diffuse(def*0.5,1),Specular(def*0.95,1)
	{}	
};

struct Effect_Type
{
	enum{ FX=0,CONSTANT,LAMBERT,PHONG,BLINN };
};

/**
 * This is a shell definition of an effect for now
 * just to resolve the new binding for the 1.4 specifictaion
 * between the imgaes, effects, and materials 
 */
class Effect : public RT::Base, public RT::Material_Data
{	
COLLADA_(public)

	using Material_Data::operator=;

	std::vector<RT::Image*> Textures;	
	
	int Type; FX::Effect *COLLADA_FX; 
	
	Effect(float def=0):Material_Data(def)	
	,Type(RT::Effect_Type::PHONG),COLLADA_FX()
	{}
};

class Material : public RT::Base, public RT::Material_Data
{
COLLADA_(public)

	using Material_Data::operator=;
		
	FX::Material *COLLADA_FX;

	RT::Effect *Effect;	

	Material(float def=0):Material_Data(def),Effect(),COLLADA_FX()
	{}
	Material(RT::Effect *e):Material_Data(*e),Effect(e),COLLADA_FX()
	{}
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__EFFECT_H__
/*C1071*/