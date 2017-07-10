/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_FX__TYPES_H__
#define __COLLADA_FX__TYPES_H__

//This is the precompiled header, but not
//really, as this isn't how it's included.
#include "FX.pch.h"

//GCC precompiled headers cannot remember
//#pragma diagnostics; so bring them back.
#ifdef PRECOMPILING_COLLADA_FX
#include "../../dom/include/WARNING.HPP"
#endif

COLLADA_(namespace)
{
	namespace DAEP
	{
		#define _ \
		struct const_effect;\
		struct const_image;\
		struct const_material;
		COLLADA_(http_www_collada_org_2005_11_COLLADASchema,namespace)
		{
			_
		}
		COLLADA_(http_www_collada_org_2008_03_COLLADASchema,namespace)
		{
			_
		}
		#undef _
	}

	namespace FX
	{
	class Annotate;
	class Annotatable;
	class BindParam;
	class Data;
	class Effect;
	class Loader;
	class Material;
	class Param;
	class Paramable;
	class NewParam;
	class Paramable;
	class Pass;
	class ShaderParam;
	class Sampler;
	class SetParam;
	class Shader;
	class Surface;
	class Technique;
	struct Bool1
	{
		typedef bool type;
		bool b0;
		//cg_*1 is not <xs:list> based.
		void operator=(bool cp){ b0 = cp; }
	};
	struct Bool2:Bool1{ bool b1; };
	struct Bool3:Bool2{ bool b2; };
	struct Bool4:Bool3{ bool b3; };
	struct Int1
	{
		typedef int type;
		int i0;
		//cg_*1 is not <xs:list> based.
		void operator=(int cp){ i0 = cp; }
	};
	struct Int2:Int1{ int i1; };
	struct Int3:Int2{ int i2; };
	struct Int4:Int3{ int i3; };
	//---.
//<------'
struct Float1
{
	typedef float type;
	//cg_*1 is not <xs:list> based.
	void operator=(float cp){ f0 = cp; }

	//2017: Adding union for Cg like support and
	//so to retire CrtVec2f-4f and CrtColor3f/4f.
	union{ float f0,r,x,s; };
};
struct Float2 : Float1
{
	union{ float f1,g,y,t; };		
};
struct Float3 : Float2
{
	union{ float f2,b,z,p; };

	Float3(){ x = y = z = 0; }

	//Float3(float xyz){ x = y = z = xyz; }
	Float3(double C4244){ x = y = z = (float)C4244; }

	Float3(float (&xyz)[3])
	{
		x = xyz[0]; y = xyz[1]; z = xyz[2]; 
	}

	template<class C4244>
	Float3(C4244 xx, C4244 yy, C4244 zz)
	{
		x = (float)xx; y = (float)yy; z = (float)zz; 

		daeCTC<sizeof(Float3)==sizeof(FX::Float2)+sizeof(float)>();
	}
		
	inline bool IsBlack()const
	{
		return r==0&&g==0&&b==0;
	}

	inline FX::Float3 &Cross(const FX::Float3 &b)
	{
		return *this = FX::Float3(y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x); 
	}

	inline FX::Float3 &Normalize()
	{
		return *this/=sqrt(x*x+y*y+z*z);
	}

	inline FX::Float3 operator+(const FX::Float2 &v)const
	{
		return FX::Float3(x+v.x,y+v.y,z);
	}
	inline FX::Float3 operator+(const FX::Float3 &v)const
	{
		return FX::Float3(x+v.x,y+v.y,z+v.z);
	}		
		
	inline FX::Float3 operator-(const FX::Float2 &v)const
	{
		return FX::Float3(x-v.x,y-v.y,z);
	}
	inline FX::Float3 operator-(const FX::Float3 &v)const
	{
		return FX::Float3(x-v.x,y-v.y,z-v.z);
	}

	inline FX::Float3 operator*(float t)const
	{
		return FX::Float3(x*t,y*t,z*t);
	}
	inline float operator*(const FX::Float3& v)const
	{
		return x*v.x+y*v.y+z*v.z;
	}

	inline FX::Float3 &operator+=(const FX::Float3 &v)
	{
		x+=v.x; y+=v.y; z+=v.z; return *this;
	}
	
	inline FX::Float3 &operator-=(const FX::Float3 &v)
	{
		x-=v.x; y-=v.y; z-=v.z; return *this;
	}

	inline FX::Float3 &operator*=(float t)
	{
		x*=t; y*=t; z*=t; return *this;
	}
		
	inline FX::Float3 &operator/=(float t)
	{
		float v = 1/t; x*=v; y*=v; z*=v; return *this;
	}

	inline bool operator==(const FX::Float3 &v)
	{
		assert(!"used?"); return x==v.x&&y==v.y&&z==v.z;
	}
	inline bool operator!=(const FX::Float3 &v)
	{
		assert(!"used?"); return x!=v.x||y!=v.y||z!=v.z;
	}
};
struct Float4 : Float3
{
	union{ float f3,a,w,q; };

	//SCHEDULED FOR REMOVAL?

	Float4():Float3(),w()
	{}
	template<class C4244>
	Float4(C4244 xyzw):Float3(xyzw),w((float)xyzw)
	{}
	template<class C4244, class A>
	//This comes from CrtColor4f.
	//It's not recommended to let aa default to 1.
	//CrtQuat code will have to use this with 0,1.
	Float4(C4244 xyz, A alpha):Float3(xyz),a((float)alpha)
	{}
	template<class C4244, class W>
	Float4(C4244 xx, C4244 yy, C4244 zz, W ww)
	:Float3(xx,yy,zz),w((float)ww)
	{
		daeCTC<sizeof(Float4)==sizeof(FX::Float2)*2>();
	}

	//THIS IS UNUSED.
	//This had been CrtVec4f::Set(CrtQuat*)
	inline void SetToQuaternionAA(const FX::Float4 &q)
	{
		float s = sqrt(1-(q.w*q.w));
		w = 2*acosf(q.w);
		if(s<0.001f)
		{
			x = q.x; y = q.y; z = q.z;
		}
		else
		{
			x = q.x/s; y = q.y/s; z = q.z/s;
		}
	}

	//UNUSED: Was CrtQuat::Set().
	inline void SetQuaternionFromAA(const FX::Float4 &axisRot)
	{
		//convert the axis rot to quat 
		w = cos(axisRot.w/2.0f);
		x = axisRot.x*sin(axisRot.w/2);
		y = axisRot.y*sin(axisRot.w/2);
		z = axisRot.z*sin(axisRot.w/2);
	}
				   
	//UNUSED: Was CrtQuat::Normalize().
	inline void NormalizeQuaternion()
	{
		float Len = sqrt(x*x+y*y+z*z+w*w);
		if(Len>1e-06f)
		{
			float ILen = 1.0f/Len;
			x*=ILen; y*=ILen; z*=ILen; w*=ILen;
		}
		else x = y = z = w = 0;
	}
};
//------.
	//<-'
	struct Float2x2
	{
		typedef float type;
		float f00,f01,f10,f11;
	};
	struct Float3x3
	{
		typedef float type;
		float f00,f01,f02,f10,f11,f12,f20,f21,f22;
	};
	struct Float4x4
	{
		typedef float type;
		float f00,f01,f02,f03,f10,f11,f12,f13,f20,f21,f22,f23,f30,f31,f32,f33;
	};
	class Sampler
	{
	COLLADA_(public)		

		Sampler():Source(),Params()
		,GenerateMipmaps(true),Cg()
		{}
		~Sampler()
		{
			//Samplers are unnamed, unassociated parameters. 
			//They are required to house sampler attributes.
			if(Cg!=nullptr) cgDestroyParameter(Cg);
		}

		/**
		 * These locate (what should be) the correct texture
		 * after calling @c FX::Material::Apply().
		 * @return Returns @a if_Missing there is no surface.
		 * The "missing" texture is returned if a surface is
		 * missing its texture or if the default overload is
		 * used.
		 * @note This functionality is exposed so the RT can
		 * select a default texture to use in its lo-fi mode.
		 */
		GLuint FindTexID(),FindTexID(GLuint if_Missing);
	  
	COLLADA_(public)	

		xs::ID Source;

		FX::Paramable *Params;

		bool GenerateMipmaps;

		/**
		 * Data doesn't generally require this any longer.
		 * Still @c cgCreateSamplerStateAssignment() does.
		 */
		CGparameter Cg;
	};
	}
}

#endif //__COLLADA_FX__TYPES_H__
/*C1071*/
