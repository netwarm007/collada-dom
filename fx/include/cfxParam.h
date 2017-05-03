/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_FX__PARAM_H__
#define __COLLADA_FX__PARAM_H__

#include "cfxData.h"
#include "cfxAnnotate.h"

COLLADA_(namespace)
{
	namespace FX
	{//-.
//<-----'

/**wARNING
 * @warning The current implementation merges the
 * @c FX::Param to the @c FX::Data because it's a
 * chicken/egg situation. It doesn't hurt to fuse
 * the @c new and @c delete operation either. The
 * same goes for @c FX::Annotate.
 */
class Param
{
COLLADA_(public)
	/**
	 * This is part of the @c Param memory block.
	 * I.e. It could be a @c virtual method also.
	 */
	FX::Data *Data;

	xs::ID Name;

	CGparameter Cg;

COLLADA_(public)

	typedef xs::ID arg2;
	Param():Data(Data),Name(Name),Cg(Cg)
	{ /*NOP*/ }
	virtual ~Param(){}

	virtual void Apply() = 0;
};

class Paramable
{
COLLADA_(public)

	std::vector<FX::Param*> Params;

COLLADA_(public)
		
	~Paramable()
	{
		for(size_t i=0;i<Params.size();i++)
		delete Params[i];	
	}

	void Apply()
	{
		for(size_t i=0;i<Params.size();i++)	
		Params[i]->Apply();
	}
};

/**
 * @see @c FX::NewParamable.
 *
 * @note For whatever reason <surface> isn't
 * treated like the @c FX::Data based values.
 */
class Surface
{	
COLLADA_(public)
	
	GLuint TexId;

	FX::Surface *Parent_Surface;

	//THIS LOOKS LIKE A BAD DESIGN PATTERN.
	//THIS LOOKS LIKE A BAD DESIGN PATTERN.
	//THIS LOOKS LIKE A BAD DESIGN PATTERN.
	std::vector<FX::Param*> Referencing_Samplers;

COLLADA_(public)

	Surface(GLuint TexId, FX::Surface *parent=nullptr)
	:TexId(TexId),Parent_Surface(parent)
	{
		//SCHEDULED FOR REMOVAL
		if(TexId!=0) while(parent!=nullptr)
		{
			//assert(0==parent->TexId);
			parent->TexId = TexId;
			parent = parent->Parent_Surface;
		}
	}

	void Apply()
	{
		////2017: It doesn't make sense to only "Apply" the 
		////parent surface's params. What the hell's it for
		////if not that???
		if(Parent_Surface!=nullptr)
		{
			Parent_Surface->TexId = TexId;
			Parent_Surface->Apply();
		}
		////else //this is the parent surface
		{
			//this goes through and calls apply on all the params which reference this surface
			//this causes them to update their init_from and texture object to support setparam of surfaces
			for(size_t i=0;i<Referencing_Samplers.size();i++)
			Referencing_Samplers[i]->Apply();	
		}
	}
};
/**
 * This is added so @c FX::Technique can contain surface 
 * <newparam> information.
 */
class NewParamable : public Paramable
{
COLLADA_(public)
	/**	 
	 * This can be one of <effect> <profile*> <technique>
	 */
	FX::NewParamable *Parent_NewParamable;

	std::vector<std::pair<xs::ID,FX::Surface*>> Surfaces;

COLLADA_(public)

	NewParamable(FX::NewParamable *parent)
	:Parent_NewParamable(parent)
	{}
	~NewParamable()
	{
		for(size_t i=0;i<Surfaces.size();i++)
		delete Surfaces[i].second;	
	}

	//REMINDER: This does <newparam><setparam> etc.
	//Cg states may be set and then reset. Maybe if
	//the resulting state can be saved the materials
	//can be swapped out speedier.
	/**RECURSIVE
	 * This is new. The pre-2017 code looks like it
	 * was not used with hierarchical effect set ups.
	 */
	void Apply()
	{
		//Build up the hierarchy in layers.
		//Can Cg save the resulting state???
		if(nullptr!=Parent_NewParamable)
		Parent_NewParamable->Apply();

		Paramable::Apply();
		for(size_t i=0;i<Surfaces.size();i++)
		Surfaces[i].second->Apply();
	}

	FX::Effect *FindEffect()
	{
		if(nullptr==Parent_NewParamable) 
		return (FX::Effect*)this;
		return Parent_NewParamable->FindEffect();
	}

	FX::Surface *FindSurface(xs::ID sid)
	{
		for(size_t i=0;i<Surfaces.size();i++)		
		if(sid==Surfaces[i].first)		
		return Surfaces[i].second;
		if(Parent_NewParamable!=nullptr) 
		return Parent_NewParamable->FindSurface(sid);
		return nullptr;
	}

	/**
	 * Gets <param> for cgSet*ArrayStateAssignment
	 * since Cg doesn't have APIs for this and the
	 * CgFX exclusive APIs cannot address elements
	 * as COLLADA's addressing model seems to want.
	 */
	FX::Data *FindSettingParam(xs::ID sid)
	{
		FX::Param *out; FX::NewParamable *out2; 
		if(FindConnectingParam(sid,out,out2)) 
		return out->Data; return nullptr;
	}
	bool FindConnectingParam(xs::ID sid, FX::Param* &out, FX::NewParamable* &out2)
	{
		for(size_t i=Params.size();i-->0;)		
		if(sid==Params[i]->Name)		
		{
			out = Params[i]; out2 = this; return true;
		}
		if(Parent_NewParamable!=nullptr) 
		return Parent_NewParamable->FindConnectingParam(sid,out,out2);
		return false;
	}
};

class NewParam : public FX::Param, public FX::Annotatable
{
COLLADA_(public)

	xs::ID Semantic; 

COLLADA_(public)

	NewParam(FX::DataMaker<FX::NewParam>*);
	
	virtual void Apply()
	{
		Annotatable::Apply(this); Data->Apply(this);
	}
};

class SetParam : public FX::Param 
{
COLLADA_(public)

	SetParam(FX::DataMaker<FX::SetParam>*);
	
	virtual void Apply(){ Data->Apply(this); }
};
/**NEW
 * The original "cfxConnectParam" was implementing the
 * <bind><param> and is renamed to @c FX::BindParam_To.
 * This is added for <connect_param>.
 *
 * This isn't called "SetParam_To" because the "To" is
 * referring to @c FX::SetParam::Name, and so that can
 * be misleading.
 */
class ConnectParam : public FX::Param 
{
COLLADA_(public)
						  
	xs::ID Name_To; CGparameter Cg_To;

	ConnectParam(xs::ID,FX::NewParamable*,xs::ID,FX::Effect*);

	virtual void Apply()
	{	
		//Thinking this is supposed to be silent/speculative.
		if(Cg==nullptr||Cg_To==nullptr)
		return;
		
		daeEH::Verbose<<"Connect from param "<<Name<<" to param "<<Name_To;
		//cgConnectParameter's args are "from" and "to."
		cgConnectParameter(Cg,Cg_To);
	}
COLLADA_(protected)

	ConnectParam(xs::ID from, xs::ID to);
	void _connect_param(FX::NewParamable*,FX::Effect*);
};

class BindParam : public FX::Param
{
COLLADA_(public)

	FX::Shader *Shader;

COLLADA_(public)

	typedef FX::Shader* arg2;
	BindParam(FX::DataMaker<FX::BindParam> *cp)
	:Shader(cp->aux)
	{}

	virtual void Apply();
}; 
class BindParam_To : public FX::ConnectParam
{
COLLADA_(public)

	FX::Shader *Shader;

COLLADA_(public)

	BindParam_To(xs::ID,FX::Shader*,xs::NCName,FX::Effect*);
};

//-------.
	}//<-'
}

#endif //__COLLADA_FX__PARAM_H__
/*C1071*/