/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <FX.pch.h> //PCH

#include "cfxEffect.h"

COLLADA_(namespace)
{
	namespace FX
	{//-.
//<-----'
		
void FX::Sampler::Apply(FX::Param *param)
{
	//Why was this this way? How did the <instance_effect>
	//assign the surface?
	//if(Surface!=nullptr)
	//return;	

	if(Surface==nullptr)
	{
		//look up source for the matching Surface
		Surface = Params->FindSurface(Source);
		if(Surface==nullptr)
		{
			daeEH::Error<<"Failed to get effect surface by name "<<Source;
			return;
		}

		//THIS LOOKS LIKE A BAD DESIGN PATTERN.
		//THIS LOOKS LIKE A BAD DESIGN PATTERN.
		//THIS LOOKS LIKE A BAD DESIGN PATTERN.
		Surface->Referencing_Samplers.push_back(param);
	}	

	//Assuming <newparam> has not been set up.
	if(0==Surface->TexId)
	return;
			  
	//daeEH::Verbose<<"Setting sampler state.";
	//this works on the currently bound texture object
	//not important which texture unit is used, we just need to use one to
	//allow cgSetSamplerState() to work properly. 

	#ifdef NDEBUG
	#error See if the state is remembered. (It probably is.)
	#endif
	//THIS IS BEING REPEATED EVERY DRAW.
	//daeEH::Verbose<<"Binding texture "<<Surface->TexId;
	glBindTexture(GL_TEXTURE_2D,Surface->TexId);

	//calling this before generate mipmaps is better cause then the call
	//to generate mipmaps already has the space set up for it to use
	cgSetSamplerState(param->Cg);

	//REMINDER: CrtTexture.cpp MAY'VE DONE THIS ALREADY.
	if(GenerateMipmaps)
	{
		//NEW: Ensure the mipmaps are not regenerated unnecessarily.
		//(Especially every time the sampler is bound.)
		GLint test = 1;
		glGetTexLevelParameteriv(GL_TEXTURE_2D,1,GL_TEXTURE_WIDTH,&test);
		if(0==test)
		{
			#ifdef SN_TARGET_PS3
			glGenerateMipmapOES(GL_TEXTURE_2D);
			#else 
			GL.GenerateMipmap(GL_TEXTURE_2D); //glGenerateMipmap
			#endif
		}
	}
}

template<class S, CGtype, class>
struct cfxData_copier
{
	S &Data;
	template<class T>
	cfxData_copier(S &d, T &e,...):Data(d)
	{ 
		_copy(*e->value.operator->()); 
	}
	template<class U, int N>
	void _copy(const daeArray<U,N> &cp)
	{
		size_t size = sizeof(S)/sizeof(typename S::type);
		for(size_t i=std::min(cp.size(),size);i-->0;)
		((typename S::type*)&Data)[i] = (typename S::type)cp[i];
	}
	template<class U>
	//This is complicated because cg_*1 isn't <xs:list> based.
	void _copy(const U &cp)
	{
		Data = cp; COLLADA_SUPPRESS_C(4244) //Casting is hard.
	}
};
template<CGtype E, class T> 
struct cfxData_copier<FX::Sampler,E,T>
{	 
	//COLLADA's schemas do not define these.				
	GLenum _wrap_GLenum(Collada05_XSD::fx_sampler_wrap_common e)
	{
		switch(e)
		{
		case e.NONE: 
		case e.BORDER: 
		default: return GL_CLAMP_TO_BORDER;
		case e.WRAP: return GL_REPEAT;
		case e.CLAMP: return GL_CLAMP_TO_EDGE;
		case e.MIRROR: return GL_MIRRORED_REPEAT; 
		}
	}
	GLenum _filter_GLenum(Collada05_XSD::fx_sampler_filter_common e)
	{
		switch(e)
		{	
		case e.NONE:
		case e.NEAREST:
		default: return GL_NEAREST;
		case e.NEAREST_MIPMAP_LINEAR: return GL_NEAREST_MIPMAP_LINEAR;
		case e.LINEAR: return GL_LINEAR;
		case e.NEAREST_MIPMAP_NEAREST: return GL_NEAREST_MIPMAP_NEAREST;
		case e.LINEAR_MIPMAP_NEAREST: return GL_LINEAR_MIPMAP_NEAREST;	
		case e.LINEAR_MIPMAP_LINEAR: return GL_LINEAR_MIPMAP_LINEAR;	
		}		
	}
	void _set(const char *cc, unsigned to)
	{
		cgSetIntStateAssignment(_new_CGstateassignment(cc),to);
	}
	void _set(const char *cc, float to)
	{
		cgSetFloatStateAssignment(_new_CGstateassignment(cc),to);
	}
	void _set(const char *cc, float *to)
	{
		cgSetFloatArrayStateAssignment(_new_CGstateassignment(cc),to);
	}		
	CGstateassignment _new_CGstateassignment(const char *cc)
	{
		//Reminder: Samplers aren't targetable for animation.
		return cgCreateSamplerStateAssignment(Data.Cg,cgGetNamedSamplerState(Cg,cc));
	}
	template<class T> void _wrap(const char *cc, const T &ee)
	{
		if(!ee.empty()) _set(cc,_wrap_GLenum(ee->value));
	}
	template<int> void _wrap_N(){ _wrap("WrapS",e->wrap_s); }
	template<> void _wrap_N<2>(){ _wrap("WrapT",e->wrap_t); _wrap_N<1>(); }
	template<> void _wrap_N<3>(){ _wrap("WrapR",e->wrap_p); _wrap_N<2>(); }			
	FX::Sampler &Data; CGcontext Cg; T &e; 
	COLLADA_NOINLINE
	cfxData_copier(FX::Sampler &s, T &e, FX::NewParamable *p, xs::ID sid)
	:Data(s),e(e)
	{ 	
		if(e->source.empty()) return;

		FX::Effect *pe = p->FindEffect();
		Cg = pe->Cg_Context; 

		//HACK: This is done here for samplers so to not
		//have to generate a lot of hot air data objects.
		if("setparam"==dae(e)->getParent()->getNCName())
		{
			Data.Cg = cgGetNamedEffectParameter(pe->Cg,sid);
			if(Data.Cg==nullptr)
			{
				assert(pe->Cg!=nullptr);
				return;
			}
		}
		else if(E==CG_UNKNOWN_TYPE) //CG_SAMPLERDEPTH
		{
			Data.Cg = cgCreateEffectParameter(pe->Cg,sid,CG_SAMPLERRECT);
		}
		else Data.Cg = cgCreateEffectParameter(pe->Cg,sid,E);

		s.Params = p; s.Source = e->source->value;
		
	////These values are accessed in <xs:sequence> order.

		//wrap_* goes by the sampler's dimensionality.
		_wrap_N<E==CG_SAMPLER1D?1:E!=CG_SAMPLER3D?2:3>();
											
		GLenum mf = GL_NEAREST;
		if(!e->minfilter.empty())
		{
			mf = _filter_GLenum(e->minfilter->value);
			_set("MinFilter",mf);
		}
		//SCHEDULED FOR REMOVAL?
		//The default for mipmap generation is true.
		if(mf==GL_LINEAR||mf==GL_NEAREST)
		Data.GenerateMipmaps = false;
					
		if(!e->magfilter.empty())
		_set("MagFilter",_filter_GLenum(e->magfilter->value));

		//Only the depth texture (sampler) is unmipmapped.
		_mipmap_if<E!=CG_UNKNOWN_TYPE>(); //CG_SAMPLERDEPTH
	}	
	template<bool> void _mipmap_if(){}
	template<> void _mipmap_if<true>()
	{	
		if(!e->mipfilter.empty()) 
		_set("MipFilter",_filter_GLenum(e->mipfilter->value));
	
		if(!e->border_color.empty()) //Mipmap related?
		{
			FX::Float4 bc(0,1);
			e->border_color->value->get4at(0,bc.r,bc.g,bc.b,bc.a);
			_set("BorderColor",&bc.x);
		}	
		
		if(!e->mipmap_maxlevel.empty())
		_set("MaxMipLevel",(unsigned)e->mipmap_maxlevel->value);

		if(!e->mipmap_bias.empty()) 
		_set("MipMapLodBias",(float)e->mipmap_bias->value);
			
		//THIS IS A <surface> ELEMENT??
		//if(!e->mipmap_generate.empty()) //bool
		//_set("GenerateMipMap",e->mipmap_generate->value);		
	}	
};

struct cfxData_MakeData
{
	//h is the header size.
	char* &o; int h; 
	//These are for samplers.
	FX::NewParamable *p; xs::ID sid;
		
	template<CGtype E, class S_Value, class T> 
	inline void _copy(S_Value &d, const T &e)
	{
		cfxData_copier<S_Value,E,const T>(d,e,p,sid);
	}
	template<class S, class T> 
	inline void _new(const T &e)
	{
		o = (char*)operator new(h+sizeof(S));
		_copy<S::Cg>((new(o+h)S)->Value,e);
	}
	COLLADA_NOINLINE
	void operator()(const const_daeChildRef &e)
	{	
		//This ensures genus cannot be mistaken.		
		if(1!=e.name()) switch(e->getElementType())
		{
		#define _1(x,y) \
		case DAEP::Schematic<Collada05::y>::genus:\
		return _new<FX::Data##x>(*(Collada05::const_##y*)&e);
		#define _3(x,y) \
		_1(x,annotate::y)_1(x,profile_CG::newparam::y)\
		_1(x,instance_effect::setparam::y)
		#define _5(x,y) _3(x,y##__alias)\
		/*_3(x##1,y##1)*/_3(x##2,y##2)_3(x##3,y##3)_3(x##4,y##4)	
		#define _S(x) /*falling thru*/\
		case DAEP::Schematic<Collada05::profile_CG::newparam::sampler##x>::genus:\
		_1(Sampler##x,texture##x::value)
		//_1(Sampler##x,instance_effect::setparam::sampler##x)
		//END MACRO DEFS
	
		#ifdef NDEBUG
		#error profile_Cg has a lot more basic data types than this.
		#endif				
		_1(Bool1,profile_CG::newparam::bool1)
		_1(Int1,profile_CG::newparam::int1)
		_1(Float1,profile_CG::newparam::float1)
		_1(String,profile_CG::newparam::string)
		_5(Bool,bool)_5(Int,int)_5(Float,float)
		_3(Float2x2,float2x2)_3(Float3x3,float3x3)_3(Float4x4,float4x4)
		//The 1.4.1 schema defines aliases for these types just for the "fun" of it.
		case DAEP::Schematic<Collada05::profile_COMMON::newparam::sampler2D>::genus:	
		_S(2D)/*falling thru*/_S(1D)_S(3D)_S(CUBE)_S(RECT)_S(DEPTH)

		//END MACRO USES
		#undef _S
		#undef _5
		#undef _3
		#undef _1

		case DAEP::Schematic<Collada05::annotate>::genus:
		case DAEP::Schematic<Collada05::modifier>::genus:
		case DAEP::Schematic<Collada05::semantic>::genus:

			//Known exceptions.
			break;

		default: assert(0);
		}
	}
	void FailData()
	{				   
		xs::ID e = "nullptr"; _new<FX::DataString>(e);
	}template<>
	inline void _copy<CG_STRING>(xs::string &d, const xs::ID &e)
	{
		d = e; //"nullptr"
	}
};
template<class T>
void FX::DataMaker<T>::MakeData()
{
	cfxData_MakeData md = {(char*&)o,sizeof(T),p,sid}; 
	o = nullptr;
	c.for_each_child(md);
	if(o==nullptr)
	md.FailData(); o->Data = (FX::Data*)(o+1);

	//HACk: Tell T::T to not override T::Cg.
	switch(o->Data->GetType()) 
	{
	case CG_SAMPLER1D: case CG_SAMPLERCUBE:
	case CG_SAMPLER2D: case CG_SAMPLERRECT:
	case CG_SAMPLER3D: case CG_UNKNOWN_TYPE: //CG_SAMPLERDEPTH
	
		//Casting because FX::Annotate::Cg is different type.
		//Sampler annotations don't exist.
		(CGparameter&)o->Cg = ((FX::DataSampler2D*)o->Data)->Value.Cg; 
		break;

	default: o->Cg = nullptr;
	}
	o->Name = sid; new(o) T(this);
}
extern void FX::MakeData2(DataMaker<FX::Annotate> &dm){ dm.MakeData(); }
extern void FX::MakeData2(DataMaker<FX::NewParam> &dm){ dm.MakeData(); }
extern void FX::MakeData2(DataMaker<FX::SetParam> &dm){ dm.MakeData(); }
extern void FX::MakeData2(DataMaker<FX::BindParam> &dm){ dm.MakeData(); }

void FX::Data::Apply(FX::Param*)
{
	daeEH::Error<<"Attempted to apply Data of a type which is unsupported for parameters.";	
} 
template<> void FX::DataString::Apply(FX::Param *param) //BOOL
{
	cgSetStringParameterValue(param->Cg,Value);	
}
template<> void FX::DataBool::Apply(FX::Param *param) //BOOL
{
	cgSetParameter1f(param->Cg,(float)Value);	
}
template<> void FX::DataBool1::Apply(FX::Param *param)
{
	cgSetParameter1f(param->Cg,(float)Value.b0);
}
template<> void FX::DataBool2::Apply(FX::Param *param)
{
	cgSetParameter2f(param->Cg,(float)Value.b0,(float)Value.b1);
}
template<> void FX::DataBool3::Apply(FX::Param *param)
{
	cgSetParameter3f(param->Cg,(float)Value.b0,(float)Value.b1,(float)Value.b2);
}
template<> void FX::DataBool4::Apply(FX::Param *param)
{
	cgSetParameter4f(param->Cg,(float)Value.b0,(float)Value.b1,(float)Value.b2,(float)Value.b3);
}
template<> void FX::DataInt::Apply(FX::Param *param) //INT
{
	cgSetParameter1f(param->Cg,(float)Value);
}
template<> void FX::DataInt1::Apply(FX::Param *param)
{
	cgSetParameter1f(param->Cg,(float)Value.i0);
}
template<> void FX::DataInt2::Apply(FX::Param *param)
{
	cgSetParameter2f(param->Cg,(float)Value.i0,(float)Value.i1);
}
template<> void FX::DataInt3::Apply(FX::Param *param)
{
	cgSetParameter3f(param->Cg,(float)Value.i0,(float)Value.i1,(float)Value.i2);
}
template<> void FX::DataInt4::Apply(FX::Param *param)
{
	cgSetParameter4f(param->Cg,(float)Value.i0,(float)Value.i1,(float)Value.i2,(float)Value.i3);
}
template<> void FX::DataFloat::Apply(FX::Param *param) //FLOAT
{
	cgSetParameter1f(param->Cg,Value);
}
template<> void FX::DataFloat1::Apply(FX::Param *param)
{
	cgSetParameter1f(param->Cg,Value.f0);
}
template<> void FX::DataFloat2::Apply(FX::Param *param)
{
	cgSetParameter2f(param->Cg,Value.f0,Value.f1);
}
template<> void FX::DataFloat3::Apply(FX::Param *param)
{
	cgSetParameter3f(param->Cg,Value.f0,Value.f1,Value.f2);
}
template<> void FX::DataFloat4::Apply(FX::Param *param)
{
	cgSetParameter4f(param->Cg,Value.f0,Value.f1,Value.f2,Value.f3);
}
template<> void FX::DataFloat2x2::Apply(FX::Param *param)
{
	//using column major matrix setting because i think that is what is used by COLLADA
	cgSetMatrixParameterfc(param->Cg,&Value.f00);
}
template<> void FX::DataFloat3x3::Apply(FX::Param *param)
{
	//using column major matrix setting because i think that is what is used by COLLADA
	cgSetMatrixParameterfc(param->Cg,&Value.f00);
}
template<> void FX::DataFloat4x4::Apply(FX::Param *param)
{
	//using column major matrix setting because i think that is what is used by COLLADA
	cgSetMatrixParameterfc(param->Cg,&Value.f00);
}			  
#define _(x) \
template<> void FX::DataSampler##x::Apply(FX::Param *param)\
{\
	((FX::DataSampler2D*)this)->DataSampler2D::Apply(param);\
}
_(1D)/*_(2D)*/_(3D)_(CUBE)_(RECT)_(DEPTH) //SAMPLER
#undef _
template<> void FX::DataSampler2D::Apply(FX::Param *param) //SAMPLER
{				 
	#ifdef NDEBUG
	#error Apply_TexId smells bad.
	#endif
	//call the sampler apply to 
	//1) initialize texture id by resolving source
	//2) have its states and stuff applied
	Value.Apply(param);
	//set the parameter for this sampler's texture id
	//using managed textures replaces the need to call enabletextureparameter
	if(Value.Surface!=nullptr)
	cgGLSetTextureParameter(param->Cg,Value.Surface->TexId);
}					

//Cg ANNOTATION SUPPORT IS APPARENTLY POOR. //ANNOTATION //ANNOTATION //ANNOTATION

void FX::Data::Apply(FX::Annotate*)
{
	daeEH::Error<<"Attempted to apply Value of a type which is unsupported for annotations.";	
}
template<> void FX::DataString::Apply(FX::Annotate *annotate)
{
	cgSetStringAnnotation(annotate->Cg,Value);	
}
template<> void FX::DataBool::Apply(FX::Annotate *annotate)
{
	cgSetBoolAnnotation(annotate->Cg,Value);	
}
template<> void FX::DataBool1::Apply(FX::Annotate *annotate)
{
	cgSetBoolAnnotation(annotate->Cg,Value.b0);
}
template<> void FX::DataInt::Apply(FX::Annotate *annotate)
{
	cgSetIntAnnotation(annotate->Cg,Value);
}
template<> void FX::DataInt1::Apply(FX::Annotate *annotate)
{
	cgSetIntAnnotation(annotate->Cg,Value.i0);
}
template<> void FX::DataFloat::Apply(FX::Annotate *annotate)
{
	cgSetFloatAnnotation(annotate->Cg,Value);
}
template<> void FX::DataFloat1::Apply(FX::Annotate *annotate)
{
	cgSetFloatAnnotation(annotate->Cg,Value.f0);
}
#define _(x,y) \
template<> void FX::Data##x::Apply(FX::Annotate*)\
{\
	daeEH::Verbose<<"API to support annotations of type "<<"CG_"#y<<" not implemented yet.";\
}
_(Bool2,BOOL2)_(Bool3,BOOL3)_(Bool4,BOOL4)
_(Int2,INT2)_(Int3,INT3)_(Int4,INT4)
_(Float2,FLOAT2)_(Float3,FLOAT3)_(Float4,FLOAT4)
_(Float2x2,FLOAT2x2)_(Float3x3,FLOAT3x3)_(Float4x4,FLOAT4x4)
_(Sampler1D,SAMPLE1D)_(Sampler2D,SAMPLER2D)_(Sampler3D,SAMPLER3D)
_(SamplerCUBE,SAMPLERCUBE)_(SamplerRECT,SAMPLERRECT)_(SamplerDEPTH,SAMPLERDEPTH)
#undef _

//-------.
	}//<-'
}

/*C1071*/