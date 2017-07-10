/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__ANIMATION_H__
#define __COLLADA_RT__ANIMATION_H__

#include "CrtData.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'

/**
 * This is COLLADA's spline and animation repertoire.
 * JUST BECAUSE EVERYTHING is here doesn't mean it's
 * all been implemented. (YET.)
 */
struct Spline_Type
{
	enum
	{	
	BEZIER=1, //Requires both IN_TANGENT/OUT_TANGENT.
	LINEAR=2,
	BSPLINE=4,
	HERMITE=8, //Requires IN/OUT_TANGENT like BEZIER.
	CARDINAL=16, //Is this synonymous with HERMITE???
	STEP=32,
	};
};

template<int UNUSED>
//GCC/C++ want this to be specialized in the namespace.
RT::Float Sample1D(size_t,RT::Spline_Point*,RT::Spline_Point*,RT::Float);

/**UNION, VARIABLE-LENGTH
 * Parameters are stored after the descriptor fields.
 * The number of parameters is held in @c RT::Spline.
 */
class Spline_Point
{
COLLADA_(public)
	/**
	 * This is designed to occupy the memory footprint
	 * of @c RT::Float.
	 */
	int Algorithm:sizeof(RT::Float)*CHAR_BIT/2;
	int LinearSteps:sizeof(RT::Float)*CHAR_BIT/2;
	   
	RT::Float *GetParameters()
	{
		daeCTC<sizeof(*this)==sizeof(RT::Float)>();
		return (RT::Float*)this+1;
	}

	bool IsBezier() //Read: has control-points.
	{
		return Algorithm==RT::Spline_Type::BEZIER;
	}
	bool IsHermite() //Read: has tangent-things.
	{
		if(Algorithm==RT::Spline_Type::HERMITE
		||Algorithm==RT::Spline_Type::CARDINAL)
		return true; return false;
	}
	bool IsBSpline() //Read: has bookend points.
	{
		return Algorithm==RT::Spline_Type::BSPLINE;
	}
		
	/**UNION, ALIASING
	 * Constructs BSPLINE bookend points.
	 */
	void Mirror(RT::Spline_Point p1, RT::Spline_Point p2)
	{
		*(RT::Float*)this = 
		*(RT::Float*)&p1-*(RT::Float*)&p2+*(RT::Float*)&p1;
	}

	template<int A>
	//GCC/C++ want this to be specialized in the namespace.
	RT::Float Sample1D(size_t parameter, RT::Spline_Point *p2, RT::Float s)
	{
		return RT::Sample1D<A>(parameter,this,p2,s);
	}
	size_t Stride(RT::Spline_Point *p2) //BEZIER(1) HERMITE(8) CARDINAL(16)
	{
		//HACK: THIS IS EQUAL TO Parameters THAT'S NOT STORED IN THE POINTS.
		size_t stride = p2-this-1; assert(stride%3==0&&0!=(Algorithm&(1|8|16)));
		return stride/3;
	}

	template<int A>
	void Fill(RT::Float *dst, size_t i, size_t iN, RT::Spline_Point *p2, RT::Float s)
	{
		while(i<iN) *dst++ = Sample1D<A>(i++,p2,s);
	}	
	void Fill(RT::Float *dst, size_t i, size_t iN, RT::Spline_Point *p2, RT::Float s)
	{
		switch(Algorithm)
		{	
		case RT::Spline_Type::BEZIER:
		Fill<RT::Spline_Type::BEZIER>(dst,i,iN,p2,s); 	
		break;	
		default: assert(0);
		case RT::Spline_Type::LINEAR: 
		Fill<RT::Spline_Type::LINEAR>(dst,i,iN,p2,s); 	
		break;
		case RT::Spline_Type::BSPLINE:
		Fill<RT::Spline_Type::BSPLINE>(dst,i,iN,p2,s); 	
		break;
		//The manual says CARDINAL's tension constant
		//is "baked into" the tangents.
		case RT::Spline_Type::CARDINAL: 
		case RT::Spline_Type::HERMITE:
		Fill<RT::Spline_Type::HERMITE>(dst,i,iN,p2,s); 	
		break;
		break;
		case RT::Spline_Type::STEP:	
		Fill<RT::Spline_Type::STEP>(dst,i,iN,p2,s); 	
		break;
		}
	}	

	template<int A>
	RT::Float Find(RT::Spline_Point *p2, RT::Float time, RT::Float tolerance=1/60.0f)
	{
		const int maximum = 16; tolerance/=2;

		if(time<=GetParameters()[0]+tolerance) 
		return 0; 
		if(time>=p2->GetParameters()[0]-tolerance) 
		return 1;

		RT::Float floor = 0, ceiling = 1, s = 0.5f;
		for(int attempts=maximum;attempts-->0;)
		{
			RT::Float difference = time-Sample1D<A>(0,p2,s); 
			if(difference>tolerance)
			{
				floor = s; s+=(ceiling-s)/2;
			}
			else if(difference<-tolerance)
			{
				ceiling = s; s-=(s-floor)/2;
			}
			else return s;
		}
		static bool animation_exceeds_max_samples = false;
		assert(animation_exceeds_max_samples); 
		animation_exceeds_max_samples = true; return s;
	}
};
template<> inline
RT::Float Sample1D<RT::Spline_Type::LINEAR>
(size_t parameter, RT::Spline_Point *p1, RT::Spline_Point *p2, RT::Float s)
{
	RT::Float out = p1->GetParameters()[parameter];
	return out+(p2->GetParameters()[parameter]-out)*s;
}
template<> inline
RT::Float Sample1D<RT::Spline_Type::STEP>
(size_t parameter, RT::Spline_Point *p1, RT::Spline_Point *p2, RT::Float s)
{
	return (s>=1?p2:p1)->GetParameters()[parameter];
}
template<> inline
RT::Float Sample1D<RT::Spline_Type::BEZIER>
(size_t parameter, RT::Spline_Point *p1, RT::Spline_Point *p2, RT::Float s)
{
	#define _ const RT::Float \
	&P0 = p1->GetParameters()[parameter],\
	&P1 = p2->GetParameters()[parameter],\
	&C0 = (&P0)[p1->Stride(p2)],\
	&C1 = (&P1)[p1->Stride(p2)*2];
	_ return //P0,P1,C0,C1
	P0*pow(1-s,3) + 3*C0*s*pow(1-s,2) + 3*C1*(1-s)*s*s + P1*s*s*s;
}
template<> inline
RT::Float Sample1D<RT::Spline_Type::HERMITE>
(size_t parameter, RT::Spline_Point *p1, RT::Spline_Point *p2, RT::Float s)
{
	_ return //P0,P1,T0,T1
	P0*(2*s*s*s-3*s*s+1) + C0*(s*s*s-2*s*s+s) + P1*(-2*s*s*s+3*s*s) + C1*(s*s*s-s*s);
	#undef _
}
template<> inline
RT::Float Sample1D<RT::Spline_Type::BSPLINE>
(size_t parameter, RT::Spline_Point *p1, RT::Spline_Point *p2, RT::Float s)
{
	const RT::Float
	&P0 = (p1-(p2-p1))->GetParameters()[parameter],
	&P1 = p1->GetParameters()[parameter],
	&P2 = p2->GetParameters()[parameter],
	&P3 = (p2+(p2-p1))->GetParameters()[parameter];
	return
	P0/6*(-s*s*s+3*s*s-3*s+1) + P1/6*(3*s*s*s-6*s*s+4) + P2/6*(-3*s*s*s+3*s*s+3*s+1) + P3/6*(s*s*s);
}

/**
 * This class describes an array of @c RT::Spline_Point.
 * It's a shared base-class of @c RT::Animation_Channel
 * and @c RT::Spline.
 */
struct Spline_Length
{
	/**
	 * @c Algorithms is the bitwise combination
	 * of every @c Spline_Point::Algorithm value.
	 */
	short Algorithms;
	/**
	 * @c For a 2-D curve this is 2, or 3 for 3-D.
	 * For key-frames the first parameter semantic
	 * is the INPUT. All that remains is the OUTPUT.
	 */
	short Parameters;
	/**
	 * @c PointSize is @c (1+Parameters)*X where
	 * X is 1 or 3 depending on if there is added
	 * data used by BEZIER and/or HERMITE segments.
	 */
	short PointSize;
	/**
	 * @c Points is the number of data points in the
	 * spline. Each point is @c sizeof(Spline_Point)
	 * times @c PointSize.
	 */
	short Points;	
};

class Animation_Channel : public RT::Spline_Length
{
COLLADA_(public)
	/**
	 * This indexes @c RT::Animation::SamplePoints.
	 * The next is at @c SamplePointsMin+PointSize.
	 */
	short SamplePointsMin;
	/**PADDING
	 * This is just padding, so may as well use it.
	 */
	short BSPLINE_Real_TimeTable_1;
	/**
	 * This is the parameters of @c SIDREF that're
	 * to be animated.
	 */
	RT::Target Target;

COLLADA_(private)

	#ifdef _DEBUG
	//This is for debugging with memory inspectors.
	xs::string SIDREF;
	#endif

COLLADA_(public)
	/**
	 * PARTIAL CONSTRUCTOR
	 */
	Animation_Channel(xs::string SIDREF)
	:SamplePointsMin(),BSPLINE_Real_TimeTable_1()
	#ifdef _DEBUG
	,SIDREF(SIDREF)
	#endif
	{ (void)SIDREF; }
	
	/**
	 * Finds parameters in the inputs, where @a out 
	 * points to @c Target.GetSize() values.
	 * @param p0 must be @c Points*PointSize values.
	 * @see RT::Animator::Animate() & RT::Main.Time.
	 */
	void Fill(RT::Float out[], RT::Spline_Point p0[]);
};

/**
 * 2017:
 * This class is being redesigned to support more animation types.
 * The "Spline" stuff is all new. The old animations seemed as if
 * they would not have worked unless all channels had 1 parameter
 * or there was only one multi-parameter channel in the last spot.
 */
class Animation : public RT::Base
{
	#ifdef _DEBUG
	std::vector<RT::Float> *__SamplePoints;
	#endif

COLLADA_(public)
	/**
	 * This is a raw data buffer punctuated by control-blocks for
	 * each control-point in the animation key-frame splines data.
	 * If BSPLINE is present there are time-tables after the data.
	 */
	std::vector<RT::Spline_Point> SamplePoints;
	/**
	 * Each channel has a SIDREF target and an entrypoint for its
	 * @c SamplePoints data, and a description of the spline data.
	 */
	std::vector<RT::Animation_Channel> Channels;
	
	RT::Float TimeMin,TimeMax; bool NowPlaying;

COLLADA_(public)

	Animation():
	#ifdef _DEBUG
	__SamplePoints((std::vector<RT::Float>*)&SamplePoints),
	#endif
	TimeMin(FLT_MAX),TimeMax(),NowPlaying()
	{}
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__ANIMATION_H__
/*C1071*/
