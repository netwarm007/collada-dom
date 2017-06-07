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

//FX::Loader support //FX::Loader support //FX::Loader support //FX::Loader support

void FX::Pass::Link()
{
	if(!ShaderParams.empty()) return;

	daeEH::Verbose<<"Linked shader program "<<Technique->Sid<<"...";

	FX::Effect *e = Technique->FindEffect();

	if(Cg!=nullptr&&Linked_Cg==nullptr&&GLSL==0)
	{	
		CGprogram programs[3]; 
		int programsN = std::min<int>(3,Shaders.size());
		assert(programsN<=3);
		for(int i=0;i<programsN;i++)
		programs[i] = Shaders[i]->Unlinked_Cg;

		Linked_Cg = cgCombinePrograms(programsN,programs);

		for(int i=0;i<programsN;i++)
		{
			cgDestroyProgram(programs[i]);
			Shaders[i]->Unlinked_Cg = nullptr; //Same.
		}

		//I think any "-Program" state will work to set
		//the combined program. It appears undocumented.
		CGstate state = 
		cgGetNamedState(e->Cg_Context,"VertexProgram");
		CGstateassignment assignment = 
		cgCreateStateAssignment(Cg,state);
		cgSetProgramStateAssignment(assignment,Linked_Cg);	

		//CG_COMPILE_MANUAL is recommended but unneeded.
		cgCompileProgram(Linked_Cg);

		//HACK: This exposes the underlying GLSL handle.		
		//cgGLGetProgramID hadn't always worked, but is
		//under the circumstances.
		//cgSetPassState(Cg);
		//glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&GLSL);
		//cgResetPassState(Cg);
		GLSL = cgGLGetProgramID(Linked_Cg); assert(GLSL!=0);

		//These are UNUSED, but are filled out just for
		//completeness sake.
		GLuint shaders[3]; 
		GL.GetAttachedShaders(GLSL,programsN,&programsN,shaders);
		for(int s,i=0;i<programsN;i++)	
		{
			GL.GetShaderiv(shaders[i],GL_SHADER_TYPE,&s);
			for(int i=0;i<programsN;i++) if(s==Shaders[i]->Stage) 
			Shaders[i]->GLSL = shaders[i];
		}
	}

	//Programs should already be attached by glAttachShader().
	if(GLSL!=0) GL.LinkProgram(GLSL);

	daeStringRef hack = *(daeStringRef*)&e->Id;

	int sampler,i,iN = 0;
	GL.GetProgramiv(GLSL,GL_ACTIVE_UNIFORMS,&iN);
	ShaderParams.resize(iN);	
	GLchar buf[100]; GLsizei len; GLint size; GLenum type;
	for(sampler=i=0;i<iN;i++)
	{
		GL.GetActiveUniform(GLSL,i,sizeof(buf),&len,&size,&type,buf);

		//These are the only samplers according to the documentation.
		if(type==GL_SAMPLER_2D||type==GL_SAMPLER_CUBE)
		{
			//Storing something more useful for binding purposes.
			//TODO: Does samplerCube require special management?
			//What about GLSL's layout(binding=i)?
			type = GL_TEXTURE0+sampler; sampler++;
		}

		ShaderParams[i].GLSL = GL.GetUniformLocation(GLSL,buf);

		GLchar *name = buf; 
		if(name[len-1]==']')		
		{
			while(len!=0&&name[len]!='[') 
			len--; 
			assert(name[len+1]=='0'); //Stripped [0]?
		}
		
		//The Cg API decorates its names.
		if(Cg!=nullptr)
		{
			//An underscore is prepended to the name.
			name++; len--; assert('_'==name[-1]); 
			
			//This is ambiguous, but if a uniform appears in more than 
			//one context, a number is added to the end that is the total
			//number of contexts in which it appears.
			if(isdigit(name[len-1])) len--;
		}
		hack.setString(name,len);

		//-1 includes # because it's not a <newparam>.
		ShaderParams[i].Name = hack-1; 
		ShaderParams[i].Type = type;
		ShaderParams[i].Size = size;
		ShaderParams[i].Cg_decoration = name[len]=='['?'\0':name[len];
		//assert(name[len]=='\0'||isdigit(name[len])||name[len]=='[');

		//daeEH::Warning<<"DEBUG: Uniform "<<i<<": "<<daeName(name,len)<<"["<<size<<"]";
	}
}
FX::ShaderParam *FX::Pass::FindShaderParam(xs::ID sym)
{
	sym--; assert(sym[0]=='#');
	for(size_t i=0;i<ShaderParams.size();i++)
	if(sym==ShaderParams[i].Name) 
	return &ShaderParams[i];
	
	if(Cg==nullptr) return nullptr;

	daeName a = *(daeStringRef*)&sym; //HACK	
	a.extent--;
	for(size_t i=0;i<ShaderParams.size();i++)
	{
		FX::ShaderParam *sp = &ShaderParams[i];
		if('\0'==sp->Cg_decoration
		||a.string[a.extent]!=sp->Cg_decoration) 
		continue;

		daeName b = *(daeStringRef*)&sp->Name; //HACK		

		//If the symbol was not found, then if it
		//matches the undecorated name, then must
		//be a false-positive decoration, or it's
		//better than nothing.
		if(a==b) return sp;
	}

	return nullptr;
}
void FX::Pass::Finalize()
{
	//Removed unused uniforms so they do not cause problems.
	for(size_t i=0;i<ShaderParams.size();)
	{
		FX::ShaderParam *sp = &ShaderParams[i];
		if(sp->Param_To==nullptr&&sp->SetData==nullptr)			
		{
			if(Cg==nullptr||sp->Cg_decoration=='\0')
			daeEH::Warning<<"Deleting unused shader uniform "<<sp->Name+1;
			if(Cg!=nullptr)
			daeEH::Warning<<"Deleting unused shader uniform "<<sp->Name+1<<" or "<<sp->Name+1<<sp->Cg_decoration;
			ShaderParams.erase(ShaderParams.begin()+i);
		}
		else i++;
	}	
	//Set the data-params to reference themself.	
	//Note: The World juggling makes pointing to
	//themselves more complicated than doing this.
	for(size_t i=0;i<ShaderParams.size();i++)
	{
		FX::ShaderParam *sp = &ShaderParams[i];
		if(sp->SetData!=nullptr) sp->Param_To = sp;
	}
}

//FX::Shader //FX::Shader //FX::Shader //FX::Shader //FX::Shader

extern bool cfxCOMMON_InitGLSL(GLenum,xs::string,GLuint&,xs::ID);

FX::Shader::Shader(FX::Pass *pass, FX::FX_Stage08 stageIn
,xs::ID prof, xs::string args, xs::ID f, xs::string src)
:Pass(pass),GLSL(),Unlinked_Cg()
{	
	if(nullptr==Pass->Cg) //GLSL?
	{
		GLenum stage = 0;
		switch(stageIn)
		{
		case stageIn.VERTEX: stage = GL_VERTEX_SHADER;		
		case stageIn.GEOMETRY: stage = GL_GEOMETRY_SHADER;
		case stageIn.FRAGMENT: stage = GL_FRAGMENT_SHADER;
		case stageIn.TESSELLATION: 
		
			//Does GLSL have single-stage tessellation?
			default: goto GLSL_tess;
		}
		if(cfxCOMMON_InitGLSL(stage,src,Pass->GLSL,Pass->Technique->Sid))
		Stage = stage;
		else Stage = 0; return;
	}
	
	CGGLenum stage; switch(stageIn) //Cg.
	{
	case stageIn.VERTEX: stage = CG_GL_VERTEX;
	case stageIn.FRAGMENT: stage = CG_GL_FRAGMENT;
	case stageIn.GEOMETRY: stage = CG_GL_GEOMETRY;
	case stageIn.TESSELLATION: GLSL_tess:
		
		stage = CG_GL_TESSELLATION_CONTROL;
		stage = CG_GL_TESSELLATION_EVALUATION;
		daeEH::Error<<"Encountered TESSELLATION shader.\n"
		"(It's unclear what COLLADA wants. Do you have some idea?)";

	default: new(this) FX::Shader(pass); return; //Error.
	}
	_InitCg(stage,prof,args,f,src);
}
FX::Shader::Shader(FX::Pass *pass, FX::Cg_Stage stageIn
,xs::ID prof, xs::string args, xs::ID f, xs::string src)
:Pass(pass),GLSL(),Unlinked_Cg()
{	
	CGGLenum stage = 
	stageIn==stageIn.VERTEX?CG_GL_VERTEX:CG_GL_FRAGMENT;	
	_InitCg(stage,prof,args,f,src);
}
void FX::Shader::_InitGLSL(GLenum stage, xs::ID prof, xs::string args, xs::ID f, xs::string src)
{
	//These should be empty and "main" respectively for OpenGL. Possibly if it went
	//via a different compiler framework?
	(void)args; (void)f; (void)prof;

	if(cfxCOMMON_InitGLSL(stage,src,Pass->GLSL,Pass->Technique->Sid))
	Stage = stage; else stage = 0;	
}
void FX::Shader::_InitCg(CGGLenum stage, xs::ID prof, xs::string args, xs::ID f, xs::string src)
{
	switch(stage)
	{
	case CG_GL_VERTEX: Stage = GL_VERTEX_SHADER; break;
	case CG_GL_FRAGMENT: Stage = GL_FRAGMENT_SHADER; break;
	case CG_GL_GEOMETRY: Stage = GL_GEOMETRY_SHADER; break;
	}

	//This had been an UNUSED data member.
	CGprofile profile;

	if(0&&prof!=nullptr&&prof[0]!='\0')
	{
		profile = cgGetProfile(prof);		
		if(CG_PROFILE_UNKNOWN==profile
		||!cgIsProfileSupported(profile))
		{
			daeEH::Warning<<"Cg did not enumerate profile string "<<prof<<"\n"
			"(Falling back to cgGLGetLatestProfile)";
			goto profile_unknown;
		}
	}
	else if(0) profile_unknown: //Assuming OpenGL.
	{
		profile = cgGLGetLatestProfile(stage);
	}
	else switch(stage) //The vp30/fp30 profiles just don't work.
	{
	case CG_GL_VERTEX: profile = CG_PROFILE_GLSLV; break;
	case CG_GL_GEOMETRY: profile = CG_PROFILE_GLSLG; break;
	case CG_GL_FRAGMENT: profile = CG_PROFILE_GLSLF; break;
	}
	
	//"" emits a warning?!
	if('\0'==args[0]) args = nullptr;

	xs::string args_0[2] = { args,nullptr };

	FX::Technique *tech = Pass->Technique;
	CGcontext context = tech->FindEffect()->Cg_Context;	

	//This had been a USED data member.
	//Now a GLSL handle is used instead.
	CGprogram program = nullptr;

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
		program = cgCreateProgram(context,cg_file_type,src,profile,f,args_0);		
		if(program!=nullptr)
		break;
		if(i==0) daeEH::Warning<<
		"(False alarm? Failed to load as binary shader. Trying to compile...)";
	}
	
	if(program==nullptr)
	{
		daeEH::Error<<"Failed to loaded Cg program.";
		Stage = 0; return;
	}
	
	
	if(Stage==GL_VERTEX_SHADER)
	{
		daeEH::Verbose<<"Loaded Cg vertex program "<<f;
		//state = cgGetNamedState(context,"VertexProgram");
	}
	else if(Stage==GL_GEOMETRY_SHADER)
	{
		daeEH::Verbose<<"Loaded Cg geometry program "<<f;
		//state = cgGetNamedState(context,"GeometryProgram");
	}
	else if(Stage==GL_FRAGMENT_SHADER)
	{
		daeEH::Verbose<<"Loaded Cg fragment program "<<f;
		//state = cgGetNamedState(context,"FragmentProgram");
	}
	else daeEH::Error<<"Loaded Cg ??? program "<<f;

	//Don't use daeEH::Warning.
	const char *listing = cgGetLastListing(context);
	if(listing!=nullptr&&'\0'!=listing[0])
	daeEH::Verbose<<"The Cg compiler warned:\n"<<listing;
	
	//Saving the intermediate program is simpler than making
	//it available to cgGetPassProgram.
	//CGstateassignment assignment = 
	//cgCreateStateAssignment(Pass->Cg,state);
	//cgSetProgramStateAssignment(assignment,program);	
	Unlinked_Cg = program;

	//////EXPERIMENTAL
	//This is because the old COLLADA-CTS requires lighting-normal
	//auto-generation, and the old demo.dae file wants tangents too.
	//If COLLADA wanted this kind of compression it could've said so.
	if(stage==CG_GL_VERTEX)
	for(CGparameter p=cgGetFirstParameter(program,CG_PROGRAM);p!=nullptr;)
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