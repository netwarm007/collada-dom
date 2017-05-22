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

int NumCgPrograms = 0;
enum{ NumCgProgramsN=10 };
CGprogram cgPrograms[NumCgProgramsN];
CGprofile cgVertexProfile,cgFragmentProfile;

//skin vertex program parameters 
CGparameter skinVertexModelViewProjMatrix;
CGparameter skinLightModelViewProjMatrix;
CGparameter skinVertexSkinMatrixStack;

//static vertex program parameters 
CGparameter staticVertexModelViewMatrix;
CGparameter staticVertexModelViewProjMatrix;
CGparameter staticVertexLightViewProjMatrix;

//phong fragment vertex program parameters 
CGparameter fragmentGlobalAmbient;
CGparameter fragmentLightColor;
CGparameter fragmentLightPosition;
CGparameter fragmentEyePosition;
CGparameter fragmentMtrlAmbient;
CGparameter fragmentMtrlDiffuse;
CGparameter fragmentMtrlSpecular;
CGparameter fragmentShininess;

//shadow fragment program 
CGparameter shadowFragmentLookupMap;
CGparameter shadowFragmentShadowMap;

GLuint ShadowLookupTexId = 0;

//static normalmap program parameters 
CGparameter staticNormalLightPosition;
CGparameter staticNormalEyePosition;
CGparameter staticNormalModelViewProj;

//fragment normalmap program parameters 
CGparameter fragmentNormalMap;
CGparameter fragmentNormalizeCubeMap;
CGparameter fragmentNormalMapMtrlAmbient;
CGparameter fragmentNormalMapGlobalAmbient;

GLuint FragmentNormalMapTexId = 0;
GLuint FragmentNormalizeCubeMapId = 0;

static void CrtCommongCg_ErrorCallback()
{
	CGerror err; 
	const char *string = cgGetLastErrorString(&err);
	const char *listing = cgGetLastListing(RT::Main.Cg.Context);
	if(RT::Main.Loading)
	daeEH::Error<<"Cg Error Detected: "<<string<<"...\n"<<listing;
	else assert(0);
}

static bool CrtCommongCg_CheckForCgError()
{
	CGerror err; 
	const char *string = cgGetLastErrorString(&err);
	if(err!=CG_NO_ERROR)
	{	 
		//Show A Message Box Explaining What Went Wrong
		daeEH::Error<<"**** Cg Error "<<string<<" *****\n"<<cgGetLastListing(RT::Main.Cg.Context);
		
		daeEH::Warning<<"**** Switching to Fixed Function OpenGL ***** ";
		
		RT::Main.Cg.DisableProfiles();
		RT::Main.Cg.Initialized = false; //OVERKILL??? 
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

void RT::Frame_Cg::Init()
{
	if(Initialized) return;

	cgSetErrorCallback(CrtCommongCg_ErrorCallback);

	//Create a context for the CG programs we are going to load and validate it was successful
	daeEH::Verbose<<"Creating Cg context.";
	Context = cgCreateContext();
	if(Context==nullptr)
	{
		daeEH::Error<<"Failed To Create Cg Context.";
		return;
	}
	
	#ifdef _DEBUG
	cgGLSetDebugMode(CG_TRUE);
	#endif
	//cgSetAutoCompile(Context,CG_COMPILE_MANUAL);
	//cgGLSetManageTextureParameters(Context,1);
	//This is probably good considering how the FX stuff does "Apply()"
	//and goes through all of the settings. The memory overhead is not
	//the issue. Even though it's yet one more shadow copy of the data.
	cgSetParameterSettingMode(Context,CG_DEFERRED_PARAMETER_SETTING);

	//Register GL states (ugly crashes if you don't do this)
	cgGLRegisterStates(Context);

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

	EnableProfiles(); //NEW
																	  
	//Check for errors
	static bool nonce = false; //HACK
	if(!nonce&&!CrtCommongCg_CheckForCgError())
	return;
	nonce = true; //Assuming lingering errors.
	
	//Suppress #include errors. There must be a <include> for each of
	//the suppresed directives, or the <shader> will unlikely compile.
	cgSetCompilerIncludeCallback(Context,CrtCommonCg_IncludeCallback);

	daeEH::Verbose<<"Cg context created."; Initialized = true;
}

bool RT::Frame_Cg::SetShadowMapFragmentProgram()
{
	if(!Initialized) return false;
	
	if(PhongFragmentShadowProgramId<0)
	{
		//add any other program loading here 
		//load the default Shadow fragment shader here 
		if(!LoadProgram(PhongFragmentShadowProgramId,"shadowFragment.cg",cgFragmentProfile))	
		return false;

		//get the look map param handle 
		shadowFragmentLookupMap = 
		cgGetNamedParameter(cgPrograms[PhongFragmentShadowProgramId],"LookupMap");
		daeEH::Verbose<<"sampler : got "<<shadowFragmentLookupMap;
		//daeEH::Verbose<<"LookupMap on unit "<<cgGLGetTextureEnum(shadowFragmentLookup)-GL_TEXTURE0;

		//get the shadow map param handle 
		shadowFragmentShadowMap = 
		cgGetNamedParameter(cgPrograms[PhongFragmentShadowProgramId],"shadow");
		daeEH::Verbose<<"fShadow : got "<<shadowFragmentShadowMap;
		//daeEH::Verbose<<"shadowmap on unit "<<cgGLGetTextureEnum(shadowFragmentShadow)-GL_TEXTURE0;

		//load the lookup jitter texture 
		assert(!"memory leak"); //delete tga?		
		RT::Texture *tga = RT::LoadTargaFromURI("legacy://COLLADA-RT/examples/Aniso2.tga");

		if(tga!=nullptr&&tga->Data!=nullptr)
		{
			glGenTextures(1,&ShadowLookupTexId);
			glBindTexture(GL_TEXTURE_2D,ShadowLookupTexId);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D,0,tga->Format,tga->Width,
			tga->Height,0,tga->Format,GL_UNSIGNED_BYTE,tga->Data);
		}
		else
		{
			daeEH::Verbose<<"Missing Lookup Texture Aniso.tga for shadow mapping lookup.";
			return false;
		}

		//COLLADA_RT_array_delete(data);
		cgGLSetTextureParameter(shadowFragmentLookupMap,ShadowLookupTexId);

		//Check for a Cg Error, If So switch to FixedFunction
		if(!CrtCommongCg_CheckForCgError()) return false;
	}

	//daeEH::Verbose<<"Setting Cg Fragment Shadow Program.";  

	//set the shadow map parameter 
	cgGLSetTextureParameter(shadowFragmentShadowMap,RT::Main.ShadowMap.Id);
	cgGLEnableTextureParameter(shadowFragmentLookupMap);
	cgGLEnableTextureParameter(shadowFragmentShadowMap);

	//Bind shadow Fragment Program to the Current State 
	cgGLBindProgram(cgPrograms[PhongFragmentShadowProgramId]); return true;
}
					 
bool RT::Frame_Cg::SetDefaultStaticProgram()
{
	if(!Initialized) return false;

	if(StaticDefaultProgramId<0) //SetupDefaultStaticProgram
	{
		//load the static vertex shader 
		if(!LoadProgram(StaticDefaultProgramId,"staticVertex.cg",cgVertexProfile))		
		return false;

		//Get Static Vertex Parameters 
		staticVertexModelViewProjMatrix = 
		cgGetNamedParameter(cgPrograms[StaticDefaultProgramId],"modelViewProj");
		staticVertexLightViewProjMatrix = 
		cgGetNamedParameter(cgPrograms[StaticDefaultProgramId],"lightViewProj");
		staticVertexModelViewMatrix = 
		cgGetNamedParameter(cgPrograms[StaticDefaultProgramId],"modelView");

		//Check for a Cg Error, If So switch to FixedFunction
		if(!CrtCommongCg_CheckForCgError()) return false;
	}
	
	//daeEH::Verbose<<"Setting Cg Static Program."; 

	//Bind Our Vertex Program To The Current State
	cgGLBindProgram(cgPrograms[StaticDefaultProgramId]);

	//Set The Modelview Matrix Of Our Shader To Our OpenGL Modelview Matrix
	cgGLSetStateMatrixParameter(staticVertexModelViewProjMatrix,CG_GL_MODELVIEW_PROJECTION_MATRIX,CG_GL_MATRIX_IDENTITY);	
	cgGLSetStateMatrixParameter(staticVertexModelViewMatrix,CG_GL_MODELVIEW_MATRIX,CG_GL_MATRIX_IDENTITY);	

	return true;
}
bool RT::Frame_Cg::SetDefaultSkinProgram()
{
	if(!Initialized) return false;

	if(SkinDefaultProgramId<0) //SetupDefaultSkinProgram
	{
		if(!LoadProgram(SkinDefaultProgramId,"skin4mVertex.cg",cgVertexProfile))
		return false;

		//Get Skin Vertex Parameters 
		skinVertexModelViewProjMatrix = 
		cgGetNamedParameter(cgPrograms[SkinDefaultProgramId],"modelViewProj");
		skinLightModelViewProjMatrix = 
		cgGetNamedParameter(cgPrograms[SkinDefaultProgramId],"lightViewProj");
		skinVertexSkinMatrixStack = 
		cgGetNamedParameter(cgPrograms[SkinDefaultProgramId],"boneMats");

		//Check for a Cg Error, If So switch to FixedFunction
		if(!CrtCommongCg_CheckForCgError()) return false;
	}
	
	//daeEH::Verbose<<"Setting Cg Skin Program."; 

	//Bind Our Vertex Program To The Current State
	cgGLBindProgram(cgPrograms[SkinDefaultProgramId]);

	//Set The Modelview Matrix Of Our Shader To Our OpenGL Modelview Matrix
	cgGLSetStateMatrixParameter(skinVertexModelViewProjMatrix,CG_GL_MODELVIEW_PROJECTION_MATRIX,CG_GL_MATRIX_IDENTITY);

	return true;
}
bool RT::Frame_Cg::SetPhongFragmentProgram()
{
	if(!Initialized) return false;

	if(PhongFragmentProgramId<0) //SetupDefaultFragmentProgram
	{
		if(!LoadProgram(PhongFragmentProgramId,"phongFragment.cg",cgFragmentProfile))
		return false;

		//Get Fragment Parameters 
		fragmentGlobalAmbient = 
		cgGetNamedParameter(cgPrograms[PhongFragmentProgramId],"globalAmbient");
		fragmentLightColor = 
		cgGetNamedParameter(cgPrograms[PhongFragmentProgramId],"lightColor");
		fragmentLightPosition = 
		cgGetNamedParameter(cgPrograms[PhongFragmentProgramId],"lightPosition");
		fragmentEyePosition = 
		cgGetNamedParameter(cgPrograms[PhongFragmentProgramId],"eyePosition");
		fragmentMtrlAmbient = 
		cgGetNamedParameter(cgPrograms[PhongFragmentProgramId],"mtrlAmbient");
		fragmentMtrlDiffuse = 
		cgGetNamedParameter(cgPrograms[PhongFragmentProgramId],"mtrlDiffuse");
		fragmentMtrlSpecular = 
		cgGetNamedParameter(cgPrograms[PhongFragmentProgramId],"mtrlSpecular");
		fragmentShininess =
		cgGetNamedParameter(cgPrograms[PhongFragmentProgramId],"shininess");

		//Check for a Cg Error, If So switch to FixedFunction
		if(!CrtCommongCg_CheckForCgError()) return false;
	}
	
	//daeEH::Verbose<<"Setting Cg Phong Program."; 
	//Bind out Fragment Program to the Current State 
	cgGLBindProgram(cgPrograms[PhongFragmentProgramId]);

	//!!!GAC This code is setup to only handle one light, so get the first instance from the scene
	//There should always be one
	RT::Stack_Data &light = RT::Main.Stack.FindAnyLight();
	FX::Float3 lightPosition(light.Matrix[M30],light.Matrix[M31],light.Matrix[M32]);

	//Get Fragment Parameters 
	RT::Material *material = RT::Main.Stack.CurrentMaterial;
	
	//Get the eye position from the camera instance's parent node's Matrix
	//REFACTOR IN PROGRESS we should have an accessor for getting the eye position
	RT::Stack_Data &eye = *RT::Main.Parent;
	FX::Float3 eyePosition(eye.Matrix[M30],eye.Matrix[M31],eye.Matrix[M32]);

	//Get the material parameters, establish some defaults for things without materials
	//!!!GAC maybe crtSceneRead should make a default material and bind polys without materials to it?
	FX::Float4 mtrlAmbientColor(0);
	FX::Float4 mtrlDiffuseColor(1);
	FX::Float4 mtrlSpecularColor(0.5,1);
	float mtrlShininess = 5;

	//Override defaults if there is a material.
	if(material!=nullptr)
	{
		mtrlAmbientColor = material->Ambient;
		mtrlDiffuseColor = material->Diffuse;
		mtrlSpecularColor = material->Specular;
		mtrlShininess = material->Shininess;
	}

	FX::Float4 lightColor = light.Node->Lights[0]->Color;

	//Set the light and material information in the shader
	cgGLSetParameter3fv(fragmentEyePosition,&eyePosition.x);
	//mtrl info 
	cgGLSetParameter3fv(fragmentMtrlAmbient,&mtrlAmbientColor.x);
	cgGLSetParameter3fv(fragmentMtrlDiffuse,&mtrlDiffuseColor.x);
	cgGLSetParameter3fv(fragmentMtrlSpecular,&mtrlSpecularColor.x);
	cgGLSetParameter1f(fragmentShininess,mtrlShininess);
	//light info 		
	cgGLSetParameter3fv(fragmentLightPosition,&lightPosition.x);
	cgGLSetParameter3fv(fragmentLightColor,&lightColor.x);
	//global info 
	FX::Float4 globalAmbient(0.2f,1);
	cgGLSetParameter3fv(fragmentGlobalAmbient,&globalAmbient.x);

	return CrtCommongCg_CheckForCgError();
}

bool RT::Frame_Cg::SetNormalMapStaticProgram()
{
	if(!Initialized) return false;

	if(StaticNormalMapId<0)
	{
		if(!LoadProgram(StaticNormalMapId,"normalMapVertex.cg",cgVertexProfile))
		return false;

		//CgStaticNormalMapId = LoadProgram( "NormalTest/bumpSimple.cg",cgVertexProfile);
		//get handles to the params 
		staticNormalLightPosition = 
		cgGetNamedParameter(cgPrograms[StaticNormalMapId],"lightPosition");
		staticNormalEyePosition = 
		cgGetNamedParameter(cgPrograms[StaticNormalMapId],"eyePosition");
		staticNormalModelViewProj = 
		cgGetNamedParameter(cgPrograms[StaticNormalMapId],"modelViewProj");

		//Check for a Cg Error, If So switch to FixedFunction
		if(!CrtCommongCg_CheckForCgError()) return false;
	}
	
	//!!!GAC This code is setup to only handle one light, so get the first instance from the scene
	//There should always be one
	RT::Stack_Data &light = RT::Main.Stack.FindAnyLight();
	FX::Float3 lightPosition(light.Matrix[M30],light.Matrix[M31],light.Matrix[M32]);

	//Get the eye position from the camera instance's parent node's Matrix
	//REFACTOR IN PROGRESS we should have an accessor for getting the eye position
	RT::Stack_Data &eye = *RT::Main.Parent;
	FX::Float3 eyePosition(eye.Matrix[M30],eye.Matrix[M31],eye.Matrix[M32]);

	//set the shadow map parameter 
	cgGLSetParameter3fv(staticNormalLightPosition,&lightPosition.x);
	cgGLSetParameter3fv(staticNormalEyePosition,&eyePosition.x);
	cgGLSetStateMatrixParameter(staticNormalModelViewProj,
	CG_GL_MODELVIEW_PROJECTION_MATRIX,CG_GL_MATRIX_IDENTITY);

	//Bind shadow Fragment Program to the Current State 
	cgGLBindProgram(cgPrograms[StaticNormalMapId]); return true;
}
bool RT::Frame_Cg::SetNormalMapFragmentProgram()
{
	if(!Initialized) return false;
	
	if(FragmentNormalMapId<0)
	{
		if(!LoadProgram(FragmentNormalMapId,"normalMapFragment.cg",cgFragmentProfile))
		return false;

		//get handles to the params 
		fragmentNormalMap = 
		cgGetNamedParameter(cgPrograms[FragmentNormalMapId],"normalMap");
		fragmentNormalizeCubeMap = 
		cgGetNamedParameter(cgPrograms[FragmentNormalMapId],"normalizeCube");
		fragmentNormalMapMtrlAmbient = 
		cgGetNamedParameter(cgPrograms[FragmentNormalMapId],"mtrlAmbient");
		fragmentNormalMapGlobalAmbient = 
		cgGetNamedParameter(cgPrograms[FragmentNormalMapId],"globalAmbient");

		assert(!"memory leak"); //delete tga?
		RT::Texture *tga = RT::LoadTargaFromURI
		("legacy://COLLADA-RT/images/SeymourPlane140AnimLight_Textures/planeNormalMap.tga");

		if(tga!=nullptr&&tga->Data!=nullptr)
		{
			glGenTextures(1,&FragmentNormalMapTexId);
			glBindTexture(GL_TEXTURE_2D,FragmentNormalMapTexId);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		
			glTexImage2D(GL_TEXTURE_2D,0,tga->Format,tga->Width,
			tga->Height,0,tga->Format,GL_UNSIGNED_BYTE,tga->Data);
		}
		else
		{
			daeEH::Verbose<<"Normal map planeNormalMap.tga isn't the 24 or 32 bit or was not found.";
			return false;
		}
		cgGLSetTextureParameter(fragmentNormalMap,FragmentNormalMapTexId);

		//Check for a Cg Error, If So switch to FixedFunction
		if(!CrtCommongCg_CheckForCgError()) return false;
	}

	//cgGLSetTextureParameter(fragmentNormalMap,FragmentNormalMapId);
	//cgGLEnableTextureParameter(fragmentNormalMap);

	#ifdef NDEBUG
	#error Whill this reset back to GL_TEXTURE0?
	#endif
	GL.ActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,FragmentNormalMapId);

	//Get Fragment Parameters 
	RT::Material *mat = RT::Main.Stack.CurrentMaterial;
	FX::Float3 mtrlAmbientColor;
	if(mat!=nullptr) mtrlAmbientColor = mat->Ambient;

	cgGLSetParameter3fv(fragmentNormalMapMtrlAmbient,&mtrlAmbientColor.f0);
	//global info 
	#ifdef NDEBUG
	#error Should this be setting alpha to 20%?
	#endif
	FX::Float4 globalAmbient(0.2f); //Alpha??? Really?
	cgGLSetParameter3fv(fragmentNormalMapGlobalAmbient,&globalAmbient.f0);

	//Bind normal fragment Program to the Current State 
	cgGLBindProgram(cgPrograms[FragmentNormalMapId]); return true;
}

void RT::Frame_Cg::EnableProfiles()
{
	if(!Initialized) return;
	
	cgGLEnableProfile(cgVertexProfile);
	cgGLEnableProfile(cgFragmentProfile);
}
void RT::Frame_Cg::DisableProfiles()
{
	if(!Initialized) return;
	
	cgGLDisableProfile(cgVertexProfile);
	cgGLDisableProfile(cgFragmentProfile);
}
 
void RT::Frame_Cg::Reset()
{
	daeEH::Verbose<<"Resetting Cg context.";

	DisableProfiles(); Destroy(true);

	Initialized = false; Init();
}
void RT::Frame_Cg::Destroy(bool resetting)
{
	if(!Initialized||Context==nullptr) return;	

	if(!resetting)
	daeEH::Verbose<<"Destroying Cg context.";

	cgDestroyContext(Context); Context = nullptr; 
}

bool RT::Frame_Cg::LoadProgram(int &out, const char *fileName, CGprofile programType)
{
	if(!Initialized||NumCgPrograms==NumCgProgramsN){ assert(0); return false; }

	if(out==-2) return false;
	out = -2; //Assume failed to compile.
	
	daeEH::Verbose<<"Loading "<<fileName<<" shader from file.";
	char fullFileName[64] = "legacy://COLLADA-RT/shaders/advanced/";
	COLLADA_SUPPRESS_C(4996)
	char *PS3_ext = strcat(fullFileName,fileName);		
	CGenum PS3_bin = CG_SOURCE;
	#ifdef SN_TARGET_PS3
	{
		//Did Sony really never get around to this?
		assert(0); if(0)
		{	
			//Note: CG_BINARY is undocumented. Is CG_OBJECT intended?
			PS3_bin = CG_BINARY;
			PS3_ext+=strlen(fullFileName)-sizeof(".cg")-2;
			memcpy(PS3_ext,programType==cgFragmentProfile?"fpo":"vpo",3);
		}
	}
	#endif
	RT::Resource file(fullFileName);
	if(file.Size==0)
	return false;

	//Load And Compile The Vertex Shader From File
	cgPrograms[NumCgPrograms] =
	cgCreateProgram(Context,PS3_bin,file.Data,programType,"main",0);

	//Validate Success
	if(cgPrograms[NumCgPrograms]==nullptr)
	{
		daeEH::Error<<"Failed to compile Cg file "<<fileName;
		//Check for a Cg Error, If So switch to FixedFunction
		CrtCommongCg_CheckForCgError(); return false;
	}

	daeEH::Verbose<<"Shader ID "<<cgPrograms[NumCgPrograms];

	cgGLLoadProgram(cgPrograms[NumCgPrograms]); 
	
	out = NumCgPrograms++; return true;
}

void RT::Frame_Cg::SetPassState(FX::Material *fx, int pass)
{	
	//Pushes the setparam values into cgFX for this material
	fx->Apply();

	//SCHEDULED FOR REMOVAL
	CGeffect Cg = fx->FindEffect()->Cg;

	#ifdef NDEBUG
	#error These can be global cgCreateParameter parameters.
	#endif
	//See if there are any common semantic Parameters we need to set
	CGparameter worldviewprojectionParam = cgGetEffectParameterBySemantic(Cg,"WORLDVIEWPROJECTION");
	CGparameter viewinverseParam = cgGetEffectParameterBySemantic(Cg,"VIEWINVERSE");
	CGparameter worldParam = cgGetEffectParameterBySemantic(Cg,"WORLD");
	CGparameter worldinversetransposeParam = cgGetEffectParameterBySemantic(Cg,"WORLDINVERSETRANSPOSE");
	CGparameter lightPositionParam = cgGetEffectParameterBySemantic(Cg,"LIGHTPOSITION");
	//UNUSED?
	CGparameter timeParam = cgGetEffectParameterBySemantic(Cg,"TIME");
	//TIME gets a tickcount for driving animated shaders
	if(timeParam!=nullptr)
	cgSetParameter1f(timeParam,std::max<float>(0,RT::Main.Time));	
	 	
	#ifdef NDEBUG
	#error Use cgCreateParameter to set these at the top of the draw loop.
	#endif
	{	
		//VIEWINVERSE is the inverse of the view matrix which is the same as the camera's Matrix
		if(viewinverseParam!=nullptr)
		{
			#if 2==COLLADA_DOM_PRECISION
			cgGLSetMatrixParameterdc(viewinverseParam,_InverseViewMatrix);
			#else
			cgGLSetMatrixParameterfc(viewinverseParam,_InverseViewMatrix);
			#endif
		}
		//LIGHTPOSITION gets the position of the primary (nearist) light
		if(lightPositionParam!=nullptr)
		{
			//There should always be one
			RT::Matrix &light = RT::Main.Stack.FindAnyLight().Matrix;
			FX::Float4 position(light[M30],light[M31],light[M32],0);
			CGtype lighttype = cgGetParameterType(lightPositionParam);
			if(lighttype==CG_FLOAT3) cgSetParameter3fv(lightPositionParam,&position.x);
			if(lighttype==CG_FLOAT4) cgSetParameter4fv(lightPositionParam,&position.x);
		}
	}

	//WORLDVIEWPROJECTION is the world+view+projection matrix of this object which we get from GL
	if(worldviewprojectionParam!=nullptr)
	cgGLSetStateMatrixParameter(worldviewprojectionParam,CG_GL_MODELVIEW_PROJECTION_MATRIX,CG_GL_MATRIX_IDENTITY);
	
	//WORLD is the localtoworld matrix for this object which we get from the scene graph
	if(worldParam!=nullptr)
	{
		#if 2==COLLADA_DOM_PRECISION
		cgGLSetMatrixParameterdc(worldParam,*_WorldMatrix);
		#else
		cgGLSetMatrixParameterfc(worldParam,*_WorldMatrix);
		#endif
	}

	//WORLDINVERSETRANSPOSE is inversetransposelocaltoworld matrix for this object from the scene graph
	if(worldinversetransposeParam!=nullptr)
	{
		//Assuming for surface normals:
		//There's not code on hand for proper 4x4 inversions.
		RT::Matrix it; RT::MatrixInvertTranspose0(*_WorldMatrix,it);
		#if 2==COLLADA_DOM_PRECISION
		cgGLSetMatrixParameterdc(worldinversetransposeParam,it);
		#else
		cgGLSetMatrixParameterfc(worldinversetransposeParam,it);
		#endif
	}
	 
	//Setup the state for the FX::Material
	fx->SetPassState(pass);
}

//-------.
	}//<-'
}

/*C1071*/