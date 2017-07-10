/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__SCENE_H__
#define __COLLADA_RT__SCENE_H__  

#include "CrtData.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'

//HACK: This is used to cast 1.5.0 elements to a 1.4.1 
//element type that must be strictly binary compatible.
#define COLLADA_RT_cast(t,e) (*(Collada05::const_##t*)&e)

//SCHEDULED FOR REMOVAL
/**
 * Previously "CrtScene." It will be retooled to integrate
 * with @c daeDB to be a basis for a @c daeDatabase object. 
 */
class DBase
{
COLLADA_(public)

	//SCHEDULED FOR REMOVAL
	std::vector<RT::Node*> Nodes;
	std::vector<RT::Animation*> Animations;
	std::vector<RT::Camera*> Cameras;
	std::vector<RT::Controller*> Controllers;
	std::vector<RT::Effect*> Effects;
	std::vector<RT::Geometry*> Geometries;
	std::vector<RT::Image*> Images;	 
	std::vector<RT::Light*> Lights;
	std::vector<RT::Material*> Materials;
	std::vector<RT::PhysicsModel*> PhysicsModels;

COLLADA_(private) //LoadCOLLADA subroutines

	friend class RT::Frame;
	bool LoadCOLLADA_1_4_1(Collada05::const_COLLADA);
	bool LoadCOLLADA_1_5_0(Collada08::const_COLLADA);
	DBase(){} ~DBase();
				   
	//SCHEDULED FOR REMOVAL
	#define ColladaYY Collada05
	#include "CrtScene08.hpp"
	#undef ColladaYY
	#define ColladaYY Collada08
	#include "CrtScene08.hpp"
	#undef ColladaYY
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__SCENE_H__
/*C1071*/