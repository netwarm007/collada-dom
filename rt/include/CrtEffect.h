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
	/**
	 * This is a single texture for legacy/FX-free rendering.
	 */
	GLuint Mono_TexId;

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
	
	/**
	 * The defaults are for if profile_COMMON is unavailable
	 * or the lighting is not so good or there's no material.
	 */
	Material_Data(float def=0):Mono_TexId()
	,Emission(def*0.08,1),Ambient(def*0.42,1),Diffuse(def*0.42,1),Specular(def*0.72,1)
	,Shininess(def*40),Transparency(1),Reflectivity(),RefractiveIndex()
	{}	
};

struct Effect_Semantics
{
	//This list is incomplete.
	unsigned Missing_FX:1,
	WORLD:1,
	WORLD_INVERSE_TRANSPOSE:1,
	WORLD_VIEW:1,
	WORLD_VIEW_INVERSE_TRANSPOSE:1,
	WORLD_VIEW_PROJECTION:1;
	void Set(FX::Semantic s)
	{
		switch(s)
		{
		#define _(x) case FX::x: x = 1; break;
		_(WORLD)
		_(WORLD_INVERSE_TRANSPOSE)
		_(WORLD_VIEW)
		_(WORLD_VIEW_INVERSE_TRANSPOSE)
		_(WORLD_VIEW_PROJECTION)
		#undef _
		default:; //-Wswitch
		}
	}
	
	Effect_Semantics(){ Semantics() = 0; }

	unsigned &Semantics(){ return *(unsigned*)this; }
};

struct Effect_Type
{
	enum{ DEFAULT=0,CONSTANT,LAMBERT,PHONG,BLINN };
};

/**
 * This is a shell definition of an effect for now
 * just to resolve the new binding for the 1.4 specifictaion
 * between the imgaes, effects, and materials 
 */
class Effect : public RT::Base
,
public RT::Material_Data, public RT::Effect_Semantics
{	
COLLADA_(public)

	using Material_Data::operator=;
	
	FX::Effect *FX; int Type;
					  	
	Effect(float def=0):Material_Data(def)
	,FX(),Type(def==0?RT::Effect_Type::DEFAULT:RT::Effect_Type::BLINN)
	{ 
		Missing_FX = 1; 
	}
	~Effect(){ delete FX; } 
};

class Material : public RT::Base, public RT::Material_Data
{
COLLADA_(public)

	using Material_Data::operator=;
		
	RT::Effect *Effect; FX::Material *FX;

	void Default();

	Material(float def=0):Material_Data(def),Effect(),FX()
	{}
	Material(RT::Effect *e):Material_Data(*e),Effect(e),FX()
	{
		//If profile_COMMON is not defined, give it some 
		//material properties so it does not appear black.
		if(e->Type==RT::Effect_Type::DEFAULT) 
		new((RT::Material_Data*)this) RT::Material_Data(1);
	}

	//SCHEDULED FOR REMOVAL
	~Material(){ delete FX; }
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__EFFECT_H__
/*C1071*/
