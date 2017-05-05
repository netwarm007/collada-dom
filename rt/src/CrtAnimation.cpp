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
		ch.Apply(out+ch.Target.Min,p0);
	}
}
void RT::Animation_Channel::Apply(RT::Float *out, RT::Spline_Point p0[])
{	
	const RT::Float time = RT::Main.Time;

	//Points-1 is excluding the last point
	//because the original code just lerps.
	struct key
	{
		const RT::Float &value;
		RT::Spline_Point *lower_bound;
		static int compare(const void *in, const void *p)
		{
			if(((key*)in)->value<((RT::Spline_Point*)p)->GetParameters()[0])
			return -1; 
			((key*)in)->lower_bound = (Spline_Point*)p;
			return +1;
		}
	}search = { time,nullptr }; 
	bsearch(&search,p0,Points-1,PointSize*sizeof(RT::Spline_Point),search.compare);

	assert(Target.Max-Target.Min==Parameters-2);

	p0 = search.lower_bound;
	if(p0==nullptr) return;

	RT::Spline_Point *p1 = p0+PointSize;
	RT::Float p0_time = p0->GetParameters()[0];
	RT::Float t = (time-p0_time)/(p1->GetParameters()[0]-p0_time);	
	if(t>1) t = 1; //Clamp to final time.
	for(short i=1;i<Parameters;i++) 
	*out++ = p0->Lerp(i,p1,t); 	
}
  
//-------.
	}//<-'
}

/*C1071*/