/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <RT.pch.h> //PCH

#include "CrtRender.h"
#include "CrtLight.h"
#include "CrtEffect.h"
#include "CrtTexture.h"
#include "CrtGeometry.h"
#include "CrtAnimation.h"
#include "CrtScene.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
	
#define _(x,y) \
std::for_each(y.begin(),y.end(),COLLADA_RT_delete<x>); y.clear();
RT::DBase::~DBase()
{	
	_(RT::Node,Nodes)
	_(RT::Light,Lights)
	_(RT::Camera,Cameras)
	_(RT::Material,Materials)
	_(RT::Image,Images)
	_(RT::Effect,Effects)
	_(RT::Controller,Controllers)
	_(RT::Animation,Animations)
	_(RT::Geometry,Geometries)
	_(RT::PhysicsModel,PhysicsModels)	
}
RT::Node::~Node()
{	
	_(RT::Geometry_Instance,Geometries)
	_(RT::Controller_Instance,Controllers)
}
#undef _

bool RT::DBase::LoadCOLLADA_1_4_1(Collada05::const_COLLADA COLLADA)
{	
	if(COLLADA==nullptr) return false;
	//Reminder: There might be multiple documents involved.
	daeEH::Verbose<<"COLLADA version line is 1.4.1 (2005)";

	//EXPERIMENTAL
	daeName keywords = COLLADA->asset->keywords->value->*daeName();
	while(!keywords.empty())
	RT::Main.COLLADA_index_keywords.push_back(keywords.pop_first_word());

	LoadCOLLADA(COLLADA); return true;
}
bool RT::DBase::LoadCOLLADA_1_5_0(Collada08::const_COLLADA COLLADA)
{	
	if(COLLADA==nullptr) return false;
	//Reminder: There might be multiple documents involved.
	daeEH::Verbose<<"COLLADA version line is 1.5.0 (2008)";

	//EXPERIMENTAL
	daeName keywords = COLLADA->asset->keywords->value->*daeName();
	while(!keywords.empty())
	RT::Main.COLLADA_index_keywords.push_back(keywords.pop_first_word());

	LoadCOLLADA(COLLADA); return true;
}

//-------.
	}//<-'
}

/*C1071*/