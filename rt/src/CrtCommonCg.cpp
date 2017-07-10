/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <RT.pch.h> //PCH

#include "CrtRender.h"
#include "CrtLight.h"
#include "CrtEffect.h"
#include "CrtTexture.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'

//UNUSED
CGprofile cgVertexProfile;
CGprofile cgFragmentProfile;

static void CrtCommongCg_ErrorCallback()
{
	CGerror err; 
	const char *string = cgGetLastErrorString(&err);
	const char *listing = cgGetLastListing(RT::Main.FX.Cg);
	if(RT::Main.Loading)
	daeEH::Error<<"Cg Error Detected: "<<string<<"...\n"<<listing;
	else if(nullptr==RT::Main.DB) 
	{
		//FreeGLUT raises "The profile is not supported."
		//CG_INVALID_PROFILE_ERROR on termination???
	}
	else assert(0); 
}

static bool CrtCommongCg_CheckForCgError()
{
	CGerror err; 
	const char *string = cgGetLastErrorString(&err);
	if(err!=CG_NO_ERROR)
	{	 
		//Show A Message Box Explaining What Went Wrong
		daeEH::Error<<"**** Cg Error "<<string<<" *****\n"<<cgGetLastListing(RT::Main.FX.Cg);
		
		daeEH::Warning<<"**** Switching to Fixed Function OpenGL ***** ";
		
		RT::Main.FX.Initialized = false; //OVERKILL??? 
		return false;
	}
	return true;
}

static void CGENTRY CrtCommonCg_IncludeCallback(CGcontext c, const char *x)
{
	daeEH::Warning<<"Ignoring Cg program #include directive for "<<x<<"\n"
	"(This is by design. cgCreateProgram cannot do it. What compiler can?)";
	cgSetCompilerIncludeString(c,x,"\n");
}

void RT::Frame_FX::Init()
{
	if(Initialized) return;

	cgSetErrorCallback(CrtCommongCg_ErrorCallback);

	//Create a context for the CG programs we are going to load and validate it was successful
	daeEH::Verbose<<"Creating Cg context.";
	Cg = cgCreateContext();
	if(Cg==nullptr)
	{
		daeEH::Error<<"Failed To Create Cg Context.";
		return;
	}
	
	#ifdef _DEBUG
	cgGLSetDebugMode(CG_TRUE);
	#endif

	//This matters much less since the FX framework now relies
	//very little on the Cg API. 
	#ifdef NDEBUG
	#error Try using cgUpdateProgramParameters with DEFERRED.
	#endif
	cgSetParameterSettingMode(Cg,CG_IMMEDIATE_PARAMETER_SETTING);
	cgSetAutoCompile(Cg,CG_COMPILE_MANUAL);
	//cgGLSetManageTextureParameters(Cg,1);

	//Register GL states (ugly crashes if you don't do this)
	cgGLRegisterStates(Cg);

	//Get The Latest GL Vertex Profile
	#ifdef SN_TARGET_PS3
	//Was hardcoded to CG_PROFILE_SCE_VP_TYPEB
	//Was hardcoded to CG_PROFILE_SCE_FP_TYPEB
	#endif
	
	//This fix is required for non-Nvidia or for
	//onboard GPUs (like my own Intel Iris Pro.)
	#ifdef NDEBUG
	#error Scan code for NORMAL0->NORMAL if keeping Cg.
	#endif
	//These return arbvp1 and arbfp1 but vp30, etc. if
	//required will not work, whereas the GLSL targets do.
	//This link describes this:
	//https://bugs.launchpad.net/panda3d/+bug/1154064
	//Including the NORMAL0->NORMAL thing.
	//cgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
	//cgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgVertexProfile = CG_PROFILE_GLSLV;
	cgFragmentProfile = CG_PROFILE_GLSLF;
	//Necessary? //EnableProfiles();
	cgGLEnableProfile(cgVertexProfile);
	cgGLEnableProfile(cgFragmentProfile);

	/*//Validate Our Profile Determination Was Successful
	if(cgVertexProfile==CG_PROFILE_UNKNOWN||cgFragmentProfile==CG_PROFILE_UNKNOWN)
	{
		#ifdef NDEBUG
		#error Depending on Error this could be double-fired.
		#endif
		daeEH::Error<<"Invalid profile type returned from cgGLGetLatestProfile.";		
		return;
	}*/

	//Set The Current Profile
	cgGLSetOptimalOptions(cgVertexProfile);
	cgGLSetOptimalOptions(cgFragmentProfile);	
																	  
	//Check for errors
	static bool nonce = false; //HACK
	if(!nonce&&!CrtCommongCg_CheckForCgError())
	return;
	nonce = true; //Assuming lingering errors.
	
	//Suppress #include errors. There must be a <include> for each of
	//the suppresed directives, or the <shader> will unlikely compile.
	cgSetCompilerIncludeCallback(Cg,CrtCommonCg_IncludeCallback);
						   	
	daeEH::Verbose<<"Cg context created."; Initialized = true;
}
 
void RT::Frame_FX::Reset()
{
	daeEH::Verbose<<"Resetting Cg context.";

	Destroy(true);

	Initialized = false; Init();
}
void RT::Frame_FX::Destroy(bool resetting)
{
	if(!Initialized||Cg==nullptr) return;	

	if(!resetting)
	daeEH::Verbose<<"Destroying Cg context.";

	cgDestroyContext(Cg); Cg = nullptr; 
}
		  
void RT::Frame_FX::Reset_Context()
{	
	TIME.Value = std::max<float>(0,RT::Main.Time);	
	
	#if 1==COLLADA_DOM_PRECISION
	glGetFloatv(GL_PROJECTION_MATRIX,RT::Main.FX.PROJECTION.Value);
	#else
	glGetDoublev(GL_PROJECTION_MATRIX,RT::Main.FX.PROJECTION.Value);
	#endif	
}
bool RT::Frame_FX::SetWorld(RT::Effect_Semantics semantics, RT::Matrix &w, RT::Matrix &wv)
{	
	//Reminder: This list is incomplete.
	if(0==semantics.Semantics()||!RT::Main.ShowCOLLADA_FX) 
	return false; 

	if(&w!=&WORLD.Value&&0!=semantics.WORLD)
	RT::MatrixCopy(w,WORLD.Value);
	
	if(0!=semantics.WORLD_INVERSE_TRANSPOSE)
	if(&w==&IDENTITY.Value)
	RT::MatrixLoadIdentity(WORLD_INVERSE_TRANSPOSE.Value);
	else
	RT::MatrixInvertTranspose0(w,WORLD_INVERSE_TRANSPOSE.Value);

	if(0!=semantics.WORLD_VIEW&&&wv!=&WORLD_VIEW.Value)	
	RT::MatrixCopy(wv,WORLD_VIEW.Value);

	if(0!=semantics.WORLD_VIEW_INVERSE_TRANSPOSE)	
	RT::MatrixInvertTranspose0(wv,WORLD_VIEW_INVERSE_TRANSPOSE.Value);

	if(0!=semantics.WORLD_VIEW_PROJECTION)
	RT::MatrixMult(wv,PROJECTION.Value,WORLD_VIEW_PROJECTION.Value); 
	
	return semantics.Missing_FX==0;
}

//-------.
	}//<-'
}

/*C1071*/