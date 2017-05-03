/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__TYPES_H__
#define __COLLADA_RT__TYPES_H__

#include "RT.pch.h"

COLLADA_(namespace)
{
	#ifndef COLLADA_RT_MAX_DEPTH
	//This is so graphs cannot go on forever due to cycles
	//whether the cycle is intentional, or accidental. But
	//it also prevents deep graphs that are acyclical.
	#define COLLADA_RT_MAX_DEPTH 32
	#endif
	
	namespace RT
	{
	class DBase; 
	extern class Frame Main;	
	template<class=xs::const_any> class Accessor;
	typedef Accessor<Collada05::const_float_array> Accessor05;
	#ifdef COLLADA__http_www_collada_org_2008_03_COLLADASchema__namespace
	typedef Accessor<Collada08::const_float_array> Accessor08;
	#endif	
	class Animation;
	struct Animator;
	class Camera;		
	class Controller;
	class Effect;
	class Geometry;
	struct Geometry_Semantic;
	class Image;
	class Light;
	class Material;	
	class Node;		
	class Physics;
	class PhysicsModel;
	struct RangeFinder;
	struct RigidBody;
	class Stack_Data;
	struct Target;	
	typedef Collada05_XSD::UpAxisType Up;
	//SCHEDULED FOR REMOVAL?
	//Is/was M00 a PlayStation convention?
	//#ifndef M00
	#ifndef COLLADA_RT_M00 //2017: Using the new RT namespace.
	#define COLLADA_RT_M00
	//Was this so the row/column convention can be overridden?
	enum{ M00=0,M01,M02,M03, M10,M11,M12,M13, M20,M21,M22,M23, M30,M31,M32,M33 };
	#endif
	//2017: This is to conditionally call RT::MatrixTranspose.
	enum{ Matrix_is_COLLADA_order=RT::M30==3 };
	//These might be double. FX::Float3 is always float based.	
	typedef daeDouble Float; 	
	typedef DAEP::Class<RT::Float[4*4]> Matrix; 
	typedef DAEP::Class<RT::Float[4]> Quaternion;	
	typedef std::pair<RT::Up,RT::Float> Up_Meter;
	const RT::Up_Meter &GetUp_Meter(size_t);
	}

	/**TEMPORARY?
	 * Historically animations have been done bottom up.
	 */
	struct RT::Animator
	{
		RT::Animation *Animation; size_t Channel;

		void Animate(RT::Float*);
	};
	/**
	 * @c RT::Animation_Channel::Target is this type of
	 * object. It seems like something that will be put
	 * to use elsewhere.
	 */
	struct RT::Target
	{	
		/**WARNING
		 * This is the member-selection segment of the 
		 * SIDREF specification. Both values are equal
		 * if there is a selection.
		 * @warning The <node> transforms are combined
		 * into a single buffer, so it's not so simple.
		 * @see @c GetSize().
		 */
		short Min,Max;

		/**
		 * @return Returns 1 if the target is a scalar
		 * value or if SIDREF member-selection applies.
		 * It cannot return 0. Other values correspond
		 * to the number of items in an <xs:list> like
		 * value. 
		 * I
		 */
		inline size_t GetSize()
		{
			return Max-Min+1; 
		}

		//SCHEDULED FOR REMOVAL
		/**
		 * This is a raw-pointer source for connecting
		 * the @c RT classes to COLLADA's animatronics.
		 */
		const DAEP::Object *Fragment;

		/**
		 * PARTIAL CONSTRUCTOR
		 */
		Target(size_t min=0, size_t max=0)
		:Min((short)min),Max((short)max),Fragment()
		{ 
			assert(Min>=0&&Max>=Min);
			if(Min<0) Min = 0; if(Max<Min) Max = Min;
		}
	};

	//THIS REQUIRES MORE THOUGHT
	/**
	 * Previously "CrtRangeData."
	 */
	struct RT::RangeFinder
	{
		float Box[2][3],Zoom;
		float &Min(int i){ return Box[0][i]; }
		float &Max(int i){ return Box[1][i]; }
		RangeFinder(){ Clear(); }	
		void Clear(){ memset(this,0x00,sizeof(*this)); }	
		void SetZoom();

		//Previously "SetRange."
		//oprerator() incorporates the RT::Asset state.
		void operator()(RT::Geometry_Semantic&,size_t);	
		template<class T> void operator+=(const T v[3])
		{
			for(int i=0;i<3;i++) if(v[i]<Min(i)) Min(i) = (float)v[i];
			for(int i=0;i<3;i++) if(v[i]>Max(i)) Max(i) = (float)v[i];
		}
		void FitZoom()
		{	
			float across = 2*std::max(Max(0),-Min(0));
			across = std::max(across,2*std::max(Max(2),-Min(2)));
			float max_height = Max(1)-Min(1);
			float zoom_height = max_height*1.376f; //1/tan(36); //FOV?			
			Zoom = std::max(across,zoom_height)*1.75f; //Fudging cube.dae.
		}
	};		
}

#endif //__COLLADA_RT__TYPES_H__
/*C1071*/


