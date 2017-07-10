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

/**
 * This is as large a list as necessary of in use
 * "semantics" normalized to some standard values.
 * @see @c FX::NewParam::_InitSemantic_etc() list
 * of synonyms & alternative spelling/punctuation.
 */
enum Semantic
{
	AMBIENT,		
	COLOR,
	DIFFUSE,
	EMISSION,
	LIGHT_POSITION,
	OPACITY,
	POSITION,
	POWER,
	PROJECTION,
	SHININESS,
	SPECULAR,	
	SPOT_DIRECTION,
	TIME,
	TRANSPARENCY,
	VIEW,
	VIEW_INVERSE,	
	WORLD,
	WORLD_INVERSE_TRANSPOSE,
	WORLD_VIEW,
	WORLD_VIEW_INVERSE_TRANSPOSE,
	WORLD_VIEW_PROJECTION,
};

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
	FX::Data *SetData;

	/**HACK
	 * For non <newparam> strings this is set to
	 * include the hidden # part of a string-ref
	 * so that it never compares as equal inside
	 * of searches.
	 * @see @c FX::Paramable::FindNewParam().
	 */
	xs::ID Name; inline bool IsNewParam()const
	{
		if(((size_t)Name&1)!=0)
		{
			assert(Name[-1]=='#'); return true;
		}
		else assert(Name[0]=='#'); return false;
	}

COLLADA_(public)

	virtual void Apply() = 0;

	virtual ~Param(){ /*UNUSED*/ }

	typedef xs::ID arg2;
	Param():SetData(SetData),Name(Name)
	{ /*NOP*/ }

	bool operator<(const FX::Param &cmp)const;
};
	
//SCHEDULED FOR REMOVAL?
#ifdef NDEBUG
//One problem with that is <sampler_image> uses
//the same <setparam> name as it sampler does?!
#error Make this data like Sampler?
#endif
/**
 * @see @c FX::Paramable.
 *
 * @note For whatever reason <surface> isn't
 * treated like the @c FX::Data based values.
 * REMINDER: 1.5.0 introduces <sampler_image>
 * which throws a wrench into things, because
 * there can be different types targeting the
 * same <setparam> names.
 */
class Surface
{	
COLLADA_(public)
	/**COLLADA 1.4.1
	 * _Mode is added to try to remember what parts are
	 * incomplete so they can be replaced when they are
	 * instanced.
	 */
	GLuint TexId; int _Model;

	/**HACK
	 * This is temporary to emulate a Param/Data pair.
	 * OR should <surface> and <sampler_image> be kept
	 * separate? 
	 *
	 * It will fail with <connect_param>.
	 */
	FX::Surface *SetParam, *ParamToSet;

COLLADA_(public)
	/**HACK
	 * Simulating @c FX::SetParam::Apply().
	 */
	void Apply(){ SetParam = this; ParamToSet->SetParam = this; }

	Surface(GLuint TexId, FX::Surface *parent=nullptr)
	:TexId(TexId),SetParam(this),ParamToSet(this)
	{
		if(parent!=nullptr) ParamToSet = parent;

		_Model = TexId==0?0:1;
	}

	/**COLLADA 1.4.1
	 * This is implementing an inheritance model for <surface> where
	 * <setparam> doesn't completely override the <newparam> setting.
	 * There is little to it because most settings are unrepresented.
	 */
	void Refresh()
	{
		if((SetParam->_Model&1)==1) TexId = SetParam->TexId;			
	}
};
/**
 * This is added so @c FX::Technique can contain surface 
 * <newparam> information.
 *
 * @todo Surfaces shouldn't be done like this, but it is
 * how they were done historically and there is a slight
 * problem with how <sampler_image> uses a <setparam> to
 * augment a sampler rather than to replace it. Surfaces
 * should be data, and @c FX::Sampler should inherit the
 * <newparam> definition as surfaces currently do. While
 * <sampler_image> is like a surface it uses the sampler
 * name to refer to itself. It needs to use an alternate
 * name so it doesn't override sampler data.
 */
class Paramable
{					   
COLLADA_(public)

	std::vector<FX::Param*> Params;

	/**	 
	 * This can be one of <effect> <profile*> <technique>
	 */
	FX::Paramable *Parent_Paramable;

	//SCHEDULED FOR REMOVAL?
	//See TODO note in the class documentation.
	std::vector<std::pair<xs::ID,FX::Surface*>> Surfaces;

COLLADA_(public)

	Paramable(FX::Paramable *parent=nullptr)
	:Parent_Paramable(parent)
	{}
	~Paramable()
	{
		for(size_t i=0;i<Params.size();i++)
		delete Params[i];	
		//SCHEDULED FOR REMOVAL
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
		if(nullptr!=Parent_Paramable)
		Parent_Paramable->Apply();

		for(size_t i=0;i<Params.size();i++)	
		Params[i]->Apply();
		//SCHEDULED FOR REMOVAL
		for(size_t i=0;i<Surfaces.size();i++)
		Surfaces[i].second->Apply();
	}
	/**
	 * @see FX::Material::Apply().
	 * @note profile_COMMON is implemented
	 * differently. Maybe it should not be.
	 */
	void Apply_COMMON()
	{
		//The order is all messed up.
		Parent_Paramable->Parent_Paramable->Apply();

		std::vector<FX::Param*> &pv = Parent_Paramable->Params;
		size_t i;
		for(i=0;i<pv.size()&&pv[i]->IsNewParam();i++)
		pv[i]->Apply();

		for(size_t j=0;j<Params.size();j++)
		Params[j]->Apply();

		for(;i<pv.size();i++) pv[i]->Apply();
	}

	FX::Effect *FindEffect()
	{
		if(nullptr==Parent_Paramable) 
		return (FX::Effect*)this;
		return Parent_Paramable->FindEffect();
	}

	FX::Surface *FindSurface(xs::ID sid)
	{
		for(size_t i=0;i<Surfaces.size();i++)		
		if(sid==Surfaces[i].first)		
		return Surfaces[i].second;
		if(Parent_Paramable!=nullptr) 
		return Parent_Paramable->FindSurface(sid);
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
		for(FX::Param *out;FindNewParam(sid,out);)
		return out->SetData; return nullptr;
	}
	FX::NewParam **_FindNewParam(xs::ID sid, FX::Paramable* &out)
	{
		for(size_t i=Params.size();i-->0;)		
		if(sid==Params[i]->Name)		
		{
			out = this; return (NewParam**)&Params[i]; 
		}
		if(Parent_Paramable!=nullptr) 
		return Parent_Paramable->_FindNewParam(sid,out);
		return nullptr;
	}
	bool FindNewParam(xs::ID sid, FX::Param* &out)
	{
		FX::Paramable *o2; FX::NewParam **o = _FindNewParam(sid,o2);		
		if(o==nullptr) return false; 
		out = (FX::Param*)*o; return true; //NewParam* is undefined.
	}
	bool FindNewParam(xs::ID sid, FX::NewParam* &out)
	{
		return FindNewParam(sid,*(FX::Param**)&out);
	}
};

class NewParam : public FX::Param, public FX::Annotatable
{
COLLADA_(public)
	/**
	 * This is lets the data be connected to one
	 * of the global managed states or semantics.
	 */
	FX::Data *ClientData;

	/**
	 * This is part of the @c Param memory block.
	 * I.e. It could be a @c virtual method also.
	 *
	 * @note This will be modified by animations.
	 */
	FX::Data &OwnData(){ return *(FX::Data*)(this+1); }

	/**WARNING
	 * @warning Not trying to detect client-data.
	 * @return Returns @c true if the param data
	 * is set; and so is not the <newparam> data.
	*/
	inline bool IsSet(){ return SetData!=&OwnData(); }

	/**
	 * The Cg connection APIs are too brittle to
	 * be of use, and FX must cover more than Cg.
	 *
	 * @note This is avoiding the word connected
	 * because Cg uses it and <connect_param> is
	 * not really connecting anything so much as
	 * its just setting one parameter to another.
	 */
	inline bool IsClientData(){ return ClientData!=&OwnData(); }
	
	/**
	 * @c SEMANTIC is converted to all caps.
	 * @see @c Subscript.
	 */
	FX::Semantic Semantic; xs::ID SEMANTIC;

	/**
	 * This is a 0-based array index pulled
	 * from the end of @c SEMANTIC applying
	 * @c Semantic.
	 * 
	 * COLLADA's <bind> element is limiting
	 * semantics to a single word, so there
	 * is no way around the likes of having
	 * to work with LIGHTPOS1 and LIGHTPOS2
	 * etc.
	 */
	unsigned Subscript;
	
	/**
	 * Tells if @c Semantic is any of WORLD or WORLD_X
	 * semantics.
	 * These semantics are high traffic. @c FX::Shader
	 * moves them to the front of its bound parameters.
	 * @see @c FX::Material::SetWorld().
	 */
	inline bool IsWorld()
	{
		switch(Semantic)
		{
		case WORLD:
		case WORLD_INVERSE_TRANSPOSE:
		case WORLD_VIEW:
		case WORLD_VIEW_INVERSE_TRANSPOSE:
		case WORLD_VIEW_PROJECTION: 
		return true;
		default:; //-Wswitch
		}
		return false;
	}

COLLADA_(public)
	/**INTERNAL
	 * @see FX::Profile_COMMON.
	 */
	NewParam():ClientData(&OwnData()){}
	/**INTERNAL
	 */
	NewParam(FX::DataMaker<FX::NewParam>*);	
	/**
	 * Call this after Annotations is available.
	 */
	FX::Semantic _InitSemantic_etc();

	virtual void Apply(){ SetData = ClientData; }
};
inline bool FX::Param::operator<(const FX::Param &cmp)const
{
	//Sort before doing <setparam>.
	//if(!IsNewParam()) return false;
	//if(!cmp.IsNewParam()) return true;
	assert(IsNewParam()&&cmp.IsNewParam());

	const FX::NewParam *a = (const FX::NewParam*)this;
	const FX::NewParam *b = (const FX::NewParam*)&cmp;

	if(a->Semantic<b->Semantic) return true;
	if(a->Semantic>b->Semantic) return false;

	if(INT_MAX!=+a->Semantic)
	return a->Subscript<b->Subscript;

	if(a->SEMANTIC==b->SEMANTIC)
	return a->Subscript<b->Subscript;

	//Alphabetizing is not necessary, but the
	//enumerated values should be sorted, so it
	//might appear to be more consistent to do so.
	//(Even still the known values will come first.)
	return strcmp(a->SEMANTIC,b->SEMANTIC)<0;
}

class SetParam : public FX::Param
{
COLLADA_(public)
	/**
	 * This is part of the @c Param memory block.
	 * I.e. It could be a @c virtual method also.
	 *
	 * @note This will be modified by animations.
	 */
	FX::Data &OwnData(){ return *(FX::Data*)(this+1); }

	FX::Param *ParamToSet;

	SetParam(FX::DataMaker<FX::SetParam>*);
	SetParam(){ /*NOP*/ } //COMMON
	void SetParamToSet(FX::NewParam *COMMON)
	{
		Name = COMMON->Name-1; ParamToSet = COMMON;
		SetData = &OwnData(); //UNUSED
	}
	
	virtual void Apply(){ ParamToSet->SetData = &OwnData(); }
};
/**COLLADA 1.4.1
 * This is <connect_param> which is just a <setparam> 
 * without any data of its own. 
 * @note 1.5.0 ues this pattern in IK but FX no more.
 * There may not be a way to do one-to-many settings:
 * https://www.khronos.org/files/collada_1_5_release_notes.pdf
 */
class SetParam_To : public FX::Param 
{
COLLADA_(public)
	/**
	 * This is not used. It's just for parity in case 
	 * <connect_param> is a dead-link and convenience.
	 */
	xs::ID Name_To;

	FX::Param *Param_To, *ParamToSet; 

	SetParam_To(FX::Paramable*,xs::ID,xs::ID); 

	/**
	 * This is sensitive to the order of the settings.
	 */
	virtual void Apply(){ ParamToSet->SetData = Param_To->SetData; }

COLLADA_(public) //profile_COMMON support

	//x is a color multiplier, going off specification or a liberal interpretation.
	SetParam_To(FX::NewParam*COMMON,FX::Paramable*,xs::ID,FX::NewParam**x=nullptr);	
	SetParam_To(FX::NewParam*COMMON,FX::NewParam*x);
};

/**
 * This is <shader><bind>. It's not a regular "param,"
 * but it's part of the framework. It's the final part.
 */
class ShaderParam : public FX::Param
{
COLLADA_(public)
	/**
	 * This is a shorthand to the selected data.
	 */
	inline FX::Data *operator->()
	{
		return Param_To->SetData;
	}

COLLADA_(public)
	/**PACKED
	 * For samplers @c Type is a GL_TEXTURE0 based
	 * constant. Otherwise it's glGetActiveUniform
	 * type enumerants.
	 *
	 * @c todo: Make @c Type an FX type and derive
	 * it from the GLSL types. Don't use an @c int
	 * based enum so it is visible in the debugger
	 * and use smaller figures.
	 */
	GLenum Type; unsigned char Size,Cg_decoration;
	/**PACKED
	 * This is the uniform "location" under OpenGL.
	 * The name comes from a time when Cg and GLSL
	 * were separated. 
	 */
	short GLSL;
		
	/**
	 * @c Name_To is simply a record; not used for
	 * any purpose. @c Param_To could derive it if
	 * need be.
	 */
	FX::Param *Param_To; xs::ID Name_To;

COLLADA_(public)

	ShaderParam():Param_To(),Name_To()
	{
		SetData = nullptr;
	}
	~ShaderParam()
	{
		delete (void*)SetData; //-Wdelete-non-virtual-dtor
	}

	void SetParam_To(FX::Pass*,xs::ID);	
	void SetParam_To(GLenum t, char s, FX::Param *to)
	{
		Param_To = to; Type = t; Size = s; Cg_decoration = '\0';
	}

	/**
	 * UNLIKE THE OTHER @c FX::Param based objects, this
	 * @c Apply() function is not done in the stage that
	 * configures the instanced effect, but is done only
	 * after that stage, on a per pass basis, committing
	 * the configuration to the shader, along with other
	 * render state changes.
	 */
	virtual void Apply(){ Param_To->SetData->Load(*this); }
};

//-------.
	}//<-'
}

#endif //__COLLADA_FX__PARAM_H__
/*C1071*/
