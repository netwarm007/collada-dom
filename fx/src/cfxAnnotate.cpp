/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <FX.pch.h> //PCH

#include "cfxData.h"
#include "cfxEffect.h"
#include "cfxParam.h"
#include "cfxPass.h"

COLLADA_(namespace)
{
	namespace FX
	{//-.
//<-----'

void FX::Annotate::Apply(const FX::Effect *effect)
{
	if(Cg==nullptr)
	Cg = cgCreateEffectAnnotation(effect->Cg,Name,Data->GetType());
	Data->Apply(this);
}
void FX::Annotate::Apply(const FX::Param *param)
{
	if(Cg==nullptr)
	Cg = cgCreateParameterAnnotation(param->Cg,Name,Data->GetType());
	Data->Apply(this);
}
void FX::Annotate::Apply(const FX::Pass *pass)
{
	if(Cg==nullptr)
	Cg = cgCreatePassAnnotation(pass->Cg,Name,Data->GetType());
	Data->Apply(this);
}
void FX::Annotate::Apply(const FX::Technique *technique)
{
	if(Cg==nullptr)
	Cg = cgCreateTechniqueAnnotation(technique->Cg,Name,Data->GetType());
	Data->Apply(this);
}
void FX::Annotate::Apply(const FX::Shader *shader)
{
	if(Cg==nullptr)
	Cg = cgCreateProgramAnnotation(shader->Cg_Program,Name,Data->GetType());
	Data->Apply(this);
}

//-------.
	}//<-'
}

/*C1071*/