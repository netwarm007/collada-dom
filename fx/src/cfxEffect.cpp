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

FX::Effect::Effect(xs::ID id, CGcontext Cg_Context)
:Paramable(nullptr),Id(id),Cg_Context(Cg_Context),Cg()
{
	if(Cg_Context!=nullptr)
	{
		//create the new effect and set its name
		Cg = cgCreateEffect(Cg_Context,nullptr,nullptr);
		cgSetEffectName(Cg,Id);
	}
}
FX::Effect::~Effect()
{
	if(Cg!=nullptr) cgDestroyEffect(Cg);

	for(size_t i=0;i<Profiles.size();i++)	
	delete Profiles[i];
	for(size_t i=0;i<Techniques.size();i++)	
	delete Techniques[i];
}

FX::Technique::Technique(FX::Paramable *parent, xs::ID  sid, void *profile_CG)
:Paramable(parent),Sid(sid),Generate(),Cg()
{
	//Reminder: The effect may use Cg, yet this technique may not.
	if(profile_CG!=nullptr) Cg = cgCreateTechnique(parent->FindEffect()->Cg,Sid);
}
FX::Technique::~Technique()
{
	for(size_t i=0;i<Passes.size();i++) delete Passes[i];	
}
	
//extern FX::Pass Profile_COMMON;

void FX::Material::SetPassState(int pass)
{
	FX::Technique *p = FindTechnique();
	if(!p->IsProfile_COMMON())
	{
		FX::Pass &q = *p->Passes[pass];
		
		if(p->IsProfile_CG())
		{
			//SCHEDULED FOR REMOVAL
			//Requires glPushAttrib!!!
			cgSetPassState(q.Cg);
		}
		else GL.UseProgram(q.GLSL);

		for(size_t i=0;i<q.ShaderParams.size();i++)
		q.ShaderParams[i].Apply();
	}
	else Profile_COMMON.SetPassState();
}
void FX::Material::SetWorld(int pass)
{
	FX::Technique *p = FindTechnique();
	if(!p->IsProfile_COMMON())	
	{
		FX::Pass &q = *p->Passes[pass];
		for(int i=0;i<q.World;i++)	
		q.ShaderParams[i].Apply();	
	}
	else FX::Profile_COMMON.SetWorld();
}
void FX::Material::ResetPassState(int pass)
{
	FX::Technique *p = FindTechnique();
	if(!p->IsProfile_COMMON())
	{
		//SCHEDULED FOR REMOVAL
		if(p->IsProfile_CG())
		{
			//SCHEDULED FOR REMOVAL
			//Requires glPopAttrib!!!
			cgResetPassState(p->Passes[pass]->Cg);
		}
		else GL.UseProgram(0);
	}
	else Profile_COMMON.ResetPassState();
}

//-------.
	}//<-'
}

/*C1071*/