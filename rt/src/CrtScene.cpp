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

void RT::DBase::LoadCOLLADA(Collada05::const_COLLADA COLLADA)
{	
	RT::Asset.Meter = COLLADA->asset->unit->meter->*RT::Float(1);
	RT::Asset.Up = COLLADA->asset->up_axis->value->*RT::Up::Y_UP;
	switch(RT::Asset.Up)
	{
	case RT::Up::X_UP: 				
	daeEH::Verbose<<"X"<<"-axis is Up axis!"; break;
	case RT::Up::Y_UP:
	daeEH::Verbose<<"Y"<<"-axis is Up axis!"; break;
	case RT::Up::Z_UP:
	daeEH::Verbose<<"Z"<<"-axis is Up axis!"; break;
	}			

	RT::Main.COLLADA_FX.Cg = RT::Main.Cg.Context;
	{
		//This preloads some libraries. It's unsaid why.
		//FOR 1) Animations need to be pre-loaded to link 
		//them. Animations don't seem to have dependencies. 
		//for(size_t i=0;i<COLLADA->library_images.size();i++)
		//if(RT::Main.LoadImages)
		//LoadLibrary("Image",COLLADA->library_images[i]->image,&DBase::LoadImage);	
		//for(size_t i=0;i<COLLADA->library_effects.size();i++)
		//LoadLibrary("Effect",COLLADA->library_effects[i]->effect,&DBase::LoadEffect);
		//for(size_t i=0;i<COLLADA->library_materials.size();i++)
		//LoadLibrary("Material",COLLADA->library_materials[i]->material,&DBase::LoadMaterial);
		if(RT::Main.LoadAnimations)
		for(size_t i=0;i<COLLADA->library_animations.size();i++)
		LoadLibrary("Animation",COLLADA->library_animations[i]->animation,&DBase::LoadAnimation);

		//This had loaded the <instance_visual_scene> but COLLADA only 
		//allows for one such scene that is also associated with physics
		//and so on, so this just loads the main scene. The end-user would
		//have to manually mix and match physics, etc. to select differently.
		LoadScene(COLLADA->scene);
	}
	RT::Main.COLLADA_FX.Cg = nullptr;
}

//-------.
	}//<-'
}

/*C1071*/