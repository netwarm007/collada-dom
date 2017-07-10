/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <FX.pch.h> //PCH

#include "cfxData.h"
#include "cfxEffect.h"
#include "cfxPass.h"
#include "cfxLoader.h"

COLLADA_(namespace)
{
	namespace FX
	{		
	#ifndef YY
	#define YY 5 //MSVC2015 wants FX::??
	namespace ColladaYY = FX::Collada05;
	namespace ColladaYY_XSD = Collada05_XSD;
	#define _if_YY(x,y) x
	#endif //-.
//<-----------'

template<class S, class T>
inline void MakeData(S* &o, const T &e, FX::Paramable *p, xs::ID a1, typename S::arg2 a2=nullptr)
{
	FX::DataMaker<S> dm = {p,a1,a2,o,e->content}; FX::_if_YY(MakeData05,MakeData08)(dm);
}

typedef ColladaYY::const_profile_CG CG;
typedef ColladaYY::const_profile_GLSL GLSL;
typedef ColladaYY::const_profile_COMMON COMMON;

struct FX::Loader::Load
{
	struct Loading;
	struct Loading_script;
	struct Loading_gl_pipeline_setting;

	xs::const_any _profile_;
	FX::Loader &loading; const daeDocument *doc;
	Load(FX::Loader &l, const daeDocument *doc):loading(l),doc(doc){}

	template<class T> struct _less
	{
		bool operator()(T const *a, T const *b)const{ return *a<*b; } 
	};
	
	bool filter(daeName platform, xs::ID id)
	{
		if(platform!=nullptr) //Profile_COMMON
		{
			if(loading.platform_Filter.empty())
			{	
				#ifndef SN_TARGET_PS3
				if("PS3"==platform) //default behavior
				{			
					daeEH::Warning<<"Filtered out PlayStation 3 effect "<<id;
					return true;
				}
				#endif
			}
			else if(nullptr==loading.platform_Filter.find(platform))
			{
				daeEH::Warning<<"Filtered out "<<platform<<" effect "<<id;
				return true;
			}
		}
		return false;
	}

	#if YY==5
	#ifdef NDEBUG
	#error init_from is an array.
	#endif
	FX::Surface *new_Surface_05(const void *ID, FX::Surface *parent, bool sRGB)
	{
		//Don't want to trigger a missing texture error if setting
		//via a <setparam>.		
		Collada05::const_image yy; doc->idLookup(ID,yy);
		GLuint texId = ID==nullptr?0:FX::Loader::GetID_TexId(yy,sRGB);		
		return new FX::Surface(texId,parent);
	}
	#endif

	template<class T> 
	void newparam(const T &newparam, FX::Paramable *c)
	{		  
		if(newparam.empty()) return;

		for(size_t i=0;i<newparam.size();i++)
		{
			const typename T::XSD::type &newparam_i = *newparam[i];

			#if YY==5
			if(!newparam_i.surface.empty())
			{
				c->Surfaces.push_back(std::make_pair(newparam_i.sid.data(),
				new_Surface_05(newparam_i.surface->init_from->value->*xs::ID(),
				nullptr,newparam_i.surface->format_hint->option->value->*FX::ColorSpace())));
			}
			else //15.0.1 samplers will push their <instance_image> onto c->Params.
			#endif
			{
				FX::NewParam *p;
				FX::MakeData(p,&newparam_i,c,newparam_i.sid,newparam_i.semantic->value->*"");
				_try_annotate(newparam_i/*.annotate*/,p);
				//The annotations need to inform the semantic information.
				p->_InitSemantic_etc();

				c->Params.push_back(p);

				//This is a client hook for connecting the shared parameters
				//to client side states. Transform and lighting for instance.
				loading.Load_ClientData(p);
			}
		}

		//Sort into blocks of arrays. They really ought to be so sorted to begin
		//with, but it's not a constraint of COLLADA.
		std::sort(c->Params.begin(),c->Params.end(),_less<FX::Param>());
	}
	
	template<class newparam>
	static void _try_annotate(const newparam &np, FX::Annotatable *c)
	{
		Load_annotate(np.annotate,c);
	}
	static void _try_annotate(const DAEP::Schematic<COMMON::newparam>::type&,FX::Annotatable*)
	{ /*NOP*/ }
	template<class T> 
	FX::Param *_try_connect_param(const T&,FX::Paramable*){ return nullptr; }
	#if YY==5 //1.5.0 appears to limit <connect_param> to IK (kinematics.)
	FX::Param *_try_connect_param(const Collada05_XSD::cg_setparam &e, FX::Paramable *c)
	{
		return e.connect_param.empty()?nullptr
		#ifdef NDEBUG
		#error If ref is a <surface> the SetParam_To is not used but its constructor adds it.
		#endif
		:new FX::SetParam_To(c,e.ref,e.connect_param->ref);
	}
	#else //8
	template<class T> 
	bool _try_sampler_image(const T&,FX::Paramable*){ return false; }
	bool _try_sampler_image(const Collada08_XSD::instance_effect_type::local__setparam &e, FX::Paramable *c)
	{
		if(e.sampler_image.empty()) return false;

		FX::Surface *parent = c->Parent_Paramable->FindSurface(e.ref);		
		Collada08::const_image yy = xs::anyURI(e.sampler_image->url).get<Collada08::image>();		
		c->Surfaces.push_back(std::make_pair(e.ref.data(),
		new FX::Surface(FX::Loader::GetID_TexId(yy),parent))); return true;
	}
	#endif
	template<class T> 
	void setparam(const T &setparam, FX::Paramable *c)
	{		  
		for(size_t i=0;i<setparam.size();i++)
		{
			const typename T::XSD::type &setparam_i = *setparam[i];

			#ifdef NDEBUG
			#error And setparam_i.program? (Not in <instance_effect>.)
			#endif		
			#if YY==5
			if(!setparam_i.surface.empty())			
			{
				FX::Surface *parent = 
				c->Parent_Paramable->FindSurface(setparam_i.ref);
				//FxComposer files use <extra><technique profile="NVIDIA_FXCOMPOSER">.
				//assert(parent!=nullptr);

				c->Surfaces.push_back(std::make_pair(setparam_i.ref.data(),
				new_Surface_05(setparam_i.surface->init_from->value->*xs::ID(""),
				parent,setparam_i.surface->format_hint->option->value->*FX::ColorSpace())));
			}
			else
			#else
			if(!_try_sampler_image(setparam_i,c))
			#endif			
			{
				FX::Param *p = _try_connect_param(setparam_i,c);
				if(p==nullptr)
				{
					FX::MakeData((FX::SetParam*&)p,&setparam_i,c,setparam_i.ref);
					//NOT SUPPORTING (FOR NOW ANYWAY)
					//1.4.1 has <setparam><annotate> under spotty conditions for 
					//GLSL, GLES, and <generator> under CG and GLSL, but this is
					//removed in 1.5.0. Template-metaprogramming is not worth it.
					//NOTE: 1.5.0 seems to have dropped <generator> as a feature.
					//Load_annotate(setparam_i.annotate,p,c);
				}
				c->Params.push_back(p);	
			}
		}
	}

	//Profile_COMMON support
	template<class phong_base>
	void phong_or_blinn(phong_base &phong, FX::Technique *c)
	{	
		lambert(phong,c); 
		FX::Profile_COMMON.Load.Specular|=
		color_or_texture(phong.specular,FX::Profile_COMMON.Specular,c);			
		float_or_param(phong.shininess,FX::Profile_COMMON.Shininess,c);
	}	
	template<class lambert_base> 
	void lambert(lambert_base &lambert, FX::Technique *c)
	{	
		constant(lambert,c); 
		FX::Profile_COMMON.Load.Diffuse|=
		color_or_texture(lambert.diffuse,FX::Profile_COMMON.Diffuse,c);
		FX::Profile_COMMON.Load.Ambient|=
		color_or_texture(lambert.ambient,FX::Profile_COMMON.Ambient,c);
	}
	template<class constant_base>
	void constant(constant_base &constant, FX::Technique *c)
	{
		FX::Profile_COMMON.Load.Emission|=
		color_or_texture(constant.emission,FX::Profile_COMMON.Emission,c);			
		FX::Profile_COMMON.Load.Transparent|=
		color_or_texture(constant.transparent,FX::Profile_COMMON.Transparent,c);					
		float_or_param(constant.transparency,FX::Profile_COMMON.Transparency,c);			
		//color_or_texture(constant.reflective,FX::Profile_COMMON.Reflective,c);	
		//float_or_param(constant.reflectivity,FX::Profile_COMMON.Reflectivity,c);			
		//float_or_param(constant.index_of_refraction,FX::Profile_COMMON.RefractiveIndex,c);
	}
	void float_or_param(const ColladaYY_XSD::
	_if_YY(common_float_or_param_type,fx_common_float_or_param_type) *in, 
	FX::Profile_COMMON::Float &o, FX::Technique *c)
	{
		if(in!=nullptr) if(!in->param.empty())
		{
			c->Params.push_back(new FX::SetParam_To(&o,c,in->param->ref));
		}
		else if(!in->float__alias.empty())
		{
			struct SetFloat:FX::SetParam,FX::DataFloat
			{}*p = new SetFloat; 
			p->SetParamToSet(&o);
			p->Value = in->float__alias->value; c->Params.push_back(p);
		}
	}
	template<class T> 
	int color_or_texture(T &child, FX::Profile_COMMON::Color_or_Texture &o, FX::Technique *c)
	{	
		if(child.empty()) return 0; 
	
		const typename T::XSD::type &in = *child;				
		
		if(!in.texture.empty())
		{	
			FX::NewParam *x = nullptr;
			c->Params.push_back(new FX::SetParam_To(&o.Texture,c,in.texture->texture,&x));
			if(x!=nullptr) 			
			switch(x->OwnData().GetType())
			{
			case CG_FLOAT3: case CG_FLOAT4:

				//HACK: Colorizing the texture to pseudo-extend
				//profile COMMON's overreliance on baked texels.
				c->Params.push_back(new FX::SetParam_To(&o.Color,x));

			default:; //-Wswitch
			}
			return 1;
		}
		else if(!in.param.empty())
		{
			c->Params.push_back(new FX::SetParam_To(&o.Color,c,in.param->ref));
		}
		else if(!in.color.empty())
		{
			struct SetColor:FX::SetParam,FX::DataFloat4
			{}*p = new SetColor; 
			p->SetParamToSet(&o.Color); p->Value.a = 1;
			in.color->value->get4at(0,p->Value.r,p->Value.g,p->Value.b,p->Value.a);
			c->Params.push_back(p);
		}
		return 0;
	}

	//T is CG GLSL or COMMON (etc.)
	template<class T> void profile_(T,FX::Effect*);	
	template<class T> 
	bool _profile_shaders(typename T::technique::pass,FX::Pass*,FX::Effect*,xs::string);	
};
typedef DAEP::Schematic<ColladaYY::annotate>::content annotate;
static void Load_annotate(const annotate &annotate, FX::Annotatable *c)
{
	if(!annotate.empty())
	{
		for(size_t i=0;i<annotate.size();i++)
		{
			ColladaYY::const_annotate annotate_i = annotate[i];
			FX::Annotate *p;
			FX::MakeData(p,annotate_i,nullptr,annotate_i->name);
			c->Annotations.push_back(p);
		}
		//Sort according to SAS priorities. E.g. View or World.
		std::sort(c->Annotations.begin(),c->Annotations.end(),FX::Loader::Load::_less<FX::Annotate>());
	}
}

//HIDEOUS //HIDEOUS //HIDEOUS //HIDEOUS //HIDEOUS

struct FX::Loader::Load::Loading
{
	FX::Data *_param;		
	FX::Paramable *params; 
	Loading(FX::Paramable *p):params(p)
	{}
		
	//Value uses this to eliminate code for single
	//element render-states.
	const DAEP::Element *element;		

	template<class S, class T>
	void Get_value(int, const S &i, T *o)
	{
		*o = (T)i; 
	}
	template<class S, int N, class T>
	void Get_value(int M, const daeArray<S,N> &i, T *o)
	{
		*o = (T)i.data()[M]; 
	}	
	void Get_value(int M, const daeStringRef &t, int *o)
	{
		//texture_env_mode
		if(t=="ADD") *o = GL_ADD;
		else if(t=="BLEND") *o = GL_BLEND;
		else if(t=="DECAL") *o = GL_DECAL;
		else if(t=="MODULATE") *o = GL_MODULATE;		
		else if(t=="REPLACE") *o = GL_REPLACE; 			
	}
	template<class T> 
	void Get_param(int N, T *o)
	{
		#ifdef NDEBUG
		#error Not casting? Assuming size/same type?
		#endif
		*o = ((T*)&((FX::DataInt*)_param)->Value)[N];
		return; daeCTC<sizeof(int)==sizeof(T)>();
	}
	void Get_param(int N, unsigned *o)
	{
		//HACK: unsigned lets CGbool be overloadable.
		*o = (&((FX::DataBool1*)_param)->Value.b0)[N];
	}
	void Get_param(int, float **o)
	{
		*o = &((FX::DataFloat1*)_param)->Value.f0;
	}
	void Get_value(int,const ColladaYY_XSD::_if_YY(float4x4,float4x4_type) &m, float **o)
	{
		#if 1==COLLADA_DOM_PRECISION
		*o = const_cast<float*>(m.data()); }//#if
		#else		
		for(int i=0;i<16;i++)
		_temp_f16[i] = (float)m[i]; *o = _temp_f16; 
	}float _temp_f16[4*4];
	#endif
	template<int N, class S, class T>
	void Get(S &x, T *o)
	{
		//complex is enabling compilers to eliminate code.
		const void *object = &x->value__ATTRIBUTE.object();
		const bool complex = element!=object;
		if(nullptr==object&&complex)
		return;
		else if(!x->param->empty()) //ATTRIBUTE
		{
			//Assuming Pass->Apply() was called.
			//TODO? <material> isn't facilitated.
			//TODO? <animation> isn't facilitated
			if(N==0||complex)
			_param = params->FindSettingParam(x->param);
			if(_param!=nullptr)
			return Get_param(complex?0:N,o+N);
		}
		//NOTE: Cg casts int to float if the types are mixed.
		Get_value(N,*x->value__ATTRIBUTE.operator->(),o+N);
	}	
	template<int, class Sampler>
	void Get(Sampler &x, FX::Param **o)
	{
		if(!x->param.empty()) //ELEMENT
		{
			//NOTE: Samplers are not <animation> friendly.			
			if(params->FindNewParam(x->param->value,*o))
			return;
		}
		char i[] = {'$',(char)('0'+x->index),'\0'};
		FX::NewParam *p;
		FX::MakeData(p,x,params,i,i+2); //"" semantic?
		params->Params.push_back(*o=p);
	}	
	void SetStateAssignment(int *v, CGstateassignment Cg)
	{
		cgSetIntArrayStateAssignment(Cg,v);
	}
	void SetStateAssignment(float *v, CGstateassignment Cg)
	{
		cgSetFloatArrayStateAssignment(Cg,v);
	}
	void SetStateAssignment(float **m, CGstateassignment Cg)
	{
		cgSetFloatArrayStateAssignment(Cg,*m);		
	}
	void SetStateAssignment(unsigned *v, CGstateassignment Cg)
	{
		//Note: unsigned lets CGbool be overloadable.
		cgSetBoolArrayStateAssignment(Cg,(CGbool*)v);
	}
	void SetStateAssignment(FX::Param **s, CGstateassignment Cg)
	{
		FX::Sampler &hack = 
		((FX::DataSampler2D*)((*s)->SetData))->Value;
		cgSetSamplerStateAssignment(Cg,hack.Cg);
	}
	template<class T, class X>
	void Setting(CGstateassignment sa, X &x)
	{
		T v[1] = {}; Get<0>(x,v);
		SetStateAssignment(v,sa);	
	}
	template<class T, class X, class Y>
	void Setting(CGstateassignment sa, X &x, Y &y)
	{
		T v[2] = {}; Get<0>(x,v); Get<1>(y,v);
		SetStateAssignment(v,sa);	
	}
	template<class T, class X, class Y, class Z>
	void Setting(CGstateassignment sa, X &x, Y &y, Z &z)
	{
		T v[3] = {}; Get<0>(x,v); Get<1>(y,v); Get<2>(z,v);
		SetStateAssignment(v,sa);	
	}
	template<class T, class X, class Y, class Z, class W>
	void Setting(CGstateassignment sa, X &x, Y &y, Z &z, W &w)
	{
		T v[4] = {}; Get<0>(x,v); Get<1>(y,v); Get<2>(z,v); Get<3>(w,v);
		SetStateAssignment(v,sa);	
	}	
};
struct FX::Loader::Load::Loading_gl_pipeline_setting : public Load::Loading
{
	CGcontext Cg; CGpass Cg_Pass;

	Loading_gl_pipeline_setting(CGcontext Cg, FX::Pass *pass)
	:Loading(pass->Technique),Cg(Cg),Cg_Pass(pass->Cg)
	{
		//This is setting the <newparam> data up for this technique
		//so the <param> refs can access it. This work is incomplete
		//because <instance_effect> is supposed to be able to set the
		//<param> references and <animation> could modify them as well.
		pass->Technique->Apply();
	}

	//0 is not accepted for the non-array states.
	inline CGstateassignment
	__CreateStateAssignmentIndex(CGpass a, CGstate b, int i)
	{
		assert(b!=nullptr); //May as well do this here.
		if(i==0) return cgCreateStateAssignment(a,b);
		else return cgCreateStateAssignmentIndex(a,b,i);
	}

	//This is for use with daeContents::for_each_child().
	void operator()(const const_daeChildRef &maybe__gl_pipeline_setting)
	{
		_for_each_yy<YY>(maybe__gl_pipeline_setting);
	}
	template<int>
	void _for_each_yy(const const_daeChildRef &maybe__gl_pipeline_setting)
	{
		//This ensures genus cannot be mistaken.		
		if(1==maybe__gl_pipeline_setting.name())
		return;

		element = *maybe__gl_pipeline_setting;
		typedef ColladaYY::const_profile_CG::technique::pass _if_YY(,::states) 
		Lpass;
		//Note: There's a cgGetConnectedStateAssignmentParameter 
		//but no way to set it (CgFX can.)
		//FX::Pass would need an array of param connected states.
		//(That would have to be managed manually.)
		//COLLADA's param access may be more fine grained anyway.					
		switch(maybe__gl_pipeline_setting->getElementType())
		{
		//Must use long (unsigned) for 64 bit platforms' xs:long.
		#define i int(e->index->*0ULL)
		#define _(x,y,t,z,...) \
		case DAEP::Schematic<Lpass::z>::genus:\
		{ Lpass::z &e = *(Lpass::z*)&element;\
		Loading::Setting<t>(__CreateStateAssignmentIndex\
		(Cg_Pass,cgGetNamedState(Cg,#x),y),__VA_ARGS__);\
		}break;	
		#ifdef NDEBUG
		#error Add states that are new in 1.5.0 if any.
		#endif
		_(AlphaFunc,0,float,alpha_func,e->func,e->value__ELEMENT)
		_(AlphaTestEnable,0,unsigned,alpha_test_enable,e)
		#if YY==5
		_(AutoNormalEnable,0,unsigned,auto_normal_enable,e)
		#endif
		_(BlendEnable,0,unsigned,blend_enable,e)
		_(BlendFunc,0,int,blend_func,e->src,e->dest)
		_(BlendFuncSeparate,0,int,blend_func_separate,e->src_rgb,e->dest_rgb,e->src_alpha,e->dest_alpha)
		_(BlendEquation,0,int,blend_equation,e)
		_(BlendEquationSeparate,0,int,blend_equation_separate,e->rgb,e->alpha)
		_(BlendColor,0,float,blend_color,e,e,e,e)
		#if YY==5
		_(ClearColor,0,float,clear_color,e,e,e,e)		
		_(ClearStencil,0,int,clear_stencil,e)
		_(ClearDepth,0,float,clear_depth,e)
		#endif
		_(ClipPlane,i,float,clip_plane,e,e,e,e)
		_(ClipPlaneEnable,i,unsigned,clip_plane_enable,e)
		_(ColorLogicOpEnable,0,unsigned,color_logic_op_enable,e)
		_(ColorMask,0,unsigned,color_mask,e,e,e,e)
		_(ColorMaterial,0,int,color_material,e->face,e->mode)
		_(ColorMaterialEnable,0,unsigned,color_material_enable,e)
		_(CullFace,0,int,cull_face,e)
		_(CullFaceEnable,0,unsigned,cull_face_enable,e)	
		_(DepthBounds,0,float,depth_bounds,e,e)
		_(DepthBoundsEnable,0,unsigned,depth_bounds_enable,e)
		_(DepthClampEnable,0,unsigned,depth_clamp_enable,e)
		_(DepthFunc,0,int,depth_func,e)
		_(DepthMask,0,unsigned,depth_mask,e)
		_(DepthRange,0,float,depth_range,e,e)
		_(DepthTestEnable,0,unsigned,depth_test_enable,e)
		_(DitherEnable,0,unsigned,dither_enable,e)
		_(FogMode,0,int,fog_mode,e)
		_(FogDensity,0,float,fog_density,e)
		_(FogStart,0,float,fog_start,e)
		_(FogEnable,0,unsigned,fog_enable,e)
		_(FogEnd,0,float,fog_end,e)
		_(FogColor,0,float,fog_color,e,e,e,e)
		_(FogCoordSrc,0,int,fog_coord_src,e)
		_(FrontFace,0,int,front_face,e)
		_(LightAmbient,i,float,light_ambient,e,e,e,e)
		_(LightEnable,i,unsigned,light_enable,e)		
		_(LightConstantAttenuation,i,float,light_constant_attenuation,e)
		_(LightDiffuse,i,float,light_diffuse,e,e,e,e)
		_(LightingEnable,0,int,lighting_enable,e)
		_(LightLinearAttenuation,i,float,light_linear_attenuation,e)
		_(LightModelAmbient,0,float,light_model_ambient,e,e,e,e)	
		_(LightModelColorControl,0,int,light_model_color_control,e)
		_(LightModelLocalViewerEnable,0,unsigned,light_model_local_viewer_enable,e)
		_(LightModelTwoSidedEnable,0,unsigned,light_model_two_side_enable,e)
		_(LightPosition,i,float,light_position,e,e,e,e)
		_(LightQuadraticAttenuation,i,float,light_quadratic_attenuation,e)
		_(LightSpecular,i,float,light_specular,e)
		_(LightSpotCutoff,i,float,light_spot_cutoff,e)
		_(LightSpotDirection,i,float,light_spot_direction,e,e,e)
		_(LightSpotExponent,i,float,light_spot_exponent,e)
		_(LineSmoothEnable,0,unsigned,line_smooth_enable,e)
		_(LineStipple,0,int,line_stipple,e,e)
		_(LineStippleEnable,0,unsigned,line_stipple_enable,e)
		_(LineWidth,0,float,line_width,e)
		_(LogicOp,0,int,logic_op,e)
		_(LogicOpEnable,0,unsigned,logic_op_enable,e)
		_(MaterialAmbient,0,float,material_ambient,e,e,e,e)
		_(MaterialDiffuse,0,float,material_diffuse,e,e,e,e)
		_(MaterialEmission,0,float,material_emission,e,e,e,e)
		_(MaterialShininess,0,float,material_shininess,e)
		_(MaterialSpecular,0,float,material_specular,e,e,e,e)
		_(ModelViewMatrix,0,float*,model_view_matrix,e)		
		_(MultisampleEnable,0,unsigned,multisample_enable,e)
		_(NormalizeEnable,0,unsigned,normalize_enable,e)
		_(PointDistanceAttenuation,0,float,point_distance_attenuation,e,e,e)
		_(PointFadeThresholdSize,0,float,point_fade_threshold_size,e)
		_(PointSize,0,float,point_size,e)
		_(PointSizeMax,0,float,point_size_max,e)
		_(PointSizeMin,0,float,point_size_min,e)
		_(PointSmoothEnable,0,unsigned,point_smooth_enable,e)
		_(PolygonMode,0,int,polygon_mode,e->face,e->mode)
		_(PolygonOffset,0,float,polygon_offset,e,e)
		_(PolygonOffsetFillEnable,0,unsigned,polygon_offset_fill_enable,e)
		_(PolygonOffsetLineEnable,0,unsigned,polygon_offset_line_enable,e)
		_(PolygonOffsetPointEnable,0,unsigned,polygon_offset_point_enable,e)
		_(PolygonSmoothEnable,0,unsigned,polygon_smooth_enable,e)
		_(PolygonStippleEnable,0,unsigned,polygon_stipple_enable,e)
		_(ProjectionMatrix,0,float*,projection_matrix,e)
		_(RecaleNormalEnable,0,unsigned,rescale_normal_enable,e)
		_(SampleAlphaToCoverageEnable,0,unsigned,sample_alpha_to_coverage_enable,e)
		_(SampleAlphaToOneEnable,0,unsigned,sample_alpha_to_one_enable,e)
		_(SampleCoverageEnable,0,unsigned,sample_coverage_enable,e)
		_(Scissor,0,int,scissor,e,e,e,e)
		_(ScissorTestEnable,0,unsigned,scissor_test_enable,e)
		_(ShadeModel,0,int,shade_model,e)
		_(StencilFunc,0,int,stencil_func,e->func,e->ref,e->mask)
		_(StencilFuncSeparate,0,int,stencil_func_separate,e->front,e->back,e->ref,e->mask)
		_(StencilMask,0,int,stencil_mask,e)
		_(StencilMaskSeparate,0,int,stencil_mask_separate,e->face,e->mask)
		_(StencilOp,0,int,stencil_op,e->fail,e->zfail,e->zpass)
		_(StencilOpSeparate,0,int,stencil_op_separate,e->face,e->fail,e->zfail,e->zpass)
		_(StencilTestEnable,0,unsigned,stencil_test_enable,e)
		_(Texture1D,i,FX::Param*,texture1D,e)
		_(Texture1DEnable,i,unsigned,texture1D_enable,e)
		_(Texture2D,i,FX::Param*,texture2D,e)
		_(Texture2DEnable,i,unsigned,texture2D_enable,e)
		_(Texture3D,i,FX::Param*,texture3D,e)
		_(Texture3DEnable,i,unsigned,texture3D_enable,e)
		_(TextureCUBE,i,FX::Param*,textureCUBE,e)
		_(TextureCUBEEnable,i,unsigned,textureCUBE_enable,e)	
		_(TextureDEPTH,i,FX::Param*,textureDEPTH,e)
		_(TextureDEPTHEnable,i,unsigned,textureDEPTH_enable,e)
		_(TextureRECT,i,FX::Param*,textureRECT,e)
		_(TextureRECTEnable,0,unsigned,textureRECT_enable,e)
		_(TextureEnvColor,i,float,texture_env_color,e,e,e,e)
		_(TextureEnvMode,i,int,texture_env_mode,e)
		#undef _
		#undef i
		}				
	}
};

template<> //Reminder: COMMON depends on ColladaYY.
inline void FX::Loader::Load::profile_<COMMON>(COMMON profile_, FX::Effect *e)
{	
	//This is because RT monitors <newparam> values
	//as it's loading an effect. The profile_COMMON 
	//effects don't have any <newparam> to speak of.
	//It's also initializing the pseudo-params with
	//data that is not available at program startup.
	FX::Profile_COMMON.Init_ClientData(loading);

	Load &Load = *this;
	FX::Paramable *c = e;
	if(!profile_->newparam.empty())
	{
		c = new FX::Paramable(e);
		e->Profiles.push_back(c);
		Load.newparam(profile_->newparam,c);		
	}

	//create and populate a FX::Technique for every technique in the cg profile	
	for(size_t i=0;i<profile_->technique.size();i++)
	{	
		COMMON::technique technique = profile_->technique[i];
		daeEH::Verbose<<"Technique is "<<technique->sid;

		//create a FX::Technique for every technique inside the cg profile
		FX::Technique *tech = new FX::Technique(c,technique->sid);	
		#if YY==5
		Load.newparam(technique->newparam,tech);		
		//Load.setparam(technique->setparam,tech);
		#endif
		//Load_annotate(technique->annotate,tech);		

		bool blinn = false;

		if(!technique->constant.empty())
		{
			FX::Profile_COMMON.Load.Constant = 1;
			Load.constant(*technique->constant,tech);
		}
		else if(!technique->lambert.empty())
		{
			FX::Profile_COMMON.Load.Lambert = 1;
			Load.lambert(*technique->lambert,tech);
		}
		else if(!technique->phong.empty())
		{
			FX::Profile_COMMON.Load.Phong = 1;
			Load.phong_or_blinn(*technique->phong,tech);
		}
		else if(!technique->blinn.empty())
		{
			FX::Profile_COMMON.Load.Blinn = 1;
			blinn = true;
			Load.phong_or_blinn(*technique->blinn,tech);
		}
		else 
		{
			delete tech; continue;
		}

		//This is miscellaneous COMMON implementation data.
		struct SetApply:FX::SetParam,FX::DataProfile_COMMON
		{}*p = new SetApply; 
		p->Value.Technique = tech; 
		p->Value.Blinn.Value = blinn;
		p->SetParamToSet(&FX::Profile_COMMON.Application);
		tech->Params.push_back(p);

		e->Techniques.push_back(tech);		
	}	
}
template<class T>
inline void FX::Loader::Load::profile_(T profile_, FX::Effect *e)
{	
	if(filter(profile_->platform,e->Id)) 
	return;

	//Reminder: Cg must go first for this to work.		
	CGcontext Cg = nullptr;
	if(DAEP::Schematic<T>::genus==DAEP::Schematic<CG>::genus)
	Cg = loading.Cg;
		
	_profile_ = profile_; //Save for '08 shaders?	

	Load &Load = *this;
	FX::Paramable *c = e;
	if(!profile_->newparam.empty())
	{
		c = new FX::Paramable(e);
		e->Profiles.push_back(c);
		Load.newparam(profile_->newparam,c);		
	}

	//code in the profile elements may be used by one or more techniques
	#if YY==5
	Loading_script script(profile_->content);
	#else
	xs::string script = nullptr; //Ignore it?
	#endif

	//create and populate a FX::Technique for every technique in the cg profile	
	for(size_t i=0;i<profile_->technique.size();i++)
	{	
		typename T::technique technique = profile_->technique[i];
		daeEH::Verbose<<"Technique is "<<technique->sid;

		//at least one pass is needed for the technique to do anything 
		if(technique->pass.empty()) continue;

		//create a FX::Technique for every technique inside the cg profile
		FX::Technique *tech = new FX::Technique(c,technique->sid,Cg);	
		#if YY==5
		Load.newparam(technique->newparam,tech);		
		Load.setparam(technique->setparam,tech);
		#endif
		Load_annotate(technique->annotate,tech);		

		//This truncates to the <profile_*> and appends <technique>.
		#if YY==5
		script = technique->content;
		#endif
						
		//at least one pass is needed for the technique to do anything 
		for(size_t i=0;i<technique->pass.size();i++)
		{
			typename T::technique::pass pass = technique->pass[i];
			//now for drawing...  that isn't cgfx...  does it fit here???

			typename T::technique::pass _if_YY(,::evaluate) eval;
			#if YY==5
			eval = pass;
			#else
			if(!pass->evaluate.empty()) eval = pass->evaluate;
			#endif			
			if(!eval->draw.empty()&&"GEOMETRY"!=eval->draw->value)
			{
				#ifdef NDEBUG
				#error Is this for <scene> only? Or???
				#endif
				//what is the meaning of these targets?
				//(Surely they're for fullscreen mode.)
				//pass->color_target
				//pass->depth_target
				//pass->stencil_target
				//pass->color_clear
				//pass->depth_clear
				//pass->stencil_clear
				daeEH::Error<<"Not expecting <draw> mode "<<eval->draw->value;
				continue;
			}

			//This should be a name, is Sid OK?
			FX::Pass *p = new FX::Pass(tech,pass->sid);
			Load_annotate(pass->annotate,p);
			

			//Populate p->Settings via FX::MakeGlPipelineSetting().
			//(Most of the types of children fall into this class.)
			Loading_gl_pipeline_setting f(loading.Cg,p);
			pass->content->for_each_child(f);
			
			//TEMPLATE-META-PROGRAMMING
			//GLES doesn't have shaders.
			if(!Load._profile_shaders<T>(pass,p,e,script))
			{
				delete p; goto nonviable_pass;				
			}
			else tech->Passes.push_back(p);
		}
		if(tech->Passes.empty()) nonviable_pass:
		{
			delete tech;
			daeEH::Error<<"Nonviable or missing pass. Omitting technique "<<tech->Sid;
		}
		else e->Techniques.push_back(tech);		
	}	
}
template<> 
bool FX::Loader::Load::_profile_shaders<ColladaYY::const_profile_GLES>
(ColladaYY::const_profile_GLES::technique::pass,FX::Pass*,FX::Effect*,xs::string)
{ /*NOP*/ return true; }
template<class T> 
bool FX::Loader::Load::_profile_shaders
(typename T::technique::pass pass, FX::Pass *p, FX::Effect *e, xs::string script)
{
	#if YY==5
	typedef typename T::technique::pass Lprogram;
	Lprogram &program = pass;
	#else
	typedef typename T::technique::pass::program Lprogram;
	Lprogram program = pass->program;
	if(program==nullptr) return false;
	Loading_script script_08(_profile_);
	#endif
	for(size_t i=0;i<program->shader.size();i++)
	{
		typename Lprogram::shader shader = program->shader[i];
		#if YY==8
		if(!shader->sources.empty()) 
		script_08 = shader->sources->content; 
		else if(nullptr!=shader->compiler->binary)
		{
			daeEH::Warning<<"Skipping <binary> in COLLADA 1.5.0 shader.\n"
			"(This will be implemented on demand.)";
			continue;
		}
		else continue;
		#endif

		FX::Shader *sh = new FX::Shader
		(p,*shader->stage.operator->(),
		#if YY==8		
		shader->compiler->target->*"",		
		shader->compiler->options->*"",shader->sources->entry->*"",script_08);		
		#else
		shader->compiler_target->value->*"",		
		shader->compiler_options->value->*"",shader->name->value->*"",script);
		Load_annotate(shader->annotate,sh);
		#endif
		if(0==sh->Stage)
		{
			delete sh; continue;
		}

		//UNUSED
		//After Link() these may as well be deleted.
		p->Shaders.push_back(sh);	
	}
	//UNUSED
	//After Link() these may as well be deleted.
	//Sort according to vertex shader < geometry < fragment shader.
	std::sort(p->Shaders.begin(),p->Shaders.end(),_less<FX::Shader>());
	if(p->Shaders.size()<2||p->Shaders[0]->Stage==GL_GEOMETRY_SHADER)	
	return false;

	//Stage 2 (GLSL needs to glLinkProgram().)
	p->Link(); //GL.UseProgram(p->GLSL);	
	p->World = 0; //Initializing.
	for(size_t i=0;i<program->shader.size();i++)
	{
		typename Lprogram::shader shader = program->shader[i];
							 
		//load the shader's parameters
		#if YY==8
		#define bind bind_uniform
		#endif		
		for(size_t i=0;i<shader->bind.size();i++)
		{
			typename Lprogram::shader::bind bind = shader->bind[i];

			FX::ShaderParam *sp = p->FindShaderParam(bind->symbol);
			if(sp==nullptr)
			{
				daeEH::Warning<<"Could not locate shader variable "<<bind->symbol;
				continue;
			}
			if(sp->Param_To!=nullptr||sp->SetData!=nullptr)
			{
				daeEH::Warning<<"Shader program reloads uniform "<<bind->symbol<<"\n"
				"(GLSL loads uniforms as a unified shader-program.)";
				continue;
			}
			if(!bind->param.empty())
			{
				sp->SetParam_To(p,bind->param->ref);
				//HACK: Ensure there are not unbound pointers so that it can
				//point to a convenient NewParam type (not just to a Param.)
				if(sp->Param_To->IsNewParam()
				&&((FX::NewParam*)sp->Param_To)->IsWorld())
				{
					p->World++;

					//WORLD etc. go to the front of the line.
					intptr_t at = sp-&p->ShaderParams.front();
					if(at>=p->World)
					{	
						FX::ShaderParam cp = *sp;
						p->ShaderParams.erase(p->ShaderParams.begin()+at);
						p->ShaderParams.insert(p->ShaderParams.begin(),cp);
					}
				}
			}
			else FX::MakeData(sp,bind,p->Technique,bind->symbol);
		}
		#undef bind		
	}	
		
	//Do various cleanup tasks that don't require template specialization
	//and are not interesting to this file's algorithm.
	p->Finalize(); return true;
}

struct FX::Loader::Load::Loading_script : daeName
{
COLLADA_(public) //Previously FX::Technique
	/**
	 * From the looks of things there's no way to know
	 * if the code is binary or not.
	 */
	std::vector<char> script;

	size_t sizeof_profile_script;

	daeSIDREF SIDREF; daeName profile;
	Loading_script(xs::const_any &_08)
	:sizeof_profile_script()
	,SIDREF("",_08),profile(_08->getNCName())
	{
		EXPERIMENTAL_Cg_to_GLSL_order();
	}
	template<class T> Loading_script(T &profile_content)
	{	
		profile = profile_content->getElement()->getNCName();
		EXPERIMENTAL_Cg_to_GLSL_order();
		profile_content->for_each_child(*this);
		sizeof_profile_script = size();
	}
	template<class T> xs::string operator=(T &technique_content)
	{
		script.resize(sizeof_profile_script);
		script.back() = '\0';
		technique_content->for_each_child(*this); return string;
	}	

	/**
	 * This is part of an effort to unify the different profiles.
	 * Switching from Cg to GLSL but still relying on Cg to load
	 * the program has a number of quirks; one being that matrix
	 * uniforms are not mat4 but are vec4[4] for example. So the
	 * glUniformMatrix API will not work with them, and a manual
	 * transpose would be required (whether there's a difference 
	 * is probably important) to get back to the RT's historical
	 * layout. Injecting this #pragma into shaders will create a
	 * probably eventually.
	 */ 
	void EXPERIMENTAL_Cg_to_GLSL_order()
	{
		char test[] = "#pragma pack_matrix(column_major)\n";
		if(profile=="profile_CG")		
		script.assign(test,test+sizeof(test)-1);
	}

	/**C++98/03 SUPPORT
	 * This is used to fill out @c code in strict order.
	 */ 
	void operator()(const daeElement *e)
	{
		if(!script.empty()) script.pop_back();

	  //// POINT OF NO RETURN //////////////////

		//Care should be taken to extract these in order.
		//Not extracting the typeID because of frequency.
		#if YY==8		
		if("inline"==e->getNCName())
		{
			//IDENTICAL TO <code> BELOW.
			Collada08::const_inline code = e->a<Collada08::inline__alias>();
			if(code!=nullptr)
			script.insert(script.end(),code->value->begin(),code->value->end());
			goto done;
		}
		else if("import"==e->getNCName())
		{
			Collada08::const_import import = e->a<Collada08::import>();
			if(import==nullptr) goto done;
			SIDREF.setSIDREF(import->ref);
			e = SIDREF.getTargetedFragment(profile); //FALLING-THRU
			if(e==nullptr) goto done;
		}
		else goto done;
		#endif
		
		if("code"==e->getNCName())
		{
			//IDENTICAL TO <inline> ABOVE.
			ColladaYY::const_code code = e->a<ColladaYY::code>();
			if(code!=nullptr)
			script.insert(script.end(),code->value->begin(),code->value->end());
		}
		else if("include"==e->getNCName())
		{
			ColladaYY::const_include include = e->a<ColladaYY::include>();
			if(include!=nullptr) 
			{
				xs::anyURI URI = include->url; URI.resolve();

				const daeDOM &DOM = *URI.getDOM();
				daeIORequest req(&DOM,nullptr,&URI,&URI);

				#ifdef NDEBUG
				#error Would rather use req.fulfillRequestI.
				#endif
				//This is the system for mapping URIs to memory files.
				daeIOSecond<> I(req);
				daeIOEmpty O; 		
				daePlatform &sys = DOM.getPlatform();
				daeRAII::CloseIO _RAII(sys);
				daeIO *IO = _RAII = sys.openIO(I,O);
				if(IO==nullptr) 
				return;  

				size_t size = IO->getLock();
				script.resize(script.size()+size);
				if(!IO->readIn(&script.back()+1-size,size))
				assert(0);
			}goto done; //C4102
		}done:  
		extent = script.size();
		script.push_back('\0'); string = &script[0]; 
	}
};

//FX::Loader //FX::Loader //FX::Loader //FX::Loader //FX::Loader

FX::Effect *FX::Loader::Load(ColladaYY::const_effect effect)
{
	if(effect==nullptr) return nullptr;

	struct Load Load(*this,dae(effect)->getDocument());
	
	//SCHEDULED FOR REMOVAL
	//Need to establish if the effect is using Cg so its top-level
	//<newparam> samplers will be Cg API friendly.	
	CGcontext using_Cg = nullptr; if(Cg!=nullptr) 
	for(size_t i=0;i<effect->profile_CG.size();i++)
	if(!Load.filter(effect->profile_CG[i]->platform,effect->id))
	{
		using_Cg = Cg; break;
	}		
	FX::Effect *out = new FX::Effect(effect->id,using_Cg);		
	Load_annotate(effect->annotate,out);
	Load.newparam(effect->newparam,out);		

	//WHAT IS THE DEFAULT TECHNIQUE? <xs:sequence> IS USED?
	//el.addCM<XS::Element>(cm,4,1,-1).setChild(toc->profile_GLSL,"profile_GLSL");
	//el.addCM<XS::Element>(cm,4,1,-1).setChild(toc->profile_COMMON,"profile_COMMON");
	//el.addCM<XS::Element>(cm,4,1,-1).setChild(toc->profile_CG,"profile_CG");
	//el.addCM<XS::Element>(cm,4,1,-1).setChild(toc->profile_GLES,"profile_GLES");
		
	//GLSL (Not fully implemented.)
	#ifdef NDEBUG
	#error What is the "pipeline_settings" model?
	for(size_t i=0;i<effect->profile_GLSL.size();i++)
	Load.profile_<GLSL>(effect->profile_GLSL[i],out);	
	#endif	

	//Should profile_COMMON be slotted first or last?
	for(size_t i=0;i<effect->profile_COMMON.size();i++)	
	Load.profile_<COMMON>(effect->profile_COMMON[i],out);	

	//CG (Historically samples/ used CG.)
	if(Cg!=nullptr) 
	for(size_t i=0;i<effect->profile_CG.size();i++)
	Load.profile_<CG>(effect->profile_CG[i],out);	
	
	if(!out->Techniques.empty()) return out; delete out; return nullptr;
}
FX::Material *FX::Loader::Load(ColladaYY::const_material material, FX::Effect *e, const daeDocument *e_doc)
{
	if(e==nullptr||material==nullptr||e->Techniques.empty())
	return nullptr;
	
	ColladaYY::const_instance_effect instance_effect = material->instance_effect;

	#ifdef NDEBUG
	#error The caller needs to be able to override these
	#endif
	size_t i,j;
	for(i=j=0;i<instance_effect->technique_hint.size();i++)
	{
		ColladaYY::const_technique_hint hint = instance_effect->technique_hint[i];
		if(!hint->platform->empty()&&!platform_Filter.empty())
		if(nullptr==platform_Filter.find(hint->platform))
		{
			daeEH::Warning<<"Omitting <instance_effect> for platform hint "<<hint->platform;
			return nullptr;
		}
		xs::ID ref = instance_effect->technique_hint->ref;
		for(j=0;j<e->Techniques.size();j++)
		if(ref==e->Techniques[j]->Sid)
		goto have_hint;
	}
	j = 0; have_hint:
	FX::Material *out = new FX::Material(e->Techniques[j],material->id);

	struct Load Load(*this,e_doc); 
	Load.setparam(instance_effect->setparam,out);	

	FX::Technique *p = out->FindTechnique();
	if(p->IsProfile_CG()) 
	for(size_t i=0;i<p->Passes.size();i++)
	for(size_t j=0;j<p->Passes[i]->Shaders.size();j++)
	//if(nullptr==p->Passes[i]->Shaders[j]->Cg)
	if(0==p->Passes[i]->Shaders[j]->GLSL)
	{
		daeEH::Warning<<"Cg shader is unusable. Avoiding cgGL API crash by not using Cg for "<<material->id;
		delete out; return nullptr;	
	}

	daeEH::Verbose<<"Created material "<<material->id<<" effect "<<instance_effect->url;

	return out;
}

//-------.
	}//<-'
}

/*C1071*/
