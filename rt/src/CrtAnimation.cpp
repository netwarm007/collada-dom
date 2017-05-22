/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <RT.pch.h> //PCH

#include "CrtRender.h"
#include "CrtAnimation.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
		
size_t RT::Animator::GetMax()
{
	return Animation->Channels[Channel].Target.Max;
}
void RT::Animator::Animate(RT::Float *out)
{
	if(Animation->NowPlaying) 
	{
		RT::Animation_Channel &ch = Animation->Channels[Channel];
		RT::Spline_Point *p0 = &Animation->SamplePoints[ch.SamplePointsMin];
		ch.Fill(out+ch.Target.Min,p0);
	}
}
void RT::Animation_Channel::Fill(RT::Float out[], RT::Spline_Point p0[])
{	
	const RT::Float time = RT::Main.Time;

	//BSPLINE has extra data at the very end. This offset is stored in padding.
	const size_t BSPLINE = RT::Main.AnimationLinear?0:BSPLINE_Real_TimeTable_1;

	struct key
	{
		const RT::Float &value;
		char *lower_bound;
		static int compare(const void *in, const void *p)
		{
			if(((key*)in)->value<*(RT::Float*)p)
			return -1; 
			((key*)in)->lower_bound = (char*)p;
			return +1;
		}
	}search = { time,nullptr }; 
	
	size_t size = sizeof(RT::Spline_Point);
	if(BSPLINE==0) size*=PointSize;	

	char *begin = (char*)(p0+1+BSPLINE);
	bsearch(&search,begin,Points-1,size,search.compare);
	p0+=PointSize*(search.lower_bound-begin)/size;

	if(nullptr==search.lower_bound)
	return;
			
	//Maybe the non LINEAR code is bad, but many files seem
	//to have truly awful INTERPOLATION information in them.
	RT::Float t; RT::Spline_Point *p1 = p0+PointSize;
	switch(RT::Main.AnimationLinear?-1:p0->Algorithm)
	{
	case RT::Spline_Type::BEZIER:

		t = p0->Find<RT::Spline_Type::BEZIER>(p1,time); 
		break;

	//The manual says CARDINAL's tension constant
	//is "baked into" the tangents.
	case RT::Spline_Type::CARDINAL:
	case RT::Spline_Type::HERMITE:

		t = p0->Find<RT::Spline_Type::HERMITE>(p1,time); 
		break;

	case RT::Spline_Type::BSPLINE:

		t = p0->Find<RT::Spline_Type::BSPLINE>(p1,time); 
		break;

	case RT::Spline_Type::STEP:
	case RT::Spline_Type::LINEAR: default:
	{
		RT::Float p0_time = p0->GetParameters()[0];
		t = (time-p0_time)/(p1->GetParameters()[0]-p0_time);	
		if(t>1) t = 1; //Clamp to final time?

		if(RT::Main.AnimationLinear)		
		return p0->Fill<RT::Spline_Type::LINEAR>(out,1,Parameters,p1,t); 	
	}}	return p0->Fill(out,1,Parameters,p1,t); 	
}
  
//-------.
	}//<-'
}

/*C1071*/