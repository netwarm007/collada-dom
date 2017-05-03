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

	RT::Image *FindImage(Collada05::const_image&)const;

COLLADA_(private) //LoadCOLLADA subroutines

	friend class RT::Frame;
	void LoadCOLLADA(Collada05::const_COLLADA);
	DBase(){} ~DBase();

	template<class S, class T>
	/**
	 * This is now only used to pre-load animations because
	 * their targets find them bottom-up instead of top-down.
	 *
	 * In theory it might be useful to force a library to be
	 * loaded, so it can be browsed. It's not clear right now
	 * if these APIs should be used for that purpose however.
	 */
	inline void LoadLibrary(const daeName &msg, S &in, T f)
	{
		daeEH::Verbose<<"Loading "<<msg<<" Library "<<in.name;
		for(size_t i=0;i<in.size();i++) (this->*f)(in[i]);
	}

	struct LoadMatrix_of;
	struct LoadTargetable_of;	
	struct LoadLight_technique_common;
	RT::Light *LoadLight(Collada05::const_light);
	struct LoadCamera_technique_common;
	RT::Camera *LoadCamera(Collada05::const_camera);
	struct LoadEffect_profile_COMMON;
	RT::Effect *LoadEffect(Collada05::const_effect);
	RT::Material *LoadMaterial(Collada05::const_material);
	friend struct RT::RigidBody;
	struct LoadGeometry_technique_common;
	RT::Geometry *LoadGeometry(Collada05::const_geometry);
	RT::Image *LoadImage(Collada05::const_image);
	struct LoadAnimation_channel;
	void LoadAnimation(Collada05::const_animation);		
	struct LoadInstances_of;
	struct LoadTransforms_of;	
	RT::Node *LoadNode(Collada05::const_node, int i=1, RT::Node *parent=nullptr);
	struct LoadController_skin;
	struct LoadController_morph;	
	RT::Controller *LoadController(Collada05::const_controller);

	friend class RT::Physics;
	struct LoadScene_Physics;	
	void LoadScene(Collada05::const_scene);	

COLLADA_(public)				   

	//SCHEDULED FOR REMOVAL
	template<class T, class U>
	inline T *_GetRT(const std::vector<T*> &v, const U &e)const
	{
		if(e!=nullptr) 
		{
			xs::string URI = nullptr;
			for(size_t i=0;i<v.size();i++)		
			{
				RT::Base *b = (RT::Base*)v[i];
				if(b->Id!=e->id) continue;
				if(URI==nullptr) URI = RT::DocURI(e);
				if(URI==b->DocURI) return v[i];
			}
		}return nullptr;
	}	
	inline RT::Camera *GetCamera(const Collada05::const_camera &e)const
	{
		return _GetRT(Cameras,e);
	}	
	inline RT::Geometry *GetGeometry(const Collada05::const_geometry &e)const
	{
		return _GetRT(Geometries,e);
	}
	inline RT::Controller *GetController(const Collada05::const_controller &e)const
	{
		return _GetRT(Controllers,e);
	}
	inline RT::Light *GetLight(const Collada05::const_light &e)const
	{
		return _GetRT(Lights,e);
	}
	inline RT::Effect *GetEffect(const Collada05::const_effect &e)const
	{
		return _GetRT(Effects,e);
	}
	inline RT::Material *GetMaterial(const Collada05::const_material &e)const
	{
		return _GetRT(Materials,e);
	}
	inline RT::Image *GetImage(const Collada05::const_image &e)const
	{
		return _GetRT(Images,e); 
	}
	inline RT::PhysicsModel *GetPhysicsModel(const Collada05::const_physics_model &e)const
	{
		return _GetRT(PhysicsModels,e); 
	}
	inline RT::Node *GetNode(const Collada05::const_node &e)const
	{
		return _GetRT(Nodes,e);
	}
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__SCENE_H__
/*C1071*/