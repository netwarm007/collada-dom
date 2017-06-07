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

void FX::Annotate::_InitSAS()
{
	SAS = (FX::SAS)INT_MAX;

	if(CG_STRING==OwnData().GetType())
	{
		if("Space"==Name||"space"==Name)		
		{					
			daeName Space = Value<daeName>();

			if(Space=="World"||Space=="world")
			SAS = FX::string_Space_World;
			if(Space=="View"||Space=="view")
			SAS = FX::string_Space_World;
			return;
		}
	}
}

//FX::Loader calls this after the annotations are loaded.
FX::Semantic FX::NewParam::_InitSemantic_etc()
{ 
	//HACK: The Loader sometimes supplies "" as a default
	//that is not a daeStringRef.
	if('\0'==SEMANTIC[0]) return Semantic;

	//HACK: Make an all caps string-ref if not all caps.
	daeStringRef &ref = daeOpaque(&SEMANTIC);
	size_t i,iN = ref.size();
	
	xs::ID e = ref+iN;
	while(e-1>=ref&&isdigit(e[-1])) 
	{
		e--; iN--;
	}
	Subscript = atoi(e);
	while(e>ref&&*e=='_') 
	{
		e--; iN--;
	}
	if(*e!='\0') goto trim_subscript; //Normalize?

	for(i=0;i<iN;i++) if(ref[i]!=toupper(ref[i]))
	{
		trim_subscript: //This is for sorting by array indices.

		enum{ small_string=49 }; if(iN<=small_string)
		{
			daeStringCP buf[small_string+1];
			for(i=0;i<iN;i++) buf[i] = toupper(ref[i]);			
			//HACK: increment ref so it's technically not zero.
			new(&ref) daeStringRef(ref); ref.setString(buf,iN); 
		}
		else assert(0); break;
	}

	SEMANTIC = ref;

	daeName base(SEMANTIC,iN);

	switch(base[0])
	{
	case 'A':

		if("AMBIENT"==base)
		return Semantic = FX::AMBIENT; 
		break;

	case 'D':

		if("DIFFUSE"==base)
		return Semantic = FX::DIFFUSE;
		break;

	case 'E':

		if("EMISSION"==base
		||"EMISSIVE"==base) //Direct3D.
		return Semantic = FX::EMISSION;					
		break;

	case 'L':

		if("LIGHTDIR"==base
		||"LIGHTDIRECTION"==base
		||"LIGHT_DIRECTION"==base)
		if(SAS_substring("Spot"))
		return Semantic = FX::SPOT_DIRECTION;
		else //Assuming directional lighting.
		return Semantic = FX::LIGHT_POSITION;

		if("LIGHTPOS"==base
		||"LIGHTPOSITION"==base
		||"LIGHT_POSITION"==base)
		return Semantic = FX::LIGHT_POSITION;
		break;

	case 'M': //WORLD-VIEW v. MODEL-VIEW.

		//Are these synonymous to "world"?

		if("MODEL"==base)
		return Semantic = FX::WORLD;		

		if("MODELINVERSETRANSPOSE"==base
		 ||"MODEL_INVERSE_TRANSPOSE"==base)
		return Semantic = FX::WORLD_INVERSE_TRANSPOSE;

		//These are commonly said phrases.

		if("MODELVIEW"==base
		 ||"MODEL_VIEW"==base)
		return Semantic = FX::WORLD_VIEW;

		if("MODELVIEWINVERSETRANSPOSE"==base
		 ||"MODEL_VIEW_INVERSE_TRANSPOSE"==base)
		return Semantic = FX::WORLD_VIEW_INVERSE_TRANSPOSE;		

		if("MODELVIEWPROJECTION"==base
		 ||"MODEL_VIEW_PROJECTION"==base)
		return Semantic = FX::WORLD_VIEW_PROJECTION;		
		break;

	case 'O':

		if("OPAQUE"==base
		||"OPACITY"==base
		||"OPAQUENESS"==base)
		return Semantic = FX::OPACITY;

	case 'P':

		if("POSITION"==base)
		if(SAS_substring("Light"))
		return Semantic = FX::LIGHT_POSITION;
		else
		return Semantic = FX::POSITION;

		if("POWER"==base)
		if(SAS_substring("Specular"))
		return Semantic = FX::SHININESS;
		else
		return Semantic = FX::POWER;

		if("PROJECTION"==base)
		return Semantic = FX::PROJECTION;
		break;
	
	case 'S':

		if("SHININESS"==base
		||"SPECULARPOWER"==base
		||"SPECULAR_POWER"==base)
		return Semantic = FX::SHININESS;

		if("SPECULAR"==base)
		if(SAS_substring("Power"))
		return Semantic = FX::SHININESS;
		else
		return Semantic = FX::SPECULAR;

		if("SPOTDIR"==base		
		||"SPOT_DIRECTION"==base
		||"SPOTLIGHT_DIRECTION"==base
		||"SPOT_LIGHT_DIRECTION"==base)
		return Semantic = FX::SPOT_DIRECTION;
		break;

	case 'T':

		if("TIME"==base
		||"TIMER"==base)
		return Semantic = FX::TIME;

		if("TRANSPARENT"==base
		||"TRANSPARENCY"==base)
		return Semantic = FX::TRANSPARENCY;
		break;

	case 'V':

		if("VIEW"==base)
		return Semantic = FX::VIEW;

		if("VIEWINVERSE"==base
		 ||"VIEW_INVERSE"==base)
		return Semantic = FX::VIEW_INVERSE;
		break;

	case 'W': 

		if("WORLD"==base)
		return Semantic = FX::WORLD;		

		if("WORLDINVERSETRANSPOSE"==base
		 ||"WORLD_INVERSE_TRANSPOSE"==base)
		return Semantic = FX::WORLD_INVERSE_TRANSPOSE;

		if("WORLDVIEW"==base
		 ||"WORLD_VIEW"==base)
		return Semantic = FX::WORLD_VIEW;

		if("WORLDVIEWINVERSETRANSPOSE"==base
		 ||"WORLD_VIEW_INVERSE_TRANSPOSE"==base)
		return Semantic = FX::WORLD_VIEW_INVERSE_TRANSPOSE;		

		if("WORLDVIEWPROJECTION"==base
		 ||"WORLD_VIEW_PROJECTION"==base)
		return Semantic = FX::WORLD_VIEW_PROJECTION;		
		break;
	}

	assert(0); //Implement me.
	return Semantic; //breakpoint
}
FX::NewParam::NewParam(FX::DataMaker<FX::NewParam> *cp)
:ClientData(&OwnData())
{
	//Only <newparam> names do not begin with #.
	Name+=1; 

	//FX::Loader calls _InitSemantic().
	//cfxParam_semantics(*this,cp->aux);
	Semantic = (FX::Semantic)INT_MAX; Subscript = 0; SEMANTIC = cp->aux;	

	daeEH::Verbose<<"New param "<<Name;
}
FX::SetParam::SetParam(FX::DataMaker<FX::SetParam> *cp)
{
	//Assuming the parameter to set is not located in the
	//setting parameter's scope.
	if(!cp->p->FindNewParam(cp->sid,ParamToSet))
	{
		ParamToSet = this;
		daeEH::Warning<<"Did not match <setparam> "<<cp->sid<<"\n"
		"(The specification says this is a speculative and so OK.)";
	}
	else if(((FX::NewParam*)ParamToSet)->IsClientData())
	{
		//Should this be a run-time option?
		ParamToSet = this; 
		daeEH::Warning<<"Disabling client-data linked <setparam> "<<cp->sid<<"\n"
		"(Should this be a run-time option?)";
	}
	else daeEH::Verbose<<"Set param "<<cp->sid;
}
FX::SetParam_To::SetParam_To(xs::ID ref, FX::Paramable *p, xs::token ref2)
{
	SetData = nullptr;
	Name = ref; if(ref[0]!='#') Name--;
	Name_To = ref2; if(ref2[0]!='#') Name_To--;

	ParamToSet = Param_To = this; 
	if(!p->FindNewParam(ref2,Param_To))
	{
		//HACK: Try to shoehorn in <surface>.
		//HACK: Try to shoehorn in <surface>.
		//HACK: Try to shoehorn in <surface>.
		FX::Surface *s = p->FindSurface(ref2);
		FX::Surface *t = p->FindSurface(ref);
		if(s!=nullptr&&t!=nullptr)
		{
			//This SetParam_To is basically going unused.
			FX::Surface *u = new FX::Surface(s->TexId,t);
			p->Surfaces.push_back(std::make_pair(ref,u));
			return;
		}

		daeEH::Warning<<"Did not match <connect_param> "<<ref2<<"\n"
		"(The specification says this is a speculative and so OK.)";
	}
	if(!p->FindNewParam(ref,ParamToSet))
	{
		daeEH::Warning<<"Did not match <setparam> "<<ref<<"\n"
		"(The specification says this is a speculative and so OK.)";
	}
	else if(((FX::NewParam*)ParamToSet)->IsClientData())
	{
		//Should this be a run-time option?
		ParamToSet = Param_To = this; 
		daeEH::Warning<<"Disabling client-data linked <setparam> "<<ref<<"\n"
		"(Should this be a run-time option?)";
	}
} 
SetParam_To::SetParam_To(FX::NewParam *COMMON, FX::Technique *tech, xs::ID ref)
:ParamToSet(this),Param_To(this)
{
	Name_To = ref; if(ref[0]!='#') Name_To-=1;
	Name = COMMON->Name-1; assert(Name[0]=='#');	
	if(!tech->FindNewParam(ref,Param_To))
	daeEH::Warning<<"Did not match profile_COMMON texture-or-color to <newparam> "<<ref;	
	else ParamToSet = COMMON;
}

//NEW: Most of the work for this type is being done by 
//FX::Pass::Link() and FX::Loader.
void FX::ShaderParam::SetParam_To(FX::Pass *sh, xs::ID ref)
{	
	Name_To = ref; if(ref[0]!='#') Name_To-=1;
	if(!sh->Technique->FindNewParam(ref,Param_To))
	daeEH::Warning<<"Did not match <bind> to <newparam> "<<ref;	
}

//-------.
	}//<-'
}

/*C1071*/