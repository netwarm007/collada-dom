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
		
	FX::Float4 Emission, Ambient, Diffuse;
	FX::Float4 Specular, Reflective, Transparent;
	float Shininess, Transparency, Reflectivity, RefractiveIndex;

	Material_Data():Shininess(40),Transparency(1)
	,Reflectivity(),RefractiveIndex()
	//These are likely just arbitrary/historical defaults.
	,Emission(0,1),Ambient(0.25f,1),Diffuse(1),Specular(0.95f,1){}
	~Material_Data(){}
};
class Material : public RT::Base, public RT::Material_Data
{
COLLADA_(public)

	using Material_Data::operator=;
		  
	RT::Effect *Effect;	

	FX::Material *COLLADA_FX;

	Material():Effect(),COLLADA_FX(){}
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
	
	FX::Effect *COLLADA_FX; Effect():COLLADA_FX(){}
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__EFFECT_H__
/*C1071*/