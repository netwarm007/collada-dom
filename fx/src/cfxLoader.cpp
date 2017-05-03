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
	{//-.
//<-----'

struct FX::Loader::Load
{
	struct Loading;
	struct Loading_script;
	struct Loading_gl_pipeline_setting;

	FX::Loader &loading; const daeDocument *doc;
	Load(FX::Loader &l, const daeDocument *doc):loading(l),doc(doc){}

	#ifdef NDEBUG
	#error init_from is an array.
	#endif
	FX::Surface *new_Surface(const void *ID, FX::Surface *parent)
	{
		//Don't want to trigger a missing texture error if setting
		//via a <setparam>.
		GLuint texId = ID==nullptr?0:
		FX::Loader::GetID_TexId(doc->idLookup<Collada05::image>(ID));		
		return new FX::Surface(texId,parent);
	}

	template<class T> 
	void newparam(const T &newparam, FX::NewParamable *c)
	{		  
		for(size_t i=0;i<newparam.size();i++)
		{
			const T::XSD::type &newparam_i = *newparam[i];

			if(!newparam_i.surface.empty())
			{		  
				c->Surfaces.push_back(std::make_pair(newparam_i.sid.data(),
				new_Surface(newparam_i.surface->init_from->value->*xs::ID(),nullptr)));
			}
			else
			{
				FX::NewParam *p;
				FX::MakeData(p,&newparam_i,c,newparam_i.sid,newparam_i.semantic->value->*"");
				Load_annotate(newparam_i.annotate,p);

				c->Params.push_back(p);
			}
		}
	}
	template<class T> 
	FX::Param *_try_connect_param(T&,FX::NewParamable*){ return nullptr; }
	template<> 
	FX::Param *_try_connect_param(const Collada05_XSD::cg_setparam &e, FX::NewParamable *c)
	{
		return e.connect_param.empty()?nullptr
		:new FX::ConnectParam(e.ref,c,e.connect_param->ref,c->FindEffect());
	}
	template<class T> 
	void setparam(const T &setparam, FX::NewParamable *c)
	{		  
		for(size_t i=0;i<setparam.size();i++)
		{
			const T::XSD::type &setparam_i = *setparam[i];

			#ifdef NDEBUG
			#error And setparam_i.program? (Not in <instance_effect>.)
			#endif		
			if(!setparam_i.surface.empty())
			{
				//What is the parent for? Mipmaps??	
				FX::Surface *parent = 
				c->Parent_NewParamable->FindSurface(setparam_i.ref);
				assert(parent!=nullptr);

				c->Surfaces.push_back(std::make_pair(setparam_i.ref.data(),
				new_Surface(setparam_i.surface->init_from->value->*xs::ID(""),parent)));
			}
			else
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
	//<profile_CG> and <profile_GLSL> are virtually identical.
	template<class T> void profile_(T,FX::Effect*);
	template<class T> 
	void _profile_shaders(typename T::technique::pass,FX::Pass*,FX::Effect*,xs::string);	
};
typedef DAEP::Schematic<Collada05::annotate>::content annotate;
static void Load_annotate(const annotate &annotate, FX::Annotatable *c)
{
	for(size_t i=0;i<annotate.size();i++)
	{
		Collada05::const_annotate annotate_i = annotate[i];
		FX::Annotate *p;
		FX::MakeData(p,annotate_i,nullptr,annotate_i->name);
		c->Annotations.push_back(p);
	}
}
FX::Effect *FX::Loader::Load(Collada05::const_effect effect)
{
	if(effect==nullptr) return nullptr;

	struct Load Load(*this,dae(effect)->getDocument());
	//TODO: Implement GLSL etc.
	//2017: Trying to avoid allocating if omitted.
	//Is Apply() deferring set up?
	FX::Effect *out = nullptr;
	if(Cg!=nullptr)
	for(size_t i=0;i<effect->profile_CG.size();i++)
	{
		daeName platform = effect->profile_CG[i]->platform;
		if(!platform_Filter.empty())
		{
			
			if(nullptr==platform_Filter.find(platform))
			{
				daeEH::Warning<<"Omitting Cg effect profile for platform "<<platform;
				continue;
			}
		}
		else if("PS3"==platform) //default behavior
		{
			#ifndef SN_TARGET_PS3
			continue;
			#endif
		}
		if(out==nullptr)
		out = new FX::Effect(effect->id,Cg);
		Load.profile_<Collada05::const_profile_CG>(effect->profile_CG[i],out);
	}
	#ifdef NDEBUG
	#error This compiles, but won't link without FX::Shader.
	#error What is the "pipeline_settings" model?
	for(size_t i=0;i<effect->profile_GLSL.size();i++)
	{
		if(out==nullptr)
		out = new FX::Effect(effect->id);
		Load.profile_<Collada05::const_profile_GLSL>(effect->profile_GLSL[i],out);
	}
	#endif
	if(out!=nullptr)
	{
		Load_annotate(effect->annotate,out);
		Load.newparam(effect->newparam,out);	

		out->Apply();
	}
	return out;
}
FX::Material *FX::Loader::Load(Collada05::const_material material, FX::Effect *e, const daeDocument *e_doc)
{
	if(e==nullptr||material==nullptr||e->Techniques.empty())
	return nullptr;
	
	Collada05::const_instance_effect instance_effect = material->instance_effect;

	#ifdef NDEBUG
	#error The caller needs to be able to override these
	#endif
	size_t i = 0;
	if(!instance_effect->technique_hint.empty())
	{
		Collada05::const_technique_hint hint = instance_effect->technique_hint;
		if(!hint->platform->empty()&&!platform_Filter.empty())
		if(nullptr==platform_Filter.find(hint->platform))
		{
			daeEH::Warning<<"Omitting <instance_effect> for platform hint "<<hint->platform;
			return nullptr;
		}
		xs::ID ref = instance_effect->technique_hint->ref;
		while(i<e->Techniques.size()&&ref!=e->Techniques[i]->Sid)
		i++;
	}
	FX::Material *out = new FX::Material(e->Techniques[i],material->id);

	struct Load Load(*this,e_doc); 
	Load.setparam(instance_effect->setparam,out);	

	FX::Technique *p = out->FindTechnique();
	if(p->Cg!=nullptr) 
	for(size_t i=0;i<p->Passes.size();i++)
	for(size_t j=0;j<p->Passes[i]->Shaders.size();j++)
	if(nullptr==p->Passes[i]->Shaders[j]->Cg_Program)
	{
		daeEH::Warning<<"Cg shader is unusable. Avoiding cgGL API crash by not using Cg for "<<material->id;
		delete out; return nullptr;	
	}

	daeEH::Verbose<<"Created material "<<material->id<<" effect "<<instance_effect->url;

	return out;
}

//HIDEOUS //HIDEOUS //HIDEOUS //HIDEOUS //HIDEOUS

struct FX::Loader::Load::Loading
{
	FX::Data *_param;		
	FX::NewParamable *params; 
	Loading(FX::NewParamable *p):params(p)
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
	void Get_value(int,const Collada05_XSD::float4x4 &m, float **o)
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
			//TODO? <animation> isn't facilitated.
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
			FX::NewParamable *_;
			if(params->FindConnectingParam(x->param->value,*o,_))
			return;
		}
		char i[] = {'$','0'+char(x->index),'\0'};
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
		cgSetSamplerStateAssignment(Cg,(*s)->Cg);
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
	:Loading(pass->Technique),Cg(Cg),Cg_Pass(pass->Cg){}

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
		//This ensures genus cannot be mistaken.		
		if(1==maybe__gl_pipeline_setting.name())
		return;

		element = *maybe__gl_pipeline_setting;
		typedef Collada05::const_profile_CG::technique::pass 
		Lpass;
		//Note: There's a cgGetConnectedStateAssignmentParameter 
		//but no way to set it (CgFX can.)
		//FX::Pass would need an array of param connected states.
		//(That would have to be managed manually.)
		//COLLADA's param access may be more fine grained anyway.					
		switch(maybe__gl_pipeline_setting->getElementType())
		{		
		#define i e->index->*0		
		#define _(x,y,t,z,...) \
		case DAEP::Schematic<Lpass::z>::genus:\
		{ Lpass::z &e = *(Lpass::z*)&element;\
		Loading::Setting<t>(__CreateStateAssignmentIndex\
		(Cg_Pass,cgGetNamedState(Cg,#x),y),__VA_ARGS__);\
		}break;	
		_(AlphaFunc,0,float,alpha_func,e->func,e->value__ELEMENT)
		_(AlphaTestEnable,0,unsigned,alpha_test_enable,e)
		_(AutoNormalEnable,0,unsigned,auto_normal_enable,e)
		_(BlendEnable,0,unsigned,blend_enable,e)
		_(BlendFunc,0,int,blend_func,e->src,e->dest)
		_(BlendFuncSeparate,0,int,blend_func_separate,e->src_rgb,e->dest_rgb,e->src_alpha,e->dest_alpha)
		_(BlendEquation,0,int,blend_equation,e)
		_(BlendEquationSeparate,0,int,blend_equation_separate,e->rgb,e->alpha)
		_(BlendColor,0,float,blend_color,e,e,e,e)
		_(ClearColor,0,float,clear_color,e,e,e,e)
		_(ClearStencil,0,int,clear_stencil,e)
		_(ClearDepth,0,float,clear_depth,e)
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

template<class T>
void FX::Loader::Load::profile_(T profile_, FX::Effect *e)
{	
	Load &Load = *this;
	FX::NewParamable *c = e;
	if(!profile_->newparam.empty())
	{
		c = new FX::NewParamable(e);
		e->Profiles.push_back(c);
		Load.newparam(profile_->newparam,c);		
	}

	//code in the profile elements may be used by one or more techniques
	Loading_script script(profile_->content);

	//create and populate a FX::Technique for every technique in the cg profile	
	for(size_t i=0;i<profile_->technique.size();i++)
	{	
		T::technique technique = profile_->technique[i];
		daeEH::Verbose<<"Technique is "<<technique->sid;

		//at least one pass is needed for the technique to do anything 
		if(technique->pass.empty()) continue;

		//create a FX::Technique for every technique inside the cg profile
		FX::Technique *tech = new FX::Technique(c,technique->sid);			
		Load.newparam(technique->newparam,tech);
		Load.setparam(technique->setparam,tech);
		Load_annotate(technique->annotate,tech);

		//This truncates to the <profile_*> and appends <technique>.
		script = technique->content;
						
		//at least one pass is needed for the technique to do anything 
		for(size_t i=0;i<technique->pass.size();i++)
		{
			T::technique::pass pass = technique->pass[i];
			//now for drawing...  that isn't cgfx...  does it fit here???
			if(!pass->draw.empty()&&"GEOMETRY"!=pass->draw->value)
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
				daeEH::Error<<"Not expecting <draw> mode "<<pass->draw->value;
				continue;
			}

			//This should be a name, is Sid OK?
			FX::Pass *p = new FX::Pass(tech,pass->sid);
			Load_annotate(pass->annotate,p);
			
			//Populate p->Settings via FX::MakeGlPipelineSetting().
			//(Most of the types of children fall into this class.)				
			pass->content->for_each_child
			(Loading_gl_pipeline_setting(loading.Cg,p));
			
			//TEMPLATE-META-PROGRAMMING
			//GLES doesn't have shaders.
			if(!script.empty())
			Load._profile_shaders<T>(pass,p,e,script); 
			
			tech->Passes.push_back(p);
		}
		e->Techniques.push_back(tech);		
	}	
}
template<> 
void FX::Loader::Load::_profile_shaders<Collada05::const_profile_GLES>
(Collada05::const_profile_GLES::technique::pass,FX::Pass*,FX::Effect*,xs::string)
{ /*NOP*/ }
template<class T> 
void FX::Loader::Load::_profile_shaders
(typename T::technique::pass pass, FX::Pass *p, FX::Effect *e, xs::string script)
{
	for(size_t i=0;i<pass->shader.size();i++)
	{
		T::technique::pass::shader shader = pass->shader[i];
		if(shader->name.empty()) 
		continue;

		FX::Shader *sh = new FX::Shader
		(p,*shader->stage.operator->(), //C++98/03 support		
		shader->compiler_target->value->*"",		
		shader->compiler_options->value->*"",shader->name->value,script);
		Load_annotate(shader->annotate,sh);

		//load the shader's parameters
		for(size_t i=0;i<shader->bind.size();i++)
		{
			T::technique::pass::shader::bind bind = shader->bind[i];

			union{ FX::BindParam *b; FX::BindParam_To *b2; };

			if(!bind->param.empty())
			b2 = new FX::BindParam_To(bind->symbol,sh,bind->param->ref,e);
			else FX::MakeData(b,bind,e,bind->symbol,sh);

			sh->Params.push_back(b);
		}
		p->Shaders.push_back(sh);
	}
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

	template<class T> Loading_script(T &profile_content)
	{
		profile_content->for_each_child(*this);
		sizeof_profile_script = size();
	}
	template<class T> void operator=(T &technique_content)
	{
		script.resize(sizeof_profile_script);
		script.back() = '\0';
		technique_content->for_each_child(*this);
	}

	/**C++98/03 SUPPORT
	 * This is used to fill out @c code in strict order.
	 */ 
	void operator()(const daeElement &include__or__code)
	{
		if(!script.empty()) script.pop_back();

		//Care should be taken to extract these in order.
		//Not extracting the typeID because of frequency.
		if("code"==include__or__code.getNCName())
		{
			Collada05::const_code code = 
			include__or__code->a<Collada05::const_code>();
			if(code!=nullptr)
			script.insert(script.end(),
			code->value->begin(),code->value->end());
		}
		else if("include"==include__or__code.getNCName())
		{
			Collada05::const_include include = 
			include__or__code->a<Collada05::const_include>();
			if(include==nullptr) 
			return;

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
		}

		extent = script.size();
		script.push_back('\0'); string = &script[0]; 
	}
};

//-------.
	}//<-'
}

/*C1071*/