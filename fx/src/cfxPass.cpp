/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <FX.pch.h> //PCH

#include "cfxEffect.h"
#include "cfxPass.h"

COLLADA_(namespace)
{
	namespace FX
	{//-.
//<-----'

FX::Shader::Shader(FX::Pass *pass, FX::Cg_Stage stageIn
,xs::ID prof, xs::string args, xs::ID f, xs::string src)
:Pass(pass)
{	
	CGGLenum stage = 
	stageIn==stageIn.VERTEX?CG_GL_VERTEX:CG_GL_FRAGMENT;	
	_InitCg(stage,prof,args,f,src);
}
FX::Shader::Shader(FX::Pass *pass, FX::FX_Stage08 stageIn
,xs::ID prof, xs::string args, xs::ID f, xs::string src)
:Pass(pass)
{	
	//ASSIUMING CG!
	assert(0!=pass->Technique->FindEffect()->Cg);
	CGGLenum stage; switch(stageIn)
	{
	case stageIn.VERTEX: stage = CG_GL_VERTEX;
	case stageIn.FRAGMENT: stage = CG_GL_FRAGMENT;
	case stageIn.GEOMETRY: stage = CG_GL_GEOMETRY;
	case stageIn.TESSELLATION: 
		
		stage = CG_GL_TESSELLATION_CONTROL;
		stage = CG_GL_TESSELLATION_EVALUATION;
		daeEH::Error<<"Encountered TESSELLATION shader.\n"
		"(It's unclear what COLLADA wants. Do you have some idea?)";

	default: new(this) FX::Shader(pass); return;
	}
	_InitCg(stage,prof,args,f,src);
}
void FX::Shader::_InitCg(CGGLenum stage, xs::ID prof, xs::string args, xs::ID f, xs::string src)
{
	if(0&&prof!=nullptr&&prof[0]!='\0')
	{
		Cg_Profile = cgGetProfile(prof);
		
		//cgGLEnableProfile(Cg_Profile);
		//CGerror err = cgGetError();
		if(CG_PROFILE_UNKNOWN==Cg_Profile
		||!cgIsProfileSupported(Cg_Profile))
		{
			daeEH::Warning<<"Cg did not enumerate profile string "<<prof<<"\n"
			"(Falling back to cgGLGetLatestProfile)";
			goto profile_unknown;
		}
	}
	else if(0) profile_unknown: //Assuming OpenGL.
	{
		Cg_Profile = cgGLGetLatestProfile(stage);
	}
	else //The vp30/fp30 profiles just don't work.
	{
		Cg_Profile = stage==CG_GL_VERTEX?CG_PROFILE_GLSLV:CG_PROFILE_GLSLF;
	}
	
	xs::string args_0[2] = { args,nullptr };

	FX::Technique *tech = Pass->Technique;
	CGcontext Cg = tech->FindEffect()->Cg_Context;	

	//From the looks of things there's no way to know
	//if the code is binary or not.
	//UNFORTUNATELY cgCreateProgram DOESN'T CHECK FOR
	//BINARY COMPATIBILITY.
	CGenum cg_file_type;
	for(size_t i=1;i<2;i++)
	{
		//Is this necessary?
		#ifdef SN_TARGET_PS3 	
		cg_file_type = i==0?CG_BINARY:CG_SOURCE;
		#else
		cg_file_type = i==0?CG_OBJECT:CG_SOURCE;
		#endif			
		Cg_Program = cgCreateProgram(Cg,cg_file_type,src,Cg_Profile,f,args_0);		
		if(Cg_Program!=nullptr)
		break;
		if(i==0) daeEH::Warning<<
		"(False alarm? Failed to load as binary shader. Trying to compile...)";
	}

	if(Cg_Program==nullptr)
	{
		daeEH::Error<<"Failed to loaded Cg program.";
		return;
	}
	else daeEH::Verbose<<"Loaded Cg program.";

	if(stage==CG_GL_VERTEX)
	{
		daeEH::Verbose<<"Vertex program "<<f;
		Cg_State = cgGetNamedState(Cg,"VertexProgram");
	}
	else if(stage==CG_GL_FRAGMENT)
	{
		daeEH::Verbose<<"Fragment program "<<f;
		Cg_State = cgGetNamedState(Cg,"FragmentProgram");
	}
	else daeEH::Error<<"Unsupported target profile.";

	Cg_Assignment = cgCreateStateAssignment(Pass->Cg,Cg_State);

	cgSetProgramStateAssignment(Cg_Assignment,Cg_Program);	

	//////EXPERIMENTAL
	//This is because the old COLLADA-CTS requires lighting-normal
	//auto-generation, and the old demo.dae file wants tangents too.
	//If COLLADA wanted this kind of compression it could've said so.
	if(stage==CG_GL_VERTEX)
	for(CGparameter p=cgGetFirstParameter(Cg_Program,CG_PROGRAM);p!=nullptr;)
	{	
		//Technically this should be recursive.
		CGparameter q;
		if(CG_STRUCT==cgGetParameterType(p))
		q = cgGetFirstStructParameter(p);
		else q = p;  do
		{
			#ifdef _DEBUG
			const char *name = cgGetParameterName(q);
			#endif
			const char *sem = cgGetParameterSemantic(q);
			if(sem!=nullptr) switch(sem[0])
			{
			case 'n': case 'N': //Assuming NORMAL0.

				if(sem[1]=='o'||sem[1]=='O')
				tech->Generate.NORMAL = true;
				break;

			case 't': case 'T': //Assuming TANGENT0.

				if(sem[1]=='a'||sem[1]=='A')
				tech->Generate.TANGENT = true;
				break;

			case 'b': case 'B': //Assuming BINORMAL0.

				if(sem[1]=='i'||sem[1]=='I')
				tech->Generate.BINORMAL = true;
				break;
			}

		}while(q!=p&&nullptr!=(q=cgGetNextParameter(q)));

		p = cgGetNextParameter(p);		
	}
}

//-------.
	}//<-'
}

/*C1071*/