/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

//SCHEDULED FOR REMOVAL
#ifndef ColladaYY 
#error CrtScene.hpp is included by CrtScene.h.
#endif

	void LoadCOLLADA(ColladaYY::const_COLLADA&);
	struct LoadTargetable_of;	
	struct LoadLight_technique_common;
	RT::Light *LoadLight(ColladaYY::const_light&);
	struct LoadCamera_technique_common;
	RT::Camera *LoadCamera(ColladaYY::const_camera&);
	struct LoadEffect_profile_COMMON;
	RT::Effect *LoadEffect(ColladaYY::const_effect&);
	RT::Material *LoadMaterial(ColladaYY::const_material&);
	friend struct RT::RigidBody;
	struct LoadGeometry_technique_common;
	RT::Geometry *LoadGeometry(ColladaYY::const_geometry&);
	RT::Image *LoadImage(ColladaYY::const_image&);
	struct LoadAnimation_channel;
	void LoadAnimation(ColladaYY::const_animation&);		
	struct LoadInstances_of;
	struct LoadTransforms_of;	
	RT::Node *LoadNode(ColladaYY::const_node&, int i=1, RT::Node *parent=nullptr);
	struct LoadController_skin;
	struct LoadController_morph;	
	RT::Controller *LoadController(ColladaYY::const_controller&);

	friend class RT::Physics;
	struct LoadScene_Physics;	
	void LoadScene(ColladaYY::const_scene&);		

COLLADA_(public)				   

	//SCHEDULED FOR REMOVAL
	#ifdef PRECOMPILING_COLLADA_RT
	#ifndef __COLLADA_RT__SCENEYY_HPP__
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
	#endif
	//const causes ambiguous resolution???
	inline RT::Camera *GetCamera(/*const*/ ColladaYY::const_camera &e)const
	{
		return _GetRT(Cameras,e);
	}	
	inline RT::Geometry *GetGeometry(/*const*/ ColladaYY::const_geometry &e)const
	{
		return _GetRT(Geometries,e);
	}
	inline RT::Controller *GetController(/*const*/ ColladaYY::const_controller &e)const
	{
		return _GetRT(Controllers,e);
	}
	inline RT::Light *GetLight(/*const*/ ColladaYY::const_light &e)const
	{
		return _GetRT(Lights,e);
	}
	inline RT::Effect *GetEffect(/*const*/ ColladaYY::const_effect &e)const
	{
		return _GetRT(Effects,e);
	}
	inline RT::Material *GetMaterial(/*const*/ ColladaYY::const_material &e)const
	{
		return _GetRT(Materials,e);
	}
	inline RT::Image *GetImage(/*const*/ ColladaYY::const_image &e)const
	{
		return _GetRT(Images,e); 
	}
	inline RT::PhysicsModel *GetPhysicsModel(/*const*/ ColladaYY::const_physics_model &e)const
	{
		return _GetRT(PhysicsModels,e); 
	}
	inline RT::Node *GetNode(/*const*/ ColladaYY::const_node &e)const
	{
		return _GetRT(Nodes,e);
	}
	#endif

#ifndef __COLLADA_RT__SCENEYY_HPP__
#define __COLLADA_RT__SCENEYY_HPP__ 
#endif
/*C1071*/