/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_FX__EFFECT_H__
#define __COLLADA_FX__EFFECT_H__

#include "cfxParam.h"

COLLADA_(namespace)
{
	namespace FX
	{//-.
//<-----'

/**
 * Effect < Profile < Technique < Material
 */
class Effect : public FX::Paramable, public FX::Annotatable
{
COLLADA_(public)
	
	//SCHEDULED FOR REMOVAL
	//Reminder: can use cgGetEffectContext if necessary.
	xs::ID Id; CGcontext Cg_Context;

	/**
	 * Previously "effect." 
	 * Capitalizing "effect" conflicts with the constructor.
	 * (All Cg variables should be distinguished eventually.)
	 */
	CGeffect Cg;

	/**
	 * If a profile_COMMON technique is present it will be
	 * @c Techniques[0].
	 */
	std::vector<FX::Technique*> Techniques; 

	std::vector<FX::Paramable*> Profiles;

COLLADA_(public)

	Effect(xs::ID,CGcontext=nullptr);
	~Effect();
};
	  
/**
 * Effect < Profile < Technique < Material
 */
class Technique : public FX::Paramable, public FX::Annotatable
{
COLLADA_(public)

	//SCHEDULED FOR REMOVAL
	xs::ID Sid;

	/**
	 * Previously "technique." 
	 * Capitalizing "technique" conflicts with the constructor.
	 * (All Cg variables should be distinguished eventually.)
	 */
	CGtechnique Cg;
	
	/**
	 * profile_COMMON techniques are the only kind that do not
	 * have an @c FX::Pass.
	 */
	std::vector<FX::Pass*> Passes;

	/**
	 * profile_COMMON techniques are using an internal unified
	 * shader.
	 */
	inline bool IsProfile_COMMON(){ return Passes.empty(); }
	/**
	 * This is easier to read, but maybe @c Cg won't be around
	 * forever.
	 */
	inline bool IsProfile_CG(){ return Cg!=nullptr; }

	//EXPERIMENTAL
	//This relates to FX::Shader but is here so the passes and
	//their shaders do not have to be itereated over and so on.
	struct Generate
	{
	unsigned NORMAL:1;
	unsigned TANGENT:1;
	unsigned BINORMAL:1;
	operator unsigned&(){ return *(unsigned*)this; }
	}Generate;

COLLADA_(public)

	Technique(FX::Paramable*,xs::ID,void*CG=nullptr);
	~Technique();
};

/**
 * Effect < Profile < Technique < Material
 *
 * @note @c FX::Paramable is the <instance_effect> child of
 * <material>.
 */
class Material : public FX::Paramable
{
COLLADA_(public)
	
	//SCHEDULED FOR REMOVAL
	xs::ID Name;

COLLADA_(public)

	Material(FX::Paramable *parent, xs::ID id)
	:Paramable(parent),Name(id){}

	/**WARNING
	 * @warning
	 * Cg restores the "default OpenGL state" and
	 * so requires glPushAttrib or is doing unneeded
	 * changes and is probably incorrect if it assumes
	 * defaults going in.
	 * 
	 * FIRST call Apply() to select the instance_effect.
	 * THEN call these to "push and pop" OpenGL's states.
	 * @see @c cgSetPassState() and @c cgResetPassState().
	 *
	 * @note @c SetWorld() refreshes any WORLD parameters.
	 * Their values come from @c FX::NewParam::ClientData.
	 * Use it only if drawing multiple views or instances.
	 */
	void SetPassState(int),SetWorld(int),ResetPassState(int);

	/**NONZERO
	 */
	inline FX::Technique *FindTechnique()
	{
		return (FX::Technique*)Parent_Paramable;
	}

	//SCHEDULED FOR REMOVAL?
	/**INTERNAL, CIRCULAR-DEPENDENCY
	 * @see @c Apply().
	 * @see @c FX::Profile_COMMON.
	 * Call this only for if profile_COMMON
	 * applies; or better yet, don't use it.	 
	 */
	COLLADA_NOINLINE void Apply_COMMON();
	/**
	 * Calls @c Apply_COMMON() as necessary.
	 * @note The "Apply" family of APIs are
	 * used to switch to an instance_effect.
	 * @c FX::Material is the topmost layer
	 * and as such is where it's kicked off.
	 *
	 * @c Apply() does not alter the OpenGL
	 * context. @c SetPassState() does that.
	 */
	inline void Apply()
	{
		//Apply_COMMON() is inline defined 
		//after FX::Profile_COMMON beneath.
		if(FindTechnique()->IsProfile_COMMON())
		return Apply_COMMON(); 
		return Paramable::Apply();
	}	
};

/**SINGLETON
 * The @c FX::Loader framework is tailored to loading
 * resources; and it looks awfully strange repurposed
 * like this.
 */
extern struct Profile_COMMON
{
COLLADA_(public)

	//HACK: RT::Frame::Refresh()
	//is checking OK in case the
	//shaders are having trouble.
	bool OK;
	Profile_COMMON(int):OK(true){}

COLLADA_(public)

	/**INTERNAL
	 * Each effect has one of these 
	 * custom parameters that makes
	 * them known to profile_COMMON.
	 */
	struct Application	
	{	
		GLuint GLSL; 
		
		FX::DataBool Blinn;		

		FX::Technique *Technique;

		Application():GLSL(),Technique()
		{}
	};
	/**INTERNAL
	 */
	typedef FX::DataType<Application> Data;	

COLLADA_(public) //RT query API.

	struct View:FX::Annotate,FX::DataString
	{};
	struct Float:FX::NewParam,FX::DataFloat
	{
		inline operator float()
		{
			return SetData->To<float>();
		}
	};	
	struct Float4:FX::NewParam,FX::DataFloat4
	{
		//This was for getting the colors, but
		//Mono() is provided because the logic
		//requires access to the textures also.
		inline operator FX::Float4()
		{
			return SetData->To<FX::Float4>();
		}
	};
	struct Float4x4:FX::NewParam,FX::DataFloat4x4
	{};
	struct Sampler:FX::NewParam,FX::DataSampler2D
	{
		Sampler() //HACK: Plug nonzero pointers.
		{
			(void*&)Value.Params = 
			(void*)&FX::Profile_COMMON.Surfaces; 
		}
		inline operator GLuint()
		{
			if(!IsSet()) return 0; //OPTIMIZING.
			FX::DataSampler2D *s = SetData->As<FX::Sampler>();
			return s==nullptr?0:s->Value.FindTexID(0);
		}
	};

	struct Color_or_Texture 
	{		
		Float4 Color; Sampler Texture;

		//This is query API for fallback rendering 
		//to OpenGL's single-texture (mono) models.
		inline void Mono(FX::Float4 &c, GLuint &t)
		{
			if(!Texture.IsSet()) c = Color;
			else{ c = 1; if(t==0) t = Texture; }
		}

		inline void Apply(){ Color.Apply(); Texture.Apply(); }
	};	
	Color_or_Texture Emission;
	Color_or_Texture Ambient;
	Color_or_Texture Diffuse;
	Color_or_Texture Specular; 
	Color_or_Texture Transparent;
	Float Shininess, Transparency;

COLLADA_(private) //INTERNALS
		
	struct Internals;
	friend FX::Loader;
	friend FX::Material;
	
	//SCHEDULED FOR REMOVAL?
	struct DataSharingSurvey
	{
		//The fragment programs branch on these.
		//Constant is needed to make it nonzero.
		unsigned Constant:1,Lambert:1,Blinn:1,Phong:1;
		unsigned Emission:1,Ambient:1,Diffuse:1;
		unsigned Specular:1,Transparent:1;
		operator int&(){ return *(int*)this; }
		DataSharingSurvey(int i=0){ *(int*)this = i; }

	}Load;	
	
	const FX::Paramable Surfaces;

	void Init_ClientData(FX::Loader&);

	struct:FX::NewParam,Data{}Application;

	void SetPassState(),SetWorld(),ResetPassState();

	void Apply()
	{
		//Application.Apply();
		Emission.Apply(); Ambient.Apply(); 
		Diffuse.Apply(); Specular.Apply(); Shininess.Apply(); 	
		//UNUSED, but initialize them nonetheless.
		Transparent.Apply(); Transparency.Apply();
	}

}Profile_COMMON; //SINGLETON

typedef FX::Profile_COMMON::Data DataProfile_COMMON;

template<>
inline void DataProfile_COMMON::Load(FX::ShaderParam&)
{ /*NOP*/ }

//SCHEDULED FOR REMOVAL?
/**INTERNAL, CIRCULAR-DEPENDENCY
 * @see @c Apply().
 * @see @c FX::Profile_COMMON.
 * Call this only for if profile_COMMON
 * applies; or better yet, don't use it.	 
 */
COLLADA_NOINLINE inline void FX::Material::Apply_COMMON()
{
	//This resets to defaults.
	FX::Profile_COMMON.Apply();

	//The order is all messed up.
	//Apply();
	{		
		//1) Apply the profile_COMMON block.
		//2) Apply the technique <newparam>.
		//3) Apply this material <setparam>.
		//4) The FX::Param that remain must
		//be FX::Profile_COMMON connections.
		Parent_Paramable->Parent_Paramable->Apply();
		//2) This is for COLLADA 1.4.1 only.
		std::vector<FX::Param*> &pv = Parent_Paramable->Params;
		size_t i;
		for(i=0;i<pv.size()&&pv[i]->IsNewParam();i++)
		pv[i]->Apply();
		//3) Apply this material <setparam>.
		for(size_t j=0;j<Params.size();j++)
		Params[j]->Apply();
		//4) FX::Profile_COMMON connections.
		for(;i<pv.size();i++) pv[i]->Apply();
	}
}

//-------.
	}//<-'
}

#endif //__COLLADA_FX__EFFECT_H__
/*C1071*/