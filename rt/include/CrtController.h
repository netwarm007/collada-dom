/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__CONTROLLER_H__
#define __COLLADA_RT__CONTROLLER_H__

#include "CrtData.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
					   
class Controller : public RT::Base
{
COLLADA_(public)

	RT::Geometry *Geometry;	
	RT::Controller *Source;	

COLLADA_(public)

	Controller():Source(),Geometry(){}
	virtual ~Controller(){}
							
	virtual void Update_VBuffer2(RT::Stack_Data**) = 0;
};

struct Skin_Weight
{
	FX::Float1 Weight; int Joint;
};
  
class Skin : public RT::Controller
{
COLLADA_(public)

	RT::Matrix BindShapeMatrix;

	std::vector<RT::Skin_Weight> Weights;

	//These can be combined into one vector, but
	//it makes loading their <accessor> trickier.
	//Maybe the matrices should just be pointers?
	std::vector<xs::string> Joints;	
	std::vector<RT::Matrix> Joints_INV_BIND_MATRIX;

	/**
	 * @note The manual says ID should not be used.
	 * (But it seems like a possible construction.)
	 */
	bool Using_IDs_to_FindJointNode;

COLLADA_(public)
		
	virtual ~Skin(){}
							
	RT::Node *FindJointNode(RT::Node*,daeName);

	virtual void Update_VBuffer2(RT::Stack_Data**);
};

struct Morph_Target
{
	RT::Float Weight;
	RT::Geometry *Vertices;
	Morph_Target():Vertices(){}
};

class Morph : public RT::Controller
{
COLLADA_(public)

	bool Using_RELATIVE_method;
	std::vector<RT::Morph_Target> MorphTargets;
	std::vector<RT::Animator> Animators;

	Morph():Using_RELATIVE_method(){}
	virtual ~Morph(){}

	virtual void Update_VBuffer2(RT::Stack_Data**);
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__CONTROLLER_H__
/*C1071*/