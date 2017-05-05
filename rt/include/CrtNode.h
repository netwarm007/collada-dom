/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__NODE_H__
#define __COLLADA_RT__NODE_H__

#include "CrtData.h"
#include "CrtMemory.h"
#include "CrtController.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
					 
class Transform
{
COLLADA_(public)
	/**
	 * This is an @c RT::Transform_Type @c enum.
	 */
	unsigned short Type, Size;

	const RT::Float *ResetData;

	//SCHEDULED FOR REMOVAL
	/**
	 * This is a raw-pointer source for connecting
	 * the @c RT classes to COLLADA's animatronics.
	 */
	const DAEP::Object *Fragment;

COLLADA_(public)

	Transform(int Size, int Type):Type(Type),Size(Size){}

};
template<int M, int N> struct Transform_Data : Transform
{
	template<class T>
	Transform_Data(const T *e):Transform(M,N)
	{
		ResetData = e->value->data(); Fragment = e;
	}
};
struct Transform_Type
{
	enum{ ROTATE=1,TRANSLATE,LOOKAT,MATRIX,SCALE,SKEW };	
};
typedef Transform_Data<3,RT::Transform_Type::SCALE> Transform_scale;
typedef Transform_Data<3+1,RT::Transform_Type::ROTATE> Transform_rotate;
typedef Transform_Data<3,RT::Transform_Type::TRANSLATE> Transform_translate;
typedef Transform_Data<3*3,RT::Transform_Type::LOOKAT> Transform_lookat;
typedef Transform_Data<4*4,RT::Transform_Type::MATRIX> Transform_matrix;
typedef Transform_Data<1+3+3,RT::Transform_Type::SKEW> Transform_skew;
  
/** 
 * The @c RT::Material_Instance class holds the material bindings found 
 * inside <bind_material><technique_common> in <instance_material>.
 */
class Material_Instance
{
COLLADA_(public)

	xs::string Symbol;	
	
	RT::Material *Material;

COLLADA_(public)

	Material_Instance():Symbol(),Material(){}
};
/** 
 * The RT::Geometry_Instance class holds the information needed to make an instance of a piece of geometry
 * This maps to the <instance_geometry> tag in COLLADA
 * NOTE: All the instances share the same original geometry, copies aren't made so changing the parent
 * will change all the instances.
 */
class Geometry_Instance
{
COLLADA_(public)
	/**
	 * The instanced data.
	 */
	RT::Geometry *Geometry;
	/**
	 * List of material instances in the <technique_common>.
	 */
	std::vector<RT::Material_Instance> Materials;
	
COLLADA_(public)

	Geometry_Instance():Geometry(){}
};
class Controller_Instance
{
COLLADA_(public)
	/**
	 * The instanced data.
	 */
	RT::Controller *Controller;		
	/**
	 * List of material instances in the <technique_common>.
	 */
	std::vector<RT::Material_Instance> Materials;
	
	/**
	 * In theory a <skin> can be applied recursively. In that
	 * case the joints of the shallowest level are first, etc.
	 *
	 * The first @c Skeletons joints are not part of the tree.
	 * They are the entry points for <skeleton> which is used
	 * to guess how many descendants can possibly appear when
	 * the <skin> is mapped to the <instance_controller> node.
	 */
	std::vector<RT::Node*> Joints; size_t Skeletons;
 
COLLADA_(public)

	Controller_Instance():Controller(){}

	inline bool Advance_ControllerData
	(RT::Stack_Data** &it, RT::Stack_Data **itt)
	{
		if(size_t(itt-it)<Joints.size()-Skeletons)
		{
			//COLLADA_RT_MAX_DEPTH?
			it = itt; return false;
		}
		Controller->Update_VBuffer2(it); 
		it+=Joints.size(); return true;
	}
};

/**
 * This class represents a node in the RT scene graph.
 */
class Node : public RT::Base
{
COLLADA_(public)

	xs::string Name;	

	//SCHEDULED FOR REMOVAL
	const DAEP::Object *Fragment;

COLLADA_(public)
	/**
	 * The animation targets are stored in the node
	 * so the transforms can be lightweight objects.
	 */
	std::vector<RT::Animator> Animators;
	std::vector<RT::Transform> Transforms;

COLLADA_(public)

	std::vector<RT::Node*> Nodes;	
	std::vector<RT::Light*> Lights;
	std::vector<RT::Camera*> Cameras;  
	std::vector<RT::Geometry_Instance*> Geometries;
	std::vector<RT::Controller_Instance*> Controllers;

COLLADA_(public)

	Node():Name(),Fragment(){}
	~Node(); 

COLLADA_(public)

	inline void Clear()
	{
		this->~Node(); new(this) RT::Node; 
	}
	
	inline size_t CountDescendants(size_t depth=0)
	{
		if(depth>COLLADA_RT_MAX_DEPTH) return 0;

		size_t n = Nodes.size();
		for(size_t i=0,iN=n;i<iN;i++)
		n+=Nodes[i]->CountDescendants(depth+1);
		for(size_t i=0;i<Controllers.size();i++)
		for(size_t j=0;j<Controllers[i]->Skeletons;j++)
		n+=Controllers[i]->Joints[j]->CountDescendants(depth+1); return n;
	}	
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__NODE_H__
/*C1071*/