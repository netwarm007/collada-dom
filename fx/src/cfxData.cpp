/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <FX.pch.h> //PCH

#include "cfxEffect.h"
#include "cfxLoader.h"

COLLADA_(namespace)
{
	namespace FX
	{//-.
//<-----'

template<class S, int YY, CGtype, class>
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
struct cfxData_copier<FX::Sampler,5,E,T>
{	 
	//COLLADA's schemas do not define these.				
	GLenum _wrap_GLenum(Collada05_XSD::fx_sampler_wrap_common e)
	{
		switch(e)
		{
		case e.NONE: 
		case e.BORDER: 
		return GL_CLAMP_TO_BORDER;
		default:
		case e.WRAP: return GL_REPEAT;
		case e.CLAMP: return GL_CLAMP_TO_EDGE;
		case e.MIRROR: return GL_MIRRORED_REPEAT; 
		}
	}
	GLenum _wrap_GLenum(Collada08_XSD::fx_sampler_wrap_enum e)
	{
		switch(e)
		{
		case e.BORDER: 
		return GL_CLAMP_TO_BORDER;
		default:
		case e.WRAP: return GL_REPEAT;
		case e.CLAMP: return GL_CLAMP_TO_EDGE;
		case e.MIRROR: return GL_MIRRORED_REPEAT; 
		case e.MIRROR_ONCE: return GL_MIRROR_CLAMP_TO_EDGE; 
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
		if(Cg!=nullptr) cgSetIntStateAssignment(_new_CGstateassignment(cc),to);
	}
	void _set(const char *cc, float to)
	{
		if(Cg!=nullptr) cgSetFloatStateAssignment(_new_CGstateassignment(cc),to);
	}
	void _set(const char *cc, float *to)
	{
		if(Cg!=nullptr) cgSetFloatArrayStateAssignment(_new_CGstateassignment(cc),to);
	}		
	CGstateassignment _new_CGstateassignment(const char *cc)
	{
		//Reminder: Samplers aren't targetable for animation.
		return cgCreateSamplerStateAssignment(Data.Cg,cgGetNamedSamplerState(Cg,cc));
	}
	template<class S> void _wrap(const char *cc, const S &ee)
	{
		if(!ee.empty()) _set(cc,_wrap_GLenum(*ee->value.operator->()));
	}
	//GCC/C++ won't facilitate explicit-specialization.
	template<int N> void _wrap_N(){ _wrap_N(daeFig<N>()); }
	void _wrap_N(daeFig<1>){ _wrap("WrapS",e->wrap_s); }
	void _wrap_N(daeFig<2>){ _wrap("WrapT",e->wrap_t); _wrap_N<1>(); }
	void _wrap_N(daeFig<3>){ _wrap("WrapR",e->wrap_p); _wrap_N<2>(); }
	//SCHEDULED FOR REMOVAL
	void _Cg_common_constructor(FX::Paramable *p, xs::ID sid)
	{
		//Data doesn't normally need Cg handles any longer
		//except the Cg API uses handles to build samplers.
		#ifdef NDEBUG //GCC doesn't like apostrophes.
		#error "Non-CG samplers don't require CG handles."
		//NOTE: At the <effect> level the samplers are shared.
		//NOTE: FX::Effect::Profiles don't know their profile.
		#endif
		FX::Effect *pe = p->FindEffect(); Cg = pe->Cg_Context;
		if(Cg==nullptr) return;		
		#ifdef NDEBUG
		#error Cg.DLL is crashing. Can the same name be reused? What are the implications?
		#endif
		//CRASHING
		//This is crashing in cgCreateSamplerStateAssignment???
		//Data.Cg = cgCreateParameter(Cg,E!=CG_UNKNOWN_TYPE?E:CG_SAMPLERRECT);
		Data.Cg = cgCreateEffectParameter(pe->Cg,sid,E!=CG_UNKNOWN_TYPE?E:CG_SAMPLERRECT);
	}
	FX::Sampler &Data; CGcontext Cg; T &e; 
	cfxData_copier(FX::Sampler &s, T &e)
	:Data(s),e(e){}
	COLLADA_NOINLINE
	cfxData_copier(FX::Sampler &s, T &e, FX::Paramable *p, xs::ID sid)
	:Data(s),e(e)
	{ 	
		if(e->source.empty()) return;

		_Cg_common_constructor(p,sid);

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
		_mipmap_if(daeFig<E!=CG_UNKNOWN_TYPE>()); //CG_SAMPLERDEPTH
	}	
	void _mipmap_if(...){}
	void _mipmap_if(daeFig<1>) //GCC/C++ hate explicit specialization.
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
template<CGtype E, class T> 
struct cfxData_copier<FX::Sampler,8,E,T> : cfxData_copier<FX::Sampler,5,E,T>
{	
	typedef cfxData_copier<FX::Sampler,5,E,T> base;
	using base::e;
	using base::Data;
	using base::_set;

	GLenum _minfilter_GLenum(Collada08_XSD::fx_sampler_min_filter_enum e)
	{
		switch(e) //Do the 1.5.0 enumerants make sense?
		{	
		case e.LINEAR: case e.ANISOTROPIC: //???
		return GL_LINEAR;
		case e.NEAREST:
		default: return GL_NEAREST;
		}		
	}
	GLenum _mipfilter_GLenum(Collada08_XSD::fx_sampler_mip_filter_enum e)
	{
		switch(e) //Do the 1.5.0 enumerants make sense?
		{	
		case e.LINEAR: 
		return GL_LINEAR;
		case e.NEAREST:
		case e.NONE: //GL_NONE?		
		default: return GL_NEAREST;
		}		
	}
	GLenum _magfilter_GLenum(Collada08_XSD::fx_sampler_mag_filter_enum e)
	{
		switch(e) //Do the 1.5.0 enumerants make sense?
		{	
		case e.LINEAR: 
		return GL_LINEAR;
		case e.NEAREST:
		default: return GL_NEAREST;
		}		
	}

	COLLADA_NOINLINE
	cfxData_copier(FX::Sampler &s, T &e, FX::Paramable *p, xs::ID sid)
	:cfxData_copier<FX::Sampler,5,E,T>(s,e)
	{ 						
		base::_Cg_common_constructor(p,sid);

		Data.Params = p; Data.Source = sid;
		
	////These values are accessed in <xs:sequence> order.

		//wrap_* goes by the sampler's dimensionality.
		base::template _wrap_N<E==CG_SAMPLER1D?1:E!=CG_SAMPLER3D?2:3>();
											
		GLenum mf = GL_NEAREST;
		if(!e->minfilter.empty())
		{
			mf = _minfilter_GLenum(e->minfilter->value);
			_set("MinFilter",mf);
		}
		//SCHEDULED FOR REMOVAL?
		//The default for mipmap generation is true.
		if(mf==GL_LINEAR||mf==GL_NEAREST)
		Data.GenerateMipmaps = false;
					
		if(!e->magfilter.empty())
		_set("MagFilter",_magfilter_GLenum(e->magfilter->value));

		//Only the depth texture (sampler) is unmipmapped.
		_mipmap_etc_if(p,daeFig<E!=CG_UNKNOWN_TYPE>()); //CG_SAMPLERDEPTH
	}	
	void _mipmap_etc_if(FX::Paramable*,...){}
	void _mipmap_etc_if(FX::Paramable *p, daeFig<1>)
	{	
		if(!e->mipfilter.empty()) 
		_set("MipFilter",_mipfilter_GLenum(e->mipfilter->value));
	
		if(!e->border_color.empty()) //Mipmap related?
		{
			FX::Float4 bc(0,1);
			e->border_color->value->get4at(0,bc.r,bc.g,bc.b,bc.a);
			_set("BorderColor",&bc.x);
		}	
		
		if(!e->mip_max_level.empty())
		_set("MaxMipLevel",(unsigned)e->mip_max_level->value);

		if(!e->mip_bias.empty()) 
		_set("MipMapLodBias",(float)e->mip_bias->value);	

		_etc_if(p,&e); //sampler_states?
	}
	void _etc_if(FX::Paramable *p,...) //GCC/C++ hate explicit specialization.
	{
		if(!e->instance_image.empty())
		{
			xs::anyURI URI = e->instance_image->url;
			Collada08::const_image yy = URI.get<Collada08::image>();		
			FX::Surface *ii = new FX::Surface(FX::Loader::GetID_TexId(yy));
			p->Surfaces.push_back(std::make_pair(Data.Source,ii));
		}
	}
	void _etc_if(FX::Paramable*,const Collada08::const_sampler_states*_e){ (void)_e; }
};
template<> 
struct cfxData_copier<FX::Sampler,8,CG_SAMPLER2D,const Collada08::const_sampler_states> 
{
	cfxData_copier(FX::Sampler &s, const Collada08::const_sampler_states &e, FX::Paramable *p, xs::ID sid)	
	{ 	
		FX::NewParam *parent; FX::DataSampler2D *cp;
		if(p->FindNewParam(sid,parent)&&nullptr!=(cp=parent->OwnData().As<FX::Sampler>()))
		{
			#ifdef NDEBUG //GCC doesn't like apostrophes.
			#error "This doesn't inherit Cg values. Should it inherit at all?"
			#error NOT-IMPLEMENTING BECAUSE THE CG PATHWAY WILL BE GONE ASAP.
			#endif
			//s = cp->Value doesn't replace the "vptr."
			//Assuming sampler_states can't change the sampler's very nature.
			memcpy(&s,&cp->Value,sizeof(s));
			s.Params = p; s.Cg = nullptr; //Cg is reset; but just to be safe.
			//HACK: _etc_if filters out the image and 
			//sampler_states has all 3 wrapping modes.
			//CG_SAMPLER2D is "recursive on all paths."
			cfxData_copier<FX::Sampler,8,CG_SAMPLER3D,const Collada08::const_sampler_states>(s,e,p,sid);
		}
		else daeEH::Verbose<<"Did not match <sampler_states> "<<sid<<"\n"
		"(The specification says this is a \"speculative call\" and so OK.)";	
	}
};

template<int YY> struct cfxData_MakeData
{
	//h is the header size.
	char* &o; int h; 
	//These are for samplers.
	FX::Paramable *p; xs::ID sid;
		
	template<CGtype E, class S_Value, class T> 
	inline void _copy(S_Value &d, const T &e)
	{
		cfxData_copier<S_Value,YY,E,const T>(d,e,p,sid);
	}
	template<class S, class T> 
	inline void _new(const T &e)
	{
		o = (char*)operator new(h+sizeof(S));
		_copy<S::Cg>((new(o+h)S)->Value,e);
	}
	void FailData()
	{				   
		xs::ID e = "nullptr"; _new<FX::DataString>(e);
	}
	template<CGtype Error>
	inline void _copy(xs::string &d, const xs::ID &e)
	{
		daeCTC<Error==CG_STRING>(); d = e; //"nullptr"
	}

	COLLADA_NOINLINE
	void operator()(const const_daeChildRef &e)
	{
		//GCC/C++ don't allow explicit specialization.
		_foreach(daeFig<YY>(),e);
	}	
	void _foreach(daeFig<5>,const const_daeChildRef &e)
	{	
		//This ensures genus cannot be mistaken.		
		if(1!=e.name()) switch(e->getElementType())
		{
		#define _1(x,y) \
		case DAEP::Schematic<Collada05::y>::genus:\
		return _new<FX::Data##x>(*(Collada05::const_##y*)&e);
		#define _3(x,y) \
		_1(x,annotate::y)_1(x,instance_effect::setparam::y)\
		_1(x,profile_CG::newparam::y)\
		_1(x,profile_GLSL::newparam::y)
		#define _5(x,y) _3(x,y##__alias)\
		/*_3(x##1,y##1)*/_3(x##2,y##2)_3(x##3,y##3)_3(x##4,y##4)
		#define _Sam(x) /*falling thru*/\
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
#ifdef NDEBUG
#error	THERE ARE MANY MORE CG_TYPES.
#endif
		_5(Bool,bool)_5(Int,int)_5(Float,float)
		_1(Float,profile_COMMON::newparam::float__alias) 
		_1(Float2,profile_COMMON::newparam::float2) 
		_1(Float3,profile_COMMON::newparam::float3) 
		_1(Float4,profile_COMMON::newparam::float4) 
		_3(Float2x2,float2x2)_3(Float3x3,float3x3)_3(Float4x4,float4x4)
		//The 1.4.1 schema defines aliases for these types just for the "fun" of it.
		case DAEP::Schematic<Collada05::profile_COMMON::newparam::sampler2D>::genus:	
		_Sam(2D)/*falling thru*/_Sam(1D)_Sam(3D)_Sam(CUBE)_Sam(RECT)_Sam(DEPTH)

		//END MACRO USES
		#undef _Sam //_S belongs to ctype.h. (And looks like 5.)
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
	void _foreach(daeFig<8>, const const_daeChildRef &e)
	{	
		//This ensures genus cannot be mistaken.		
		if(1!=e.name()) switch(e->getElementType())
		{
		#define _1(x,y) \
		case DAEP::Schematic<Collada08::y>::genus:\
		return _new<FX::Data##x>(*(Collada08::const_##y*)&e);
		#define _3(x,y) \
		/*_1(x,annotate::y)_1(x,profile_CG::newparam::y)*/\
		_1(x,instance_effect::setparam::y)
		#define _5(x,y) _3(x,y##__alias)\
		/*_3(x##1,y##1)*/_3(x##2,y##2)_3(x##3,y##3)_3(x##4,y##4)	
		#define _Sam(x) /*falling thru*/\
		/*case DAEP::Schematic<Collada08::profile_CG::newparam::sampler##x>::genus:*/\
		_1(Sampler##x,texture##x::value)
		//_1(Sampler##x,instance_effect::setparam::sampler##x)
		//END MACRO DEFS
	
		#ifdef NDEBUG
		#error profile_Cg has a lot more basic data types than this.
		#endif				
		//_1(Bool1,profile_CG::newparam::bool1)
		//_1(Int1,profile_CG::newparam::int1)
		//_1(Float1,profile_CG::newparam::float1)
		_1(String,profile_CG::newparam::string)
		_5(Bool,bool)_5(Int,int)_5(Float,float)
		//float2-4 are able merged into synthetic types.
		_1(Float,profile_COMMON::newparam::float__alias)
		_1(Float,annotate::float__alias)
		_1(Float,profile_CG::newparam::float__alias)
		_1(Float,profile_GLSL::newparam::float__alias)
		_3(Float2x2,float2x2)_3(Float3x3,float3x3)_3(Float4x4,float4x4)
		//The 1.4.1 schema defines aliases for these types just for the "fun" of it.
		//case DAEP::Schematic<Collada08::profile_COMMON::newparam::sampler2D>::genus:	
		_Sam(2D)/*falling thru*/_Sam(1D)_Sam(3D)_Sam(CUBE)_Sam(RECT)_Sam(DEPTH)

		//This is a counterpart to sampler_image.
		_1(Sampler2D,sampler_states)

		//END MACRO USES
		#undef _Sam //_S belongs to ctype.h. (And looks like 5.)
		#undef _5
		#undef _3
		#undef _1

		case DAEP::Schematic<Collada08::annotate>::genus:
		case DAEP::Schematic<Collada08::modifier>::genus:
		//case DAEP::Schematic<Collada08::semantic>::genus:

			//Known exceptions.
			break;
			if("semantic"==e->getNCName())
			break;

		default: assert(0);
		}
	}
};

template<class T>
template<int YY> 
void FX::DataMaker<T>::MakeData()
{
	T *io = o; //FX::ShaderParam.
	int os = Emplace(io)?0:sizeof(T);
	cfxData_MakeData<YY> md = {(char*&)o,os,p,sid}; 
	o = nullptr;
	c.for_each_child(md);
	if(o==nullptr)
	md.FailData();
	MakeData2(io,sid); assert(sid[-1]=='#');
}
template<class T>
void FX::DataMaker<T>::MakeData2(T*, xs::ID sid)
{
	o->SetData = &o->OwnData(); o->Name = sid-1; new(o) T(this);
}
template<>
void FX::DataMaker<FX::Annotate>::MakeData2(FX::Annotate*, xs::ID sid)
{
	o->Name = sid-1; new(o) FX::Annotate(this);
}
template<>
void FX::DataMaker<FX::ShaderParam>::MakeData2(FX::ShaderParam *io, xs::ID)
{
	io->SetData = (FX::Data*)o;
}
COLLADA_(extern) //rror: explicit qualification in declaration
void /*FX::*/MakeData05(FX::DataMaker<FX::Annotate> &dm){ dm.MakeData<5>(); }
void /*FX::*/MakeData05(FX::DataMaker<FX::NewParam> &dm){ dm.MakeData<5>(); }
void /*FX::*/MakeData05(FX::DataMaker<FX::SetParam> &dm){ dm.MakeData<5>(); }
void /*FX::*/MakeData05(FX::DataMaker<FX::ShaderParam> &dm){ dm.MakeData<5>(); }
void /*FX::*/MakeData08(FX::DataMaker<FX::Annotate> &dm){ dm.MakeData<8>(); }
void /*FX::*/MakeData08(FX::DataMaker<FX::NewParam> &dm){ dm.MakeData<8>(); }
void /*FX::*/MakeData08(FX::DataMaker<FX::SetParam> &dm){ dm.MakeData<8>(); }
void /*FX::*/MakeData08(FX::DataMaker<FX::ShaderParam> &dm){ dm.MakeData<8>(); }

template<> void FX::DataString::Load(FX::ShaderParam &r)
{
	//Reminder: A "(null)" DataString is used to fill bad 
	//data pointers. 
	//assert(0);

	//I think this had a use, but I can't remember what.
	//If so, is there a corresponding GLSL functionality?
	//Reminder: The use may be a GLES (pre-shader) thing??	
	//cgSetStringParameterValue(r.Cg,Value);	
}
template<> void FX::DataBool::Load(FX::ShaderParam &r) //BOOL
{
	GL.Uniform1f(r.GLSL,(float)Value);	
}
template<> void FX::DataBool1::Load(FX::ShaderParam &r)
{
	GL.Uniform1f(r.GLSL,(float)Value.b0);
}
template<> void FX::DataBool2::Load(FX::ShaderParam &r)
{
	GL.Uniform2f(r.GLSL,(float)Value.b0,(float)Value.b1);
}
template<> void FX::DataBool3::Load(FX::ShaderParam &r)
{
	GL.Uniform3f(r.GLSL,(float)Value.b0,(float)Value.b1,(float)Value.b2);
}
template<> void FX::DataBool4::Load(FX::ShaderParam &r)
{
	GL.Uniform4f(r.GLSL,(float)Value.b0,(float)Value.b1,(float)Value.b2,(float)Value.b3);
}
template<> void FX::DataInt::Load(FX::ShaderParam &r) //INT
{
	GL.Uniform1f(r.GLSL,(float)Value);
}
template<> void FX::DataInt1::Load(FX::ShaderParam &r)
{
	GL.Uniform1f(r.GLSL,(float)Value.i0);
}
template<> void FX::DataInt2::Load(FX::ShaderParam &r)
{
	GL.Uniform2f(r.GLSL,(float)Value.i0,(float)Value.i1);
}
template<> void FX::DataInt3::Load(FX::ShaderParam &r)
{
	GL.Uniform3f(r.GLSL,(float)Value.i0,(float)Value.i1,(float)Value.i2);
}
template<> void FX::DataInt4::Load(FX::ShaderParam &r)
{
	GL.Uniform4f(r.GLSL,(float)Value.i0,(float)Value.i1,(float)Value.i2,(float)Value.i3);
}
template<> void FX::DataFloat::Load(FX::ShaderParam &r) //FLOAT
{
	GL.Uniform1f(r.GLSL,Value);
}
template<> void FX::DataFloat1::Load(FX::ShaderParam &r)
{
	GL.Uniform1f(r.GLSL,Value.f0);
}
template<> void FX::DataFloat2::Load(FX::ShaderParam &r)
{
	GL.Uniform2f(r.GLSL,Value.f0,Value.f1);
}
template<> void FX::DataFloat3::Load(FX::ShaderParam &r)
{
	GL.Uniform3f(r.GLSL,Value.f0,Value.f1,Value.f2);
}
template<> void FX::DataFloat4::Load(FX::ShaderParam &r)
{
	if(r.Type==GL_FLOAT_VEC3)
	GL.Uniform3f(r.GLSL,Value.f0,Value.f1,Value.f2);
	else
	GL.Uniform4f(r.GLSL,Value.f0,Value.f1,Value.f2,Value.f3);
}
template<> void FX::DataFloat2x2::Load(FX::ShaderParam &r)
{
	//The Cg type is vec2[2].
	//See Loading_script::EXPERIMENTAL_Cg_to_GLSL_order().
	if(r.Type==GL_FLOAT_MAT2)
	GL.UniformMatrix2fv(r.GLSL,1,GL_FALSE,&Value.f00);
	else //Assuming Cg float2[2].
	GL.Uniform2fv(r.GLSL,2,&Value.f00);
}
template<> void FX::DataFloat3x3::Load(FX::ShaderParam &r)
{
	//The Cg type is vec3[3].
	//See Loading_script::EXPERIMENTAL_Cg_to_GLSL_order().
	if(r.Type==GL_FLOAT_MAT3)
	GL.UniformMatrix3fv(r.GLSL,1,GL_FALSE,&Value.f00);
	else //Assuming Cg float3[3].
	GL.Uniform3fv(r.GLSL,3,&Value.f00);
}
template<> void FX::DataFloat4x4::Load(FX::ShaderParam &r)
{
	//The Cg type is vec4[4].
	//See Loading_script::EXPERIMENTAL_Cg_to_GLSL_order().
	if(r.Type==GL_FLOAT_MAT4)
	GL.UniformMatrix4fv(r.GLSL,1,GL_FALSE,&Value.f00);
	else //Assuming Cg float4[4].
	GL.Uniform4fv(r.GLSL,4,&Value.f00);
}			  
template<> void FX::DataType<double>::Load(FX::ShaderParam &r)
{
	//client data type//
	GL.Uniform1f(r.GLSL,(float)Value);
}
template<> void FX::DataType<RT::Matrix>::Load(FX::ShaderParam &r)
{	
	//client data type//
	#if 1==COLLADA_DOM_PRECISION
	float *v = Value;
	#else
	float v[16]; for(int i=0;i<16;i++) v[i] = (float) Value[i];
	#endif
	//The Cg type is vec4[4].
	//See Loading_script::EXPERIMENTAL_Cg_to_GLSL_order().
	if(r.Type==GL_FLOAT_MAT4)
	GL.UniformMatrix4fv(r.GLSL,1,GL_FALSE,v);
	else //Assuming Cg float4[4].
	GL.Uniform4fv(r.GLSL,4,v);	
}	
template<> void FX::DataSampler2D::Load(FX::ShaderParam &r) //SAMPLER
{
	int TexId = Value.FindTexID();
			
	if(Value.Cg!=nullptr)
	{
		//Not sure what this does. THe Cg stuff.
		cgGLSetTextureParameter(Value.Cg,TexId);		
	}
	//Reminder: GL_SAMPLER_2D is replaced with GL_TEXTURE0+i.
	GL.ActiveTexture(r.Type);
	//glEnable(GL_TEXTURE_2D);
	GL.Uniform1i(r.GLSL,r.Type-GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,TexId);
	if(Value.Cg!=nullptr)
	{
		cgSetSamplerState(Value.Cg);	
	}

	//LEGACY.
	//REMINDER: CrtTexture.cpp MAY'VE DONE THIS ALREADY.
	//calling this before generate mipmaps is better cause then the call
	//to generate mipmaps already has the space set up for it to use
	if(Value.GenerateMipmaps)
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
#define _(x) /*Do this after above (instantiation.)*/\
template<> void FX::DataSampler##x::Load(FX::ShaderParam &r)\
{\
	((FX::DataSampler2D*)this)->DataSampler2D::Load(r);\
}
_(1D)/*_(2D)*/_(3D)_(CUBE)_(RECT)_(DEPTH) //SAMPLER
#undef _
GLuint FX::Sampler::FindTexID(GLuint missing)
{
	FX::Surface *surf = Params->FindSurface(Source);
	if(surf==nullptr) return missing;
	
	//This inherits from previously defined surfaces.
	//Should this be done? Should samplers do so too?
	//NOTE: This is done when the surface is created
	//but it can be completed by an <instance_effect>.			
	surf->Refresh();
	
	assert(0!=surf->TexId); return surf->TexId;
}
GLuint FX::Sampler::FindTexID()
{
	GLuint out = FindTexID(0); if(out!=0) return out;

	//Can't spout off a warning at 60 frames per second.
	Collada05::const_image dummy;
	static bool nonce = false;
	if(nonce==false)
	{
		nonce = true;
		#ifdef NDEBUG
		#error Do this in an <instance_effect> validation step.
		#endif		
		daeEH::Warning<<"Could not locate <surface> or <instance_image> for "<<Source<<"\n"
		"(This warning is issued once per run only because it's generated in real-time.)";
	}
	static int missing = FX::Loader::GetID_TexId(dummy); return missing;
}

//-------.
	}//<-'
}

/*C1071*/
