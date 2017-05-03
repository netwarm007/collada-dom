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
struct Spline_Algorithm
{
	enum
	{	
	BEZIER=1, //Requires both IN_TANGENT/OUT_TANGENT.
	LINEAR=2,
	BSPLINE=4,
	HERMITE=8, //Requires IN/OUT_TANGENT like BEZIER.
	CARDINAL=16,
	STEP=32,
	INTERPOLATION_MASK=0xFF,
	C0=256, //CONTINUITY
	C1=512, //CONTINUITY
	G1=1024, //CONTINUITY
	CONTINUITY_MASK=0x700,
	LINEAR_STEPS=2048, //LINEAR_STEPS prescence only.
	};
};

/**VARIABLE-LENGTH
 * Parameters are stored after the descriptor fields.
 * The number of parameters is held in @c RT::Spline.
 */
struct Spline_Point
{
	/**
	 * This is designed to occupy the memory footprint
	 * of @c RT::Float.
	 */
	int Algorithm:sizeof(RT::Float)*CHAR_BIT/2;
	int LinearSteps:sizeof(RT::Float)*CHAR_BIT/2;

	RT::Float *GetParameters()
	{
		daeCTC<sizeof(this)==sizeof(RT::Float)>;
		return (RT::Float*)this+1;
	}

	RT::Float Lerp(size_t param, RT::Spline_Point *p2, RT::Float t)
	{
		RT::Float out = GetParameters()[param];
		return out+(p2->GetParameters()[param]-out)*t;
	}
};

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
	#ifdef _DEBUG
	//This is for debugging with memory inspectors.
	xs::string SIDREF;
	#endif

COLLADA_(public)
	/**
	 * This indexes @c RT::Animation::SamplePoints.
	 * The next is at @c SamplePointsMin+PointSize.
	 */
	short SamplePointsMin;
	/**
	 * This is the parameters of @c SIDREF that're
	 * to be animated.
	 */
	RT::Target Target;

COLLADA_(public)
	/**
	 * PARTIAL CONSTRUCTOR
	 */
	Animation_Channel(xs::string SIDREF)
	:SamplePointsMin()
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
	void Apply(RT::Float out[], RT::Spline_Point p0[]);
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
	 */
	std::vector<RT::Spline_Point> SamplePoints;
	/**
	 * Each channel has a SIDREF target and an entrypoint for its
	 * @c SamplePoints data, and a description of the spline data.
	 */
	std::vector<RT::Animation_Channel> Channels;
	
	RT::Float TimeMin,TimeMax; bool NowPlaying;

COLLADA_(public)

	Animation():TimeMin(FLT_MAX),TimeMax(),NowPlaying()
	#ifdef _DEBUG
	,__SamplePoints((std::vector<RT::Float>*)&SamplePoints)
	#endif
	{}
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__ANIMATION_H__
/*C1071*/