/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <FX.pch.h> //PCH

#include "cfxParam.h"
#include "cfxData.h"
#include "cfxPass.h"
#include "cfxEffect.h"

COLLADA_(namespace)
{
	namespace FX
	{//-.
//<-----'
   
FX::NewParam::NewParam
(FX::DataMaker<FX::NewParam> *cp):Semantic(cp->aux)
{
	#ifdef NDEBUG
	#error Is Cg instantiation deferred to avoid SID-clashes?
	#endif
	//REMINDER: Sampler types are setting up Cg in advance.
	if(Cg==nullptr)
	{
		Cg = cgCreateEffectParameter
		(cp->p->FindEffect()->Cg,Name,Data->GetType());		
	}
	cgSetParameterSemantic(Cg,Semantic);
	daeEH::Verbose<<"New <param> "<<Name;
}
FX::SetParam::SetParam(FX::DataMaker<FX::SetParam> *cp)
{
	//REMINDER: Sampler types are setting up Cg in advance.
	if(Cg==nullptr) 
	{
		//this allows params created in the effect to be set later
		Cg = cgGetNamedEffectParameter(cp->p->FindEffect()->Cg,Name);		
	}
	
	if(Cg!=nullptr) 
	daeEH::Verbose<<"Set param "<<Name;
}
FX::ConnectParam::ConnectParam(xs::ID from, xs::ID to)
{
	//Not using FX::DataMaker.
	Name = from; Data = nullptr; Cg = Cg_To = nullptr; Name_To = to;
}
FX::ConnectParam::ConnectParam(xs::ID ref, FX::NewParamable *p, xs::token ref2, FX::Effect *e)
{
	new(this) ConnectParam(ref2,ref);

	if(e->Cg!=nullptr) Cg_To = cgGetNamedEffectParameter(e->Cg,Name_To);

	_connect_param(p,e);
}
void FX::ConnectParam::_connect_param(FX::NewParamable *lv, FX::Effect *e)
{	 	
	//source parameter is from an effect 
	//2017: This is technically unnecessary, but it sets Data.
	FX::Param *p = nullptr; do
	{
		//lv constrains the search to the current/outer scope.
		lv->FindConnectingParam(Name,p,lv);
	}
	while(nullptr!=dynamic_cast<FX::BindParam_To*>(p));
	if(p!=nullptr)
	{
		Cg = p->Cg; Data = p->Data;
	}
	else //old way 
	{	
		Cg = cgGetNamedEffectParameter(e->Cg,Name);
		assert(nullptr==Cg);
	}
}
 
void FX::BindParam::Apply()
{
	Cg = cgGetNamedParameter(Shader->Cg_Program,Name);
	if(Data!=nullptr)
	Data->Apply(this);
	else daeEH::Verbose<<
	"Value settings not implemented yet for bind params - data must support all cg types for this.";
}
FX::BindParam_To::BindParam_To(xs::ID symbol, FX::Shader *shader, xs::NCName ref, FX::Effect *e)
:Shader(shader),ConnectParam(ref,symbol)
{
	if(Shader->Cg_Program!=nullptr)
	{
		//dest parameter intended to be in a program
		Cg_To = cgGetNamedParameter(Shader->Cg_Program,Name_To);
		if(Cg_To==nullptr)
		{
			//IS THIS CORRECT?????????????????????????
			//nothing to stop it from being in an effect
			Cg_To = cgGetNamedEffectParameter(e->Cg,Name_To);
		}
		if(Cg_To==nullptr)
		daeEH::Error<<"Could not locate shader variable "<<Name_To<<" for binding to "<<Name;
	}

	_connect_param(Shader->Pass->Technique,e);
}

//-------.
	}//<-'
}

/*C1071*/