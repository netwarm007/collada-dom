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
class Effect : public FX::NewParamable, public FX::Annotatable
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
	
	std::vector<FX::Technique*> Techniques; 

	std::vector<FX::NewParamable*> Profiles;

COLLADA_(public)

	Effect(xs::ID id, CGcontext=nullptr);
	~Effect();

	void Apply();
};
	  
/**
 * Effect < Profile < Technique < Material
 */
class Technique : public FX::NewParamable, public FX::Annotatable
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
							  	
	std::vector<FX::Pass*> Passes;

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

	Technique(FX::NewParamable *parent, xs::ID sid);
	~Technique();

	void Apply();
};

/**
 * Effect < Profile < Technique < Material
 *
 * @note @c FX::NewParamable is the <instance_effect> child of
 * <material>.
 */
class Material : public FX::NewParamable
{
COLLADA_(public)
	
	//SCHEDULED FOR REMOVAL
	xs::ID Name;

COLLADA_(public)

	Material(FX::NewParamable *parent, xs::ID id)
	:NewParamable(parent),Name(id){}

	/**
	 * The RT component is using these to begin/end
	 * a pass, but only with 1 pass, and it's unclear
	 * if NewParamable::Apply() should apply as well???
	 */
	void SetPassState(int),ResetPassState(int);

	FX::Technique *FindTechnique()
	{
		return (FX::Technique*)Parent_NewParamable;
	}
};

//-------.
	}//<-'
}

#endif //__COLLADA_FX__EFFECT_H__
/*C1071*/