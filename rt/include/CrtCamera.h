/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__CAMERA_H__
#define __COLLADA_RT__CAMERA_H__

#include "CrtTypes.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
		 
/**
 * The RT::Camera class holds the parameters for a camera taken from a <camera> inside a <library_cameras>.  
 * These parameters are then used to instantiate an <instance_camera> in a node.  Currently, the
 * instance_cameras reference this data so if you change it here all the related instance_cameras will
 * change too.  This should probably be changed so each <instance_camera> has a copy of the <camera> data,
 * this would allow the parameters of individual <instance_camera> to change.
 */
class Camera : public RT::Base
{
COLLADA_(public)

	RT::Float 
	Xfov, /**< X field of view for perspective camera */
	Yfov, /**< Y field of view for perspective camera */
	Aspect, /**< Aspect Ratio for perspective camera */
	ZNear, /**< Z clip near for perspective camera */
	ZFar, /**< Z clip far for perspective camera */
	Xmag, /**< Nonzero X magnification for an orthographic camera */
	Ymag; /**< Nonzero Y magnification for an orthographic camera */

COLLADA_(public)

	Camera():Xfov(),Yfov(),Aspect(),ZNear(0.1f),ZFar(1000),Xmag(),Ymag()
	,_Model(){}
	~Camera(){}

	//There's probably a better way?
	//New: It seems COLLADA lets the aspect ratio be viewport dendendent.
	int _Model;

	inline int Refresh(RT::Float vpAspect=0)
	{
		int o = Xmag!=0||Ymag!=0?2:3;
		RT::Float &x = o==2?Xmag:Xfov;
		RT::Float &y = o==2?Ymag:Yfov;
		if(_Model==0)
		{
			if(x!=0) _Model|=1;
			if(y!=0) _Model|=2;
			if(Aspect!=0) _Model|=4;
		}
		RT::Float &z = (_Model&4)!=0?Aspect:vpAspect;

		Refresh(x,y,z); return o;
	}
	inline void Refresh(RT::Float &X, RT::Float &Y, const RT::Float &A)
	{
		if((_Model&4)==0) //Need aspect?
		{
			Aspect = (_Model&1)==0||(_Model&2)==0?A!=0?A:16.0f/9:X/Y; 
		}
		if((_Model&2)==0) //Need ymag/yfov?
		{
			Y = (_Model&1)!=0?A/X:36;
		}
		if((_Model&1)==0) X = A*Y; //xmag/xfov?
	}
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__CAMERA_H__
/*C1071*/