/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_FX__PASS_H__
#define __COLLADA_FX__PASS_H__

#include "cfxEffect.h"

COLLADA_(namespace)
{
	namespace FX
	{//-.
//<-----'

//These are limited to vertex/fragment shaders.
typedef Collada05_XSD::cg_pipeline_stage Cg_Stage;
typedef Collada05_XSD::glsl_pipeline_stage GLSL_Stage;
//1.5.0 has more shader types and does not differentiate.
//(Possibly the existence of the Cg handles could be used.)
typedef Collada08_XSD::fx_pipeline_stage_enum FX_Stage08;

class Shader : public FX::Annotatable, public FX::Paramable
{
COLLADA_(public)

	FX::Pass *Pass;

	CGprofile Cg_Profile;
	CGprogram Cg_Program;
	CGstate Cg_State; 
	CGstateassignment Cg_Assignment;

	struct Generate
	{
	unsigned NORMAL:1;
	unsigned TANGENT:1;
	unsigned BINORMAL:1;
	operator unsigned&(){ return *(unsigned*)this; }
	}Generate;

COLLADA_(public)

	using Paramable::Apply;

	Shader(FX::Pass *p):Pass(p),Generate()
	,Cg_Profile(),Cg_Program(),Cg_State(),Cg_Assignment()
	{}
	Shader(FX::Pass*,FX::Cg_Stage,xs::ID prof, xs::string args, xs::ID f, xs::string src);
	Shader(FX::Pass*,FX::GLSL_Stage,xs::ID,xs::string,xs::ID,xs::string);
	Shader(FX::Pass*,FX::FX_Stage08,xs::ID,xs::string,xs::ID,xs::string);	
	void _InitCg(CGGLenum,xs::ID,xs::string,xs::ID,xs::string);
};

class Pass : public FX::Annotatable
{
COLLADA_(public)

	//SCHEDULED FOR REMOVAL
	FX::Technique *Technique;
		
	std::vector<FX::Shader*> Shaders;

	/**
	 * Previously "pass." 
	 * Capitalizing "pass" conflicts with the constructor.
	 * (All Cg variables should be distinguished eventually.)
	 */
	CGpass Cg;

COLLADA_(public)

	Pass(FX::Technique *technique, xs::ID sid)
	:Technique(technique)
	{
		Cg = cgCreatePass(Technique->Cg,sid);
	}
	~Pass()
	{
		for(size_t i=0;i<Shaders.size();i++)
		delete Shaders[i];
	}

	void Apply()
	{
		for(size_t i=0;i<Shaders.size();i++)
		Shaders[i]->Apply();
	}
};

//-------.
	}//<-'
}

#endif //
/*C1071*/