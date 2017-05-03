/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <FX.pch.h> //PCH

#include "cfxData.h"
#include "cfxPass.h"
#include "cfxEffect.h"
					 
COLLADA_(namespace)
{
	namespace FX
	{//-.
//<-----'

FX::Effect::Effect(xs::ID id, CGcontext ctxt)
:NewParamable(nullptr),Id(id),Cg_Context(ctxt)
{
	if(ctxt!=nullptr)
	{
		//create the new effect and set its name
		Cg = cgCreateEffect(ctxt,nullptr,nullptr);
		cgSetEffectName(Cg,Id);
		assert(Cg!=nullptr);
	}
	else assert(0);
}
FX::Effect::~Effect()
{
	//#ifndef SN_TARGET_PS3 //???
	cgDestroyEffect(Cg);
	//#endif

	for(size_t i=0;i<Profiles.size();i++)	
	delete Profiles[i];
	for(size_t i=0;i<Techniques.size();i++)	
	delete Techniques[i];
}
void FX::Effect::Apply()
{
	//THIS CAN'T WORK.
	#ifdef NDEBUG 
	#error <instance_effect> must instantiate these things, top-down.
	#endif
	Annotatable::Apply(this); NewParamable::Apply();

	for(size_t i=0;i<Profiles.size();i++)	
	Profiles[i]->Apply();

	//////moving technique earlier so the connection can happen before the set because 
	//////propagation of parameter value is not quite implemented properly yet.
	for(size_t i=0;i<Techniques.size();i++)	
	{
		Techniques[i]->Apply();
							 
		//i think there should be a better place for this, but until i know what that is...
		if(!cgValidateTechnique(Techniques[i]->Cg))
		{
			daeEH::Error<<"INVALID TECHNIQUE"; //breakpoint
		}
		else daeEH::Verbose<<"VALID TECHNIQUE";
	}
}

FX::Technique::Technique(FX::NewParamable *parent, xs::ID  sid)
:NewParamable(parent),Sid(sid)
{
	Cg = cgCreateTechnique(parent->FindEffect()->Cg,Sid);
}
FX::Technique::~Technique()
{
	for(size_t i=0;i<Passes.size();i++)
	delete Passes[i];	
}
void FX::Technique::Apply()
{
	Annotatable::Apply(this); NewParamable::Apply();

	for(size_t i=0;i<Passes.size();i++)
	Passes[i]->Apply();
}
	
void FX::Material::SetPassState(int pass)
{
	cgSetPassState(((FX::Technique*)Parent_NewParamable)->Passes[pass]->Cg);
}
void FX::Material::ResetPassState(int pass)
{
	cgResetPassState(((FX::Technique*)Parent_NewParamable)->Passes[pass]->Cg);
}

//-------.
	}//<-'
}

/*C1071*/