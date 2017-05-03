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

	Camera():Xfov(),Yfov(),Aspect(),ZNear(0.1f),ZFar(1000),Xmag(),Ymag(){}
	~Camera(){}

	inline int Refresh()
	{
		int o = Xmag!=0||Ymag!=0?2:3;
		Refresh(o==2?Xmag:Xfov,o==2?Ymag:Yfov); return o;
	}
	inline void Refresh(RT::Float &X, RT::Float &Y)
	{
		if(Aspect==0)
		{
			Aspect = X==0||Y==0?16.0f/9:X/Y; 
		}
		if(Y==0)
		{
			Y = X!=0?Aspect/X:36;
		}
		if(X==0) X = Aspect*Y;
	}
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__CAMERA_H__
/*C1071*/