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

class Shader : public FX::Annotatable
{
COLLADA_(public)

	FX::Pass *Pass;

	/**
	 * It's tricky to get an OpenGL handle for the Cg 
	 * objects.
	 */
	GLuint GLSL; CGprogram Unlinked_Cg;

	/**
	 * The loader uses these to priortize vertex shaders 
	 * and the WORLD family of semantics.
	 */
	GLenum Stage;

	bool operator<(const FX::Shader &cmp)const
	{
		if(Stage==GL_VERTEX_SHADER) 
		return true;
		return cmp.Stage==GL_FRAGMENT_SHADER;
	}

	//SCHEDULED FOR REMOVAL
	CGdomain Stage_Cg()
	{
		switch(Stage)
		{
		default: case 0: return CG_UNKNOWN_DOMAIN;
		case GL_VERTEX_SHADER: return CG_VERTEX_DOMAIN;
		case GL_GEOMETRY_SHADER: return CG_GEOMETRY_DOMAIN;
		case GL_FRAGMENT_SHADER: return CG_FRAGMENT_DOMAIN;
		}
	}

COLLADA_(public)

	Shader(FX::Pass *p):Pass(p),GLSL(),Unlinked_Cg()
	{}
	Shader(FX::Pass*,FX::Cg_Stage,xs::ID prof, xs::string args, xs::ID f, xs::string src);
	Shader(FX::Pass*,FX::GLSL_Stage,xs::ID,xs::string,xs::ID,xs::string);
	Shader(FX::Pass*,FX::FX_Stage08,xs::ID,xs::string,xs::ID,xs::string);	
	void _InitCg(CGGLenum,xs::ID,xs::string,xs::ID,xs::string);
	void _InitGLSL(GLenum,xs::ID,xs::string,xs::ID,xs::string);
};

class Pass : public FX::Annotatable
{
COLLADA_(public)

	FX::Technique *Technique;
		
	std::vector<FX::Shader*> Shaders;

	std::vector<FX::ShaderParam> ShaderParams;
	/**
	 * @c FX::Loader uses this to match names against Cg's
	 * quirky decorations.
	 */
	FX::ShaderParam *FindShaderParam(xs::ID);
	/**
	 * @c FX::Loader uses this to erase any unused uniform
	 * variables and to set the data-params to loopback on
	 * themselves. (Containers of self-referencing objects
	 * is a classical problem that @c Finalize sidesteps.)
	 */
	void Finalize();

	/**
	 * Previously "pass." 
	 * Capitalizing "pass" conflicts with the constructor.
	 * (All Cg variables should be distinguished eventually.)
	 */
	CGpass Cg;

	CGprogram Linked_Cg; GLuint GLSL; void Link();

	int World;

COLLADA_(public)

	Pass(FX::Technique *technique, xs::ID sid)
	:Technique(technique),Cg(),Linked_Cg(),GLSL()
	{
		if(Technique->Cg!=nullptr)
		Cg = cgCreatePass(Technique->Cg,sid);
	}
	~Pass()
	{
		for(size_t i=0;i<Shaders.size();i++)
		delete Shaders[i];

		//These are owned by Cg.
		if(Cg==nullptr&&0!=GLSL) GL.DeleteProgram(GLSL);
	}
};

//-------.
	}//<-'
}

#endif //
/*C1071*/