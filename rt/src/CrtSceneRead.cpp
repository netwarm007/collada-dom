/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <RT.pch.h> //PCH

#include "../../fx/include/cfxPass.h"

#include "CrtTypes.h" 
#ifndef YY
COLLADA_(namespace)
{	
	namespace RT
	{	
		#define YY 5 //MSVC2015 wants RT::??
		namespace ColladaYY = RT::Collada05;
		namespace ColladaYY_XSD = Collada05_XSD;	
		#define _if_YY(x,y) x
	}
}
#endif 
#include "CrtScene.h" //Checks YY
#include "CrtRender.h"
#include "CrtTexture.h"
#include "CrtLight.h"
#include "CrtCamera.h"
#include "CrtEffect.h"
#include "CrtGeometry.h"
#include "CrtController.h"
#include "CrtAnimation.h"
					
COLLADA_(namespace)
{	
	//cfxLoader.h
	//SCHEDULED FOR REMOVAL
	GLuint FX::Loader::GetID_TexId
	(RT::ColladaYY::const_image image, FX::ColorSpace sRGB)
	{
		if(image!=nullptr)
		{
			int out = RT::Main.DB->FindImage(image,sRGB)->TexId;
			if(out!=0) return out;
		}		
		return RT::Frame::Missing_Image_TexId();
	}
	RT::Image *RT::DBase::FindImage(RT::ColladaYY::const_image &e, bool sRGB)const
	{
		assert(RT::Main.Loading); //Delayed loading?
		return const_cast<RT::DBase*>(this)->LoadImage(e,sRGB);
	}

	namespace RT
	{//-.
//<-----'

	template<class T>
	/**HACK
	 * Assuming <rigid_body><technique_common> and
	 * <instance_rigid_body><technique_common> are 
	 * compatible with <shape> but ONLY WITH SHAPE.
	 */
	inline bool RigidBody::OverrideWith(T &in, xs::anyURI &URI)
	{
		//The schema requires this element, but it's very often empty.
		if(!in.technique_common.empty())
		{
			const typename T::local__technique_common *tc;
			tc = in.technique_common;
			Sid = in.sid;
			if(!tc->mass.empty())
			Mass = tc->mass->value;
			if(!tc->dynamic.empty()) 
			Dynamic = tc->dynamic->value;
			ColladaYY::const_physics_material yy;
			if(!tc->physics_material.empty())
			yy = tc->physics_material;			
			else if(!tc->instance_physics_material.empty())
			{
				URI = tc->instance_physics_material->url;
				yy = URI.get<ColladaYY::physics_material>();
			}
			if(yy!=nullptr)
			Material = COLLADA_RT_cast(physics_material,yy);
			_LoadShape(COLLADA_RT_cast(rigid_body::technique_common,tc));
		}
		return Shape!=nullptr;
	}

template<class T=xs::const_any> 
struct AccessorYY : RT::Accessor<T,ColladaYY::const_mesh::source>
{ 
	template<class TT>
	AccessorYY(const TT &c):RT::Accessor<T,ColladaYY::const_mesh::source>(c)
	{} 
};
template<> //This is short hand.
struct AccessorYY<float> : RT::AccessorYY<ColladaYY::const_float_array>
{
	template<class TT>
	AccessorYY(const TT &c):RT::AccessorYY<ColladaYY::const_float_array>(c)
	{}
};
	
template<class T>
inline void RT::Frame_Asset::_OverrideWith(const daeElement *e)
{
	typename DAEP::Schematic<T>::schema yy;

	T &t = *(T*)&e; if(!t->asset.empty()) 
	{	
		//Can't stop unless both are supplied.
		typename T::asset asset = t->asset;
		if(!asset->up_axis.empty())
		{
			if(!asset->unit.empty())
			Meter = asset->unit->meter;
			else _operator_YY(e->getParent(),yy);
			Up = asset->up_axis->value; return;
		}
		else if(!asset->unit.empty())
		{
			_operator_YY(e->getParent(),yy);
			Meter = asset->unit->meter; return;
		}		
	}
	_operator_YY(e->getParent(),yy);
} 
void RT::Frame_Asset::_operator_YY
(const daeElement *e, _if_YY(Collada05_XSD,Collada08_XSD)::__NB__ yy)
{
	if(e==nullptr)
	{
		Up = RT::Up::Y_UP; Meter = 1;
	}
	else switch(e->getElementType())
	{
	#define _(x) \
	case DAEP::Schematic<ColladaYY::x>::genus:\
	return _OverrideWith<ColladaYY::const_##x>(e);
	//This list is concerned with up_axis/unit.
	//(It may be incomplete.)		
	_(COLLADA)
	_(library_nodes)_(node)	
	_(library_visual_scenes)_(visual_scene)
	_(library_physics_scenes) _(physics_scene)
	_(library_physics_models) _(physics_model)		
	//Assuming <source><asset> doesn't specify up/meter.
	_(library_geometries)_(geometry)
	#undef _
	default: _operator_YY(e->getParent(),yy);
	}
}

//Adding this so these values can
//be hooked up if needed later on.
struct RT::DBase::LoadTargetable_of
{ 
	template<class S>
	LoadTargetable_of(S &in, float &v)
	{
		if(b=!in.empty()) v = in->value;
	}
	template<class S>
	LoadTargetable_of(S &in, double &v)
	{
		if(b=!in.empty()) v = in->value;
	}
	template<class S>
	LoadTargetable_of(S &in, FX::Float3 &v)
	{
		if(b=!in.empty()) in->value->get3at(0,v.x,v.y,v.z);
	}
	template<class S>
	LoadTargetable_of(S &in, FX::Float4 &v)
	{
		if(b=!in.empty()) in->value->get4at(0,v.x,v.y,v.z,v.w);
	}
	template<class S>
	LoadTargetable_of(S &in, RT::Matrix &v)
	{
		//This is tortured to avoid loading an identity matrix.
		const typename S::XSD::type *e;
		if(b=!in.empty()) e = in;
		else e=0; //-Wmaybe-uninitialized
		if(!b||e->value->size()<16) RT::MatrixLoadIdentity(v); 
		if(b) e->value->get4at(0,v[M00],v[M10],v[M20],v[M30]);
		if(b) e->value->get4at(4,v[M01],v[M11],v[M21],v[M31]);
		if(b) e->value->get4at(8,v[M02],v[M12],v[M22],v[M32]);
		if(b) e->value->get4at(12,v[M03],v[M13],v[M23],v[M33]);
		
	}
	operator bool(){ return b; } bool b;
};

struct RT::DBase::LoadLight_technique_common
{
	RT::Light *light;
	LoadLight_technique_common(RT::Light *l, 
	ColladaYY::const_light::technique_common in):light(l)
	{		
		if(in==nullptr) return;

		if(!in->ambient.empty())
		{
			light->Type = RT::Light_Type::AMBIENT;
			LoadTargetable_of(in->ambient->color,light->Color);
		}
		else if(!in->directional.empty())
		{
			light->Type = RT::Light_Type::DIRECTIONAL;
			LoadTargetable_of(in->directional->color,light->Color);
		}
		else if(!in->point.empty())
		{
			light->Type = RT::Light_Type::POINT;
			LoadTargetable_of(in->point->color,light->Color);
			Load_attenuation(*in->point);
		}
		else if(!in->spot.empty())
		{
			light->Type = RT::Light_Type::SPOT;
			ColladaYY::const_spot spot = in->spot;			
			LoadTargetable_of(spot->color,light->Color);
			Load_attenuation(*spot);
			LoadTargetable_of(spot->falloff_angle,light->FalloffAngle);
			LoadTargetable_of(spot->falloff_exponent,light->FalloffExponent);
		}
		//I can't convince myself this is correct for quadratic attentuation.
		//The correct way would be to pass the scale-factor to the lighting
		//model to scale the distances used to attenuate, but OpenGL doesn't
		//have a way to do that.
		l->LinearAttenuation/=RT::Asset.Meter;
		l->QuadraticAttenuation/=RT::Asset.Meter*RT::Asset.Meter;
	}
	template<class T> void Load_attenuation(T &in)
	{
		LoadTargetable_of(in.constant_attenuation,light->ConstantAttenuation);
		LoadTargetable_of(in.linear_attenuation,light->LinearAttenuation);
		LoadTargetable_of(in.quadratic_attenuation,light->QuadraticAttenuation);
	}
};
RT::Light *RT::DBase::LoadLight(ColladaYY::const_light &in)
{
	if(in==nullptr) return nullptr;
	RT::Light *out = GetLight(in);
	if(out!=nullptr) return out; //light was instanced
				   
	daeEH::Verbose<<"Adding new light "<<in->id;
						 
	out = COLLADA_RT_new(RT::Light);	
	out->Id = in->id; out->DocURI = RT::DocURI(in);
	
	LoadLight_technique_common(out,in->technique_common);

	Lights.push_back(out); return out;
}

struct RT::DBase::LoadCamera_technique_common
{
	RT::Camera *out;
	LoadCamera_technique_common(RT::Camera *cm,
	ColladaYY::const_optics::technique_common in):out(cm)
	{
		if(in!=nullptr)
		if(!in->orthographic.empty()) Load_shape(*in->orthographic);
		else if(!in->perspective.empty()) Load_shape(*in->perspective);		
		out->Refresh();
	}
	template<class T> void Load_shape(T &in)
	{
		Load_shape_2(in);
		LoadTargetable_of(in.aspect_ratio,out->Aspect);
		if(LoadTargetable_of(in.znear,out->ZNear)) 
		out->ZNear*=RT::Asset.Meter;
		if(LoadTargetable_of(in.zfar,out->ZFar)) 
		out->ZFar*=RT::Asset.Meter;
		out->Xmag*=RT::Asset.Meter;
		out->Ymag*=RT::Asset.Meter;
	}
	void Load_shape_2(const DAEP::Schematic<ColladaYY::orthographic>::type &in)
	{
		#ifdef NDEBUG //GCC can't stand apostrophes.
		#error "It should be easy to fix this since X/Ymag shouldn't be 0."
		#endif
		daeEH::Warning<<"Support for orthographic cameras is rudimentary.";
		LoadTargetable_of(in.xmag,out->Xmag); out->Xfov = 0;
		LoadTargetable_of(in.ymag,out->Ymag); out->Yfov = 0;
	}
	void Load_shape_2(const DAEP::Schematic<ColladaYY::perspective>::type &in)
	{
		LoadTargetable_of(in.xfov,out->Xfov); out->Xmag = 0; 
		LoadTargetable_of(in.yfov,out->Yfov); out->Ymag = 0;
	}
};
RT::Camera *RT::DBase::LoadCamera(ColladaYY::const_camera &in)
{
	if(in==nullptr) return nullptr;	
	RT::Camera *out = GetCamera(in);
	if(out!=nullptr) return out; //camera was instanced

	daeEH::Verbose<<"Adding new Camera "<<in->id;

	out = COLLADA_RT_new(RT::Camera);
	out->Id = in->id; out->DocURI = RT::DocURI(in);

	LoadCamera_technique_common(out,in->optics->technique_common);

	Cameras.push_back(out); return out;
}

template<class T> //sampler or control_vertices
struct CrtSceneRead_spline
{
	static const bool is_animation =
	DAEP::Schematic<ColladaYY::const_sampler>::genus==
	DAEP::Schematic<T>::genus;

	typedef const typename DAEP::Schematic<typename T::input>::type 
	SamplerInput;

	const daeDocument *doc; bool fat;
	CrtSceneRead_spline(const DAEP::Object *in)
	:doc(dae(in)->getDoc()->getDocument()) //...
	,POSITION(INPUT),LINEAR_STEPS(OUTPUT)
	,INPUT(doc),OUTPUT(doc)
	,IN_TANGENT(doc),OUT_TANGENT(doc),INTERPOLATION(doc)
	{
		assert(&doc<(void*)&INPUT); //C++		
	}	
	short BSPLINE,BSPLINE2;
	//Reminder: CONTINUITY is strictly notational.
	RT::AccessorYY<float> &POSITION,&LINEAR_STEPS;
	RT::AccessorYY<float> INPUT,OUTPUT,IN_TANGENT,OUT_TANGENT;
	RT::AccessorYY<ColladaYY::const_Name_array> INTERPOLATION;	
	void SetSamplerInput(const SamplerInput &in)
	{	
		if(in.semantic=="INPUT"
			 &&is_animation) INPUT.bind(&in);
		else if(in.semantic=="OUTPUT"
			 &&is_animation) OUTPUT.bind(&in);
		else if(in.semantic=="POSITION"
			 &&!is_animation) POSITION.bind(&in);
		else if(in.semantic=="IN_TANGENT") IN_TANGENT.bind(&in);
		else if(in.semantic=="OUT_TANGENT") OUT_TANGENT.bind(&in);
		else if(in.semantic=="INTERPOLATION") INTERPOLATION.bind(&in);
		else if(in.semantic=="LINEAR_STEPS"
			 &&!is_animation) LINEAR_STEPS.bind(&in);
		else daeEH::Warning<<"Unrecognized <sampler> semantic: "<<in.semantic;
	}		
	bool BindInputs(T &sampler)
	{
		if(sampler==nullptr) return false;

		//Each channel's <sampler> is almost required to be unique.		
		INPUT.bind(); IN_TANGENT.bind();
		OUTPUT.bind(); OUT_TANGENT.bind(); INTERPOLATION.bind();
		POSITION.bind(); LINEAR_STEPS.bind(); //CONTINUITY.bind();
		{
			for(size_t i=0;i<sampler->input.size();i++)
			SetSamplerInput(*sampler->input[i]);
		}		
		fat = IN_TANGENT!=nullptr&&OUT_TANGENT!=nullptr;
		
		RT::Name a,b;
		if(INTERPOLATION!=nullptr)
		{			
			INTERPOLATION->value->get1at(0,a);
			INTERPOLATION->value->get1at(INTERPOLATION.accessor->count-1,b);
		}
		BSPLINE = "BSPLINE"==a?1:0; BSPLINE2 = "BSPLINE"==b?1:0; return true;
	}	
	bool Sample(RT::Spline_Length &ch, RT::Spline_Point *pp, bool closed=false)
	{
		//HISTORY
		//This subroutine is hoisted up out of LoadAnimatin_channel 
		//so that it can be used with LoadGeometry_technique_common.
		ch.Algorithms = 0; RT::Spline_Point *p;
		
		//jN was designed for INPUT, so it's 1 less.
		size_t iN = ch.Points, jN = ch.Parameters-1;
		
		//Copying variable length packs is dicey!?!
		#ifdef NDEBUG
		#error This needs thought.
		#endif
		RT::AccessorYY<float>&OUTPUT_or_POSITION 
		= is_animation?OUTPUT:POSITION;
		const size_t o_s = OUTPUT_or_POSITION.accessor->stride;	
		const size_t o_o = OUTPUT_or_POSITION.accessor->offset;		
		const size_t OoR = o_o+o_s*iN-o_s+jN+(is_animation?0:1);
		if(OoR>OUTPUT_or_POSITION->value.size())
		{
			OUTPUT._out_of_range(); return false;
		}	
		const RT::Float *o_1 = &OUTPUT_or_POSITION->value[o_o]-1;
		if(!is_animation) //UNDO the -1?
		{
			 o_1+=1; assert(&OUTPUT_or_POSITION==&INPUT); //<spline>
		}

		if(INTERPOLATION!=nullptr)
		{
			p = pp; xs::ID a = nullptr, b = a;
			int algo = RT::Spline_Type::LINEAR;
			size_t i; for(i=0;i<iN;i++,p+=ch.PointSize) closed:
			{
				if(1==INTERPOLATION.get1at(i,a)&&a!=b) 
				{
					RT::Name c = b = a;
					if(c=="BEZIER"&&fat)
					algo = RT::Spline_Type::BEZIER;
					else if("LINEAR"==c)
					algo = RT::Spline_Type::LINEAR;
					else if("BSPLINE"==c)
					algo = RT::Spline_Type::BSPLINE;
					else if("HERMITE"==c&&fat)
					algo = RT::Spline_Type::HERMITE;
					else if("CARDINAL"==c&&fat)
					algo = RT::Spline_Type::CARDINAL;
					else if("STEP"==c)
					algo = RT::Spline_Type::STEP;					
					else daeEH::Error<<"Unrecognized INTERPOLATION mode "<<c
					<<(fat?" or missing OUT_TANGENT or IN_TANGENT.":"");
					ch.Algorithms|=algo; 
				}
				p->Algorithm = algo;
			}
			if(closed&&i==iN) goto closed;
		}
		else ch.Algorithms = RT::Spline_Type::LINEAR;
			
		p = pp;
		for(size_t i=0;i<iN;i++,p+=ch.PointSize)
		{
			RT::Float *params = p->GetParameters();
			if(is_animation) INPUT.get1at(i,params[0]);
			if(!is_animation) params[0] = o_1[0];
			for(size_t j=1;j<=jN;j++) params[j] = o_1[j]; o_1+=o_s;
		}
		if(fat) //Triplewide?
		{				
			//Special case 1-D tangent values?
			//HOW THE HECK CAN THIS BE DETECTED???
			const int special_case = is_animation
			&&OUT_TANGENT.accessor->param.size()==OUTPUT.accessor->param.size();

			//Copying variable length packs is dicey!?!
			#ifdef NDEBUG
			#error This needs thought.
			#endif			
			const RT::Float *ot = nullptr, *it = nullptr;
			const size_t ot_s = OUT_TANGENT.accessor->stride;	
			const size_t ot_o = OUT_TANGENT.accessor->offset;
			if(ot_o+ot_s*OUT_TANGENT.accessor->count<=OUT_TANGENT->value.size())
			ot = &OUT_TANGENT->value[ot_o]-special_case;
			else OUT_TANGENT._out_of_range(); 
			const size_t it_s = IN_TANGENT.accessor->stride;	
			const size_t it_o = IN_TANGENT.accessor->offset;
			if(it_o+it_s*IN_TANGENT.accessor->count<=IN_TANGENT->value.size())
			it = &IN_TANGENT->value[it_o]-special_case;
			else IN_TANGENT._out_of_range();
			if(it==nullptr||ot==nullptr) return false;

			p = pp; for(size_t i=iN;i-->0;p+=ch.PointSize)
			{
				RT::Float *params = p->GetParameters()+ch.Parameters;
				for(size_t j=special_case;j<=jN;j++) params[j] = ot[j]; ot+=ot_s;
				params+=ch.Parameters;
				for(size_t j=special_case;j<=jN;j++) params[j] = it[j]; it+=it_s;
			}

			//"Special case 1-D tangent values" 
			if(1==special_case&&is_animation)
			{
				daeEH::Error<<"VIOLATION OF SECTION \"Special case 1-D tangent values\" DETECTED!";
				daeEH::Warning<<"Such violations are more common than not; Following directive.";			

				p = pp; RT::Float i0,i1,i_1 = 0; //Read i-1.
				for(size_t i=iN-1;i-->0;i_1=i0,p+=ch.PointSize)
				{
					RT::Float *params = p->GetParameters();
					//OUT_TANGENT
					i0 = params[0];					
					i1 = params[ch.PointSize];
					params+=ch.Parameters;
					//NOTE: The manual has criss-crossed
					//control points. It may be a mistake.
					//These are uncrossed and rewritten to
					//be easily undestood.
					//It changes the length of the tangents
					//but they are linear.
					params[0] = i0+(i1-i0)/3; //i0*2/3+i1/3;
					if(p->Algorithm==RT::Spline_Type::HERMITE)
					params[0] = 3*(params[0]-i0);				
					//IN_TANGENT
					params+=ch.Parameters;
					params[0] = i_1+(i0-i_1)*2/3; //i_1/3+i0*2/3;
					if(p->Algorithm==RT::Spline_Type::HERMITE)
					params[0] = 3*(i0-params[0]);
				}//IN_TANGENT[iN-1]
				RT::Float *params = p->GetParameters(); p+=ch.PointSize;
				i0 = params[0];
				params+=ch.Parameters;
				params[0] = 0; //This must not be used.
				params+=ch.Parameters;
				params[0] = i_1+(i0-i_1)*2/3; //i_1/3+i0*2/3;
				if(p->Algorithm==RT::Spline_Type::HERMITE)
				params[0] = 3*(i0-params[0]);				
			}					
		}
		
		#ifdef NDEBUG
		#error Is there IN/OUT_TANGENT values if modes differ?
		#endif
		//MIRRORING/WRAPPING
		//Reminder: INTERPOLATION includes an additional
		//mode if closed that needs to be preserved here.
		RT::Spline_Point *ppN = pp+iN*ch.PointSize;		
		if(BSPLINE!=0) 
		{
			int k = ch.PointSize; p = pp-k; if(!closed) //Mirror?
			{
				p[0].Algorithm = RT::Spline_Type::BSPLINE;
				while(k-->1) p[k].Mirror(pp[k],pp[k+ch.PointSize]);
			}
			else while(k-->0) p[k] = ppN[k-ch.PointSize]; //Or wrap?
		}
		if(BSPLINE2!=0||closed)
		{
			int k = ch.PointSize; if(BSPLINE2!=0) //Mirror?
			{
				p = ppN-ch.PointSize; assert(!closed);
				ppN[0].Algorithm = RT::Spline_Type::BSPLINE;
				while(k-->1) ppN[k].Mirror(p[k],p[k-ch.PointSize]);
			}
			else while(k-->1) ppN[k] = pp[k]; //Or wrap?
		}		

		return true;
	}
};
struct RT::DBase::LoadAnimation_channel 
:
CrtSceneRead_spline<ColladaYY::const_sampler>
{	
	RT::Animation *out;
	LoadAnimation_channel(RT::Animation *out, ColladaYY::const_animation &in)
	:CrtSceneRead_spline(in),out(out)
	{
		for(size_t i=0;i<in->channel.size();i++) 
		Load(in->channel[i]);			
		if(out->Channels.empty()) daeEH::Warning<<"No <channel> remains/exists.";
	}		
	daeSIDREF SIDREF;	
	void Load(ColladaYY::const_channel in)
	{	
		daeRefRequest req;
		SIDREF = in->target;
		if(!SIDREF.get(req)||!req.isAtomicType())
		{
			#ifdef NDEBUG //GCC can't stand apostrophes.
			#error "If there is an element but no data, then use the value's capacity."
			#endif
			if(req.object!=nullptr)
			daeEH::Error<<"Animation target "<<in->target<<" is not understood.";
			return;
		}

		//This check is needed, because if a selection is not applied, the returned
		//length is the length of the string itself, which will treat each codepoint
		//as a keyframe parameter.
		switch(req.type->writer->getAtomicType())
		{
		case daeAtomicType::STRING: case daeAtomicType::TOKEN:

			daeEH::Error<<"Animation target is an untyped string "<<in->target<<"\n"<<
			"(Is the target an extensions?)";
			return;
		}

		ColladaYY::const_sampler sampler;			
		doc->idLookup(in->source,sampler);
		if(!BindInputs(sampler)||INPUT==nullptr||OUTPUT==nullptr)
		{	
			daeEH::Error<<"No <channel> INPUT and/or OUTPUT data for sampler "<<in->source;
			return;
		}			

		RT::Animation_Channel ch = in->target;
		RT::Target t(req.rangeMin,req.rangeMax);
		t.Fragment = req->a<xs::any>();
		ch.Target = t;
		size_t iN = INPUT.accessor->count;
		size_t jN = t.GetSize();			
		if(iN<2)
		{
			daeEH::Error<<"Degenerate animation INPUT count is "<<iN;
			return;
		}		

		bool transpose = false;
		//2017: ItemTargeted was added so not to encode
		//so much information for no purpose. But there
		//is still this problem of transposing <matrix>.
		if(!RT::Matrix_is_COLLADA_order
		&&nullptr!=req->a<ColladaYY::matrix>()) if(jN==1) 
		{
			ch.Target.Min =
			ch.Target.Max = ch.Target.Min%4*4+ch.Target.Min/4;
		}
		else if(jN!=16) //SID member selection is all-or-1.
		{
			daeEH::Error<<
			"Failed to transpose animated <matrix> with "<<jN<<" params/channels.\n"
			"(1 or 16 expected.)";
			return;
		}
		else transpose = true;
		
		ch.Points = (short)iN;
		ch.Parameters = (short)(1+jN);
		ch.PointSize = (short)1+ch.Parameters;
		if(fat) ch.PointSize+=2*ch.Parameters;
		BSPLINE*=ch.PointSize; BSPLINE2*=ch.PointSize;
		ch.SamplePointsMin = BSPLINE+(short)out->SamplePoints.size();
		
		const RT::Spline_Point def = { RT::Spline_Type::LINEAR,0 };
		out->SamplePoints.resize(ch.SamplePointsMin+iN*ch.PointSize+BSPLINE2,def);
		RT::Spline_Point *p,*pp = &out->SamplePoints[ch.SamplePointsMin];
		if(!Sample(ch,pp))
		return out->SamplePoints.resize(ch.SamplePointsMin-BSPLINE);
		
		if(transpose) //RT::Matrix_is_COLLADA_order?
		{
			p = pp;
			for(size_t i=0;i<iN;i++,p+=ch.PointSize)			
			{
				RT::Matrix4x4Transpose(p->GetParameters()+1);
				if(fat)
				RT::Matrix4x4Transpose(p->GetParameters()+1+16+1);
				if(fat)
				RT::Matrix4x4Transpose(p->GetParameters()+1+16+1+16+1);
			}
		}
		
		out->TimeMin = std::min(out->TimeMin,pp->GetParameters()[0]);
		out->TimeMax = std::max(out->TimeMax,pp[(iN-1)*ch.PointSize].GetParameters()[0]);

		//If BSPLINE is involved a global time-table is present.
		if(0!=(ch.Algorithms&RT::Spline_Type::BSPLINE))
		{
			//Allocate iN time records on the back and counting
			//backward fill them in, including the last point's
			//that is special because it doesn't have a segment.
			out->SamplePoints.resize(out->SamplePoints.size()+iN);
			pp = &out->SamplePoints[ch.SamplePointsMin];
			p = &out->SamplePoints.back()+1;
			RT::Float *t = (RT::Float*)p-1;			
			p-=2*ch.PointSize+BSPLINE2+iN;
			p->Fill(t--,0,1,p+ch.PointSize,1);
			for(;p>=pp;p-=ch.PointSize)
			p->Fill(t--,0,1,p+ch.PointSize,0);
			ch.BSPLINE_Real_TimeTable_1 = short(t-(RT::Float*)pp);
		}

		out->Channels.push_back(ch);		
	}
};
void RT::DBase::LoadAnimation(ColladaYY::const_animation &in)
{						
	assert(RT::Main.LoadAnimations);

	daeEH::Verbose<<"Adding new animation "<<in->id;

	RT::Animation *out = COLLADA_RT_new(RT::Animation);	
	out->Id = in->id; out->DocURI = RT::DocURI(in);

	LoadAnimation_channel(out,in);

	//also get it's last key time and first key time
	RT::Asset.TimeMin = std::min(RT::Asset.TimeMin,out->TimeMin);
	RT::Asset.TimeMax = std::max(RT::Asset.TimeMax,out->TimeMax);

	//RECURSIVE
	Animations.push_back(out);
	for(size_t i=0;i<in->animation.size();i++) 
	{
		ColladaYY::const_animation yy = in->animation[i];
		LoadAnimation(yy);	
	}
}

struct RT::DBase::LoadEffect_profile_COMMON
{
	RT::Effect *out;
	RT::DBase *dbase; 	
	ColladaYY::const_profile_COMMON profile_COMMON; 
	const daeDocument *doc;
	LoadEffect_profile_COMMON(RT::DBase *db, RT::Effect *ef, ColladaYY::const_effect &ef2)
	:out(ef),dbase(db),doc(dae(ef2)->getDocument())
	{
		#ifdef NDEBUG
		#error This only accesses the first profile_COMMON/technique.
		#endif
		profile_COMMON = ef2->profile_COMMON;
		if(profile_COMMON==nullptr)
		return;

		#if YY==5
		for(size_t i=0;i<profile_COMMON->image.size();i++)	
		{
			Collada05::const_image yy = profile_COMMON->image[i];
			dbase->LoadImage(yy);
		}
		#endif

		ColladaYY::const_profile_COMMON::technique technique;
		technique = profile_COMMON->technique;
		if(technique!=nullptr)
		{
			#if YY==5
			for(size_t i=0;i<technique->image.size();i++)
			{
				Collada05::const_image yy = technique->image[i];
				dbase->LoadImage(yy);
			}
			#endif

			//This support is really basic, since the shader models don't descend from a common type
			//we have to handle each one individually.  There can only be one in the technique.
			//All of them assume the texture is in the diffuse component for now.		
			if(!technique->constant.empty())
			Load_constant(*technique->constant);			
			else if(!technique->lambert.empty())
			Load_lambert(*technique->lambert);			
			else if(!technique->phong.empty())
			Load_phong(*technique->phong);
			else if(!technique->blinn.empty())
			Load_blinn(*technique->blinn);	
		}
	}
	template<class blinn_t> void Load_blinn(blinn_t &blinn)
	{
		out->Type = RT::Effect_Type::BLINN;
		Load_phong(blinn);
	}	
	template<class phong_base> void Load_phong(phong_base &phong)
	{
		out->Type = RT::Effect_Type::PHONG;
		Load_lambert(phong); 
		Load_color_or_texture(phong.specular,out->Specular);			
		Load_float_or_param(phong.shininess,out->Shininess);
	}	
	template<class lambert_base> void Load_lambert(lambert_base &lambert)
	{	
		out->Type = RT::Effect_Type::LAMBERT;
		//Try to use a DIFFUSE texture before EMISSION ones.
		Load_color_or_texture(lambert.diffuse,out->Diffuse);
		Load_constant(lambert);
		Load_color_or_texture(lambert.ambient,out->Ambient);		
	}
	template<class constant_base> void Load_constant(constant_base &constant)
	{
		out->Type = RT::Effect_Type::CONSTANT;
		Load_color_or_texture(constant.emission,out->Emission);	
		Load_color_or_texture(constant.reflective,out->Reflective);	
		Load_float_or_param(constant.reflectivity,out->Reflectivity);			
		Load_color_or_texture(constant.transparent,out->Transparent);			
		Load_float_or_param(constant.transparency,out->Transparency);			
		Load_float_or_param(constant.index_of_refraction,out->RefractiveIndex);
	}
	void Load_float_or_param(const ColladaYY_XSD::
	_if_YY(common_float_or_param_type,fx_common_float_or_param_type) *in, float &o)
	{
		if(in!=nullptr&&!LoadTargetable_of(in->float__alias,o)&&!in->param.empty())
		{
			ColladaYY::const_profile_COMMON::newparam newparam;
			dae(profile_COMMON)->sidLookup(in->param->ref,newparam,doc);
			if(newparam!=nullptr) LoadTargetable_of(newparam->float__alias,o);
		}
	}
	template<class T> void Load_color_or_texture(T &child, FX::Float4 &o)
	{	
		if(child.empty()) return; 
		
		const typename T::XSD::type &in = *child;				
		if(LoadTargetable_of(in.color,o)) return;
	
		typedef ColladaYY::const_profile_COMMON::newparam Lnewparam;
		Lnewparam newparam;
		if(!in.texture.empty())
		{
			//The COMMON profile doesn't modulate textures.
			o = FX::Float4(1);
			//The non-FX path won't ever use multi-texture.
			if(0!=out->Mono_TexId)
			return;

			FX::ColorSpace sRGB;
			ColladaYY::const_image yy;			
			#if YY==5
			//1.4.1 IS RIDICULOUS?!?!
			//The texture image is three levels of indirection away!						
			Lnewparam::sampler2D::source source = //1
			dae(profile_COMMON)->sidLookup(in.texture->texture,newparam,doc)->sampler2D->source; 			
			if(source!=nullptr)
			{
				Lnewparam::surface surface = //2
				dae(profile_COMMON)->sidLookup(source->value,newparam,doc)->surface;
				if(surface!=nullptr&&!surface->init_from.empty())
				{
					//REMINDER: init_from IS AN ARRAY.
					assert(1==surface->init_from.size());
					doc->idLookup(surface->init_from->value,yy); //3					
					if(!surface->format_hint.empty()) sRGB = surface->format_hint->option->value;
				}
			}
			#else //8
			xs::anyURI URI(dae(profile_COMMON)->sidLookup
			(in.texture->texture,newparam,doc)->sampler2D->instance_image->url->*"",doc);
			yy = URI.get<Collada08::image>();			
			#endif
			out->Mono_TexId = FX::Loader::GetID_TexId(yy,sRGB);			
		}
		else if(!in.param.empty())
		{
			#ifdef NDEBUG
			#error collada_spec_1_4.pdf About Parameters has guidelines on casting.
			#endif
			dae(profile_COMMON)->sidLookup(in.param->ref,newparam,doc);
			if(newparam!=nullptr)
			{
				//TODO: "RT::Newparam::cast" via database integration.
				if(!LoadTargetable_of(newparam->float4,o))
				if(!LoadTargetable_of(newparam->float3,o))
				daeEH::Error<<"Failed to cast color_or_texture_type param.";
			}			
		}
	}
};
RT::Effect *RT::DBase::LoadEffect(ColladaYY::const_effect &in)
{
	if(in==nullptr) return nullptr;
	RT::Effect *out = GetEffect(in);
	if(out!=nullptr) return out; //effect was instanced
	
	daeEH::Verbose<<"Adding new Effect "<<in->id;

	#if YY==5
	for(size_t i=0;i<in->image.size();i++)	
	{
		Collada05::const_image yy = in->image[i];
		LoadImage(yy);
	}
	#endif
	
	out = COLLADA_RT_new(RT::Effect);
	out->Id = in->id; out->DocURI = RT::DocURI(in);
	if(RT::Main.LoadEffects)
	out->FX = RT::Main.FX.Load(in);
	out->Missing_FX = out->FX==nullptr?1:0;
	RT::Main.FX.Load_Connect(out);

	RT::DBase::LoadEffect_profile_COMMON(this,out,in);	

	Effects.push_back(out); return out;
}

RT::Material *RT::DBase::LoadMaterial(ColladaYY::const_material &in)
{
	if(in==nullptr) return nullptr;	
	RT::Material *out = GetMaterial(in);
	if(out!=nullptr) return out; //material was instanced	

	const daeDocument *doc = dae(in)->getDocument();
	
	//find the effect that the material is refering too 
	ColladaYY::const_effect effect = 
	xs::anyURI(in->instance_effect->url->*"",doc).get<ColladaYY::effect>();
	RT::Effect *e = LoadEffect(effect);	
	if(e!=nullptr)
	{
		doc = dae(effect)->getDocument();
		daeEH::Verbose<<"Adding new Material "<<in->id<<"...\n"
		"Attaching effect "<<effect->id;
	}
	else e = RT::Stack::DefaultMaterial.Effect;
	
	out = COLLADA_RT_new(RT::Material(e));
	out->Id = in->id; out->DocURI = RT::DocURI(in);		
	out->FX = RT::Main.FX.Load(in,e->FX,doc);

	//SCHEDULED FOR REMOVAL
	//LoadCOLLADA does more with the materials 
	//once the shared profile_COMMON is online.

	Materials.push_back(out); return out;	
}

struct RT::DBase::LoadGeometry_technique_common 
{
	void Load_spline(ColladaYY::const_spline in)
	{	
		CrtSceneRead_spline<ColladaYY::const_spline::control_vertices>
		spline(inputs.array.document);

		ColladaYY::const_spline::control_vertices cv = in->control_vertices;
		if(!spline.BindInputs(cv)||spline.POSITION==nullptr)
		{
			daeEH::Error<<"No <spline> POSITION data for "<<out->Id;
			return;
		}

		assert(out->IsSpline()); //SHADOWING
		RT::Spline *out = (RT::Spline*)this->out;
		
		size_t iN = spline.POSITION.accessor->count;
		if(iN<2) return;
		size_t jN = spline.POSITION.accessor->param.size();
		if(jN==0) return;		
		out->Points = (short)iN;
		out->Parameters = (short)jN;
		out->PointSize = (short)1+out->Parameters;		
		if(spline.fat) out->PointSize+=2*out->Parameters;
		spline.BSPLINE*=out->PointSize; 
		spline.BSPLINE2*=in->closed?0:out->PointSize;
		out->Sense = spline.POSITION.sense();
		if(in->closed) out->Open = 0; 
		if(spline.BSPLINE!=0) out->BSPLINE_Entry = 1;
				
		const RT::Spline_Point def = { RT::Spline_Type::LINEAR,0 };
		out->SamplePoints.resize(spline.BSPLINE+(in->closed?iN+1:iN)*out->PointSize+spline.BSPLINE2,def);		
		if(!spline.Sample(*out,&out->SamplePoints[spline.BSPLINE],in->closed))	
		{
			out->SamplePoints.clear(); out->Points = 0; 
		}
	}
	template<class mesh_or_convex_mesh>
	void Load_mesh(mesh_or_convex_mesh &in)
	{
		ColladaYY::const_vertices vertices = in.vertices;
		if(vertices!=nullptr)
		{
			inputs = vertices->input;
			if(0!=inputs.normal.count) 
			out->Missing_Normals = false; //HACK
			out->Vertices = inputs.position.count;
			out->Positions = inputs.position.source;			
			out->Normals = inputs.normal.source;
			out->TexCoords = inputs.texture0.source;						
		}	
		//These parameters are humble triangulation codes.
		//0 for fans that pivot around 0.
		//1 for non-fan that repeat every 1.
		//2 for lines: repeating every 2.
		//3 for triangles: repeating every 3.
		//- implicates line primitives.		
		Load_p<0>(in.polylist); Load_p<3>(in.triangles);
		Load_p<0>(in.polygons); Load_p<1>(in.tristrips);		
		Load_p<0>(in.trifans);			
		Load_p<-2>(in.lines); Load_p<-1>(in.linestrips);	
	}
	RT::Geometry *out; 
	LoadGeometry_technique_common(RT::Geometry *out, ColladaYY::const_geometry &in)
	:out(out),inputs(dae(in)->getDoc()->getDocument())
	{		
		if(!in->mesh.empty())
		{
			Load_mesh(*in->mesh);		
		}
		else if(!in->convex_mesh.empty())
		{
			Load_mesh(*in->convex_mesh);
			//LoadGeometry_technique_common will not do this.
			assert(in->convex_mesh->convex_hull_of->empty());
		}
		else if(!in->spline.empty())
		{
			Load_spline(in->spline);
		}

		out->Generate_Positions(inputs.position_sense);

		//This sorts in order of material, format, then mode.
		std::sort(out->Elements.begin(),out->Elements.end());

		//TODO: ONCE THIS PACKAGE IS MATURE, HERE, MERGE THE
		//ELEMENT PACKS WHERE POSSIBLE, AND SEE ABOUT ADDING
		//A STRIPPER/FANNER TO THE MIX THAT ISN'T INTERESTED
		//IN CACHE-LOCALITY TO TRANSMIT LESS DATA TO SERVERS.
		//LESS DATA IS ALSO LESS WORK FOR THE CLIENT INDEXER.
	}

	struct Inputs //SINGLETON
	{	
		RT::AccessorYY<float> array;

		Inputs(const daeDocument *doc):array(doc)
		{} 	
		
		struct Input
		{	
			unsigned offset, count;

			Input():offset(),count(){}

			RT::Geometry_Semantic source;

		}position,normal,texture0;
		
		int position_sense; int max_offset;
		
		template<class input> void operator=(input &in)
		{
			max_offset = 0;
			position = normal = texture0 = Input();

			for(size_t i=0;i<in.size();i++)
			{
				Input *set = _SetOffset(*in[i]);
				if(set==nullptr) continue;
				
				array.bind(in[i]);
				set->source = COLLADA_RT_cast(float_array,array);
				if(array==nullptr) continue;

				size_t dimens = array.accessor->param.size();								
				if(dimens==0) continue;
				if(dimens<3) if(set==&position) //2-D or 1-D?
				{
					position_sense = array.sense(); //Which XYZ are present?
				}
				else if(set==&normal)
				{
					daeEH::Error<<"Unsupported NORMAL dimension is "<<dimens;
					continue;
				}
				if(set==&texture0) if(dimens==1)
				{
					daeEH::Error<<"Unsupported TEXCOORD dimension is 1.";
					continue;
				}
				else if(dimens>2)
				{
					daeEH::Warning<<"Slicing FX TEXCOORD to 2-D.";
				}

				set->count = array.accessor->count;
				set->source.Offset = array.accessor->offset;
				set->source.Stride = array.accessor->stride;				
				set->source.Dimension = dimens;
			}
		}
		Input *_SetOffset(const ColladaYY_XSD::
		_if_YY(InputLocal,input_local_type) &in_i)
		{
			if("POSITION"==in_i.semantic) return &position;			
			if("NORMAL"==in_i.semantic) return &normal;			
			return "TEXCOORD"==in_i.semantic?&texture0:nullptr;
		}
		Input *_SetOffset(const ColladaYY_XSD::
		_if_YY(InputLocalOffset,input_local_offset_type) &in_i)
		{
			int os = (int)in_i.offset;
			if(max_offset<=os) max_offset = os+1; 		
			if("VERTEX"==in_i.semantic)
			{
				//The VERTEX input refers to <vertices> 
				//as opposed to a proper <source> ref.
				position.offset = os; return nullptr; //&position;
			}
			else if("NORMAL"==in_i.semantic)
			{
				normal.offset = os;	return &normal;
			}
			else if("TEXCOORD"==in_i.semantic)
			{
				if(in_i.set==0u)
				{
					texture0.offset = os; return &texture0;
				}
				else daeEH::Error<<"Unsupported TEXCOORD set "<<+in_i.set;
			}
			return nullptr;
		}

	}inputs; enum{ triangulate_offsetN=3 };
		
	int triangulate_offset[triangulate_offsetN];
			
	int push_back_Elements(xs::string material, GLenum mode)
	{	
		int max, VERTEX_index = inputs.position.offset;		

		RT::Geometry_Elements e;
		e.Material_Instance_Symbol = material; e.Mode = mode;
		e.Normals = inputs.normal.source;		
		e.TexCoords = inputs.texture0.source;	
		//.Index is used as inputs below.
		e.Normals.Index = inputs.normal.offset;
		e.TexCoords.Index = inputs.texture0.offset;
		//e._Collapse_Index(inputs.position.offset);		
		{
		  //This is designed to make the VERTEX data
		  //come first, since it's stored implicitly.
		  //AND to remove vertex attributes from the
		  //inputs that RT::Geometry isn't ready for. 

			RT::Geometry_Semantic *it; 
			for(max=0,it=e.Begin();it<e.End();it++)		
			if(it->Index!=VERTEX_index&&*it!=nullptr)
			{
				if(0!=VERTEX_index) it->Index++;
				max = std::max<int>(max,it->Index);
			}
			else it->Index = 0;
			for(int i=1;i<=max;i++)
			{
				bool keep = false;
				for(it=e.Begin();it<e.End();it++)
				{
					if(i==it->Index) keep = true;
				}
				if(keep) continue; i--; max--;
				for(it=e.Begin();it<e.End();it++)
				if(i<(int)it->Index) it->Index--;
			}

			//Build a map from the Index values to the original offset.
			triangulate_offset[e.Normals.Index] = inputs.normal.offset;
			triangulate_offset[e.TexCoords.Index] = inputs.texture0.offset;
			triangulate_offset[0] = VERTEX_index; //Important this is last.
		}		
		e.Region = out->ElementBuffer.size(); assert(max<triangulate_offsetN); 
		
		out->Elements.push_back(e); daeCTC<3==triangulate_offsetN>(); return max+1;
	}
	template<int M, class T> void Load_p(T &in)
	{
		for(size_t i=0;i<in.size();i++)
		{
			const typename T::XSD::type &in_i = *in[i];	
			if(in_i.p.empty()) continue;

			inputs = in_i.input;
			if(0!=inputs.normal.count)  
			out->Missing_Normals = false; //HACK
			int indexes = //Unhandled vertex-attributes are discarded.
			push_back_Elements(in_i.material,M<0?GL_LINES:GL_TRIANGLES);	
			for(int i=0;i<indexes;i++)
			{
				triangulate_offset[0] = triangulate_offset[i];
				if(M*M>=2) Load_p_1<M>(in_i); 
				if(M*M<=1) Load_p_N<M>(in_i); 
				if(i==0) //Capture the number of triangulated indices.
				out->Elements.back().Width = 
				out->ElementBuffer.size()-out->Elements.back().Region;
			}	
			if(0==out->Elements.back().Width) out->Elements.pop_back();
		}
	}
	template<int M, class T> void Load_p_1(T &in_i)
	{									   
		ColladaYY::const_p p = in_i.p;
		Triangulate<M>(p,p->value->size()/inputs.max_offset);
	}	
	template<int M, class T> void Load_p_N(T &in_i)
	{
		for(size_t i=0;i<in_i.p.size();i++)
		{
			ColladaYY::const_p p = in_i.p[i];			
			Triangulate<M>(p,p->value->size()/inputs.max_offset);
		}
	}	
	template<int M> void Load_p_N(const ColladaYY_XSD::_if_YY(polylist,polylist_type) &in_i)
	{	
		//GCC/C++ make explicit-specialization impractical to impossible.
		daeCTC<M==0>();

		GLuint restart = 0;
		ColladaYY::const_p p = in_i.p;		
		ColladaYY::const_vcount vcount = in_i.vcount;
		//<vcount> is not required, but the manual doesn't say what to do without it.
		if(vcount==nullptr) 
		{	
			GLuint restartN = (GLuint)p->value->size()/inputs.max_offset;
			GLuint vcount_i = restartN/(GLuint)in_i.count;
			if(p->value->size()!=vcount_i*in_i.count*inputs.max_offset)			
			daeEH::Warning<<"<polylist><vcount> is absent and <polylist count> does\n"
			"not agree with the <p> data stride calculated by <input offset>.\n";			
			while(restart<restartN)
			{
				Triangulate<0>(p,vcount_i,restart); restart+=vcount_i;
			}
		}
		else for(size_t i=0;i<vcount->value->size();i++)
		{
			Triangulate<0>(p,vcount->value[i],restart); restart+=vcount->value[i];
		}
	}
	template<int M> void Triangulate(ColladaYY::const_p &p, size_t count, GLuint restart=0)
	{
		if(0==count) return;	

		enum{ nN = M<0?-M:M==0?1:M, tristrip=M==1 };

		std::vector<GLuint> &buf = out->ElementBuffer; 
		GLuint stride = inputs.max_offset;
		GLuint i = triangulate_offset[0]+restart*stride;
		GLuint iN = i+stride*(GLuint)count;
		GLuint index[nN], index0 = 0, index1 = 0;
		if(0!=p->value->size()%stride)
		{
			//Don't want trouble with RT::Geometry_Elements::Width.
			daeEH::Error<<"<p> size is not a multiple of <input> offsets.";
			return;
		}		

		if(M==0||tristrip) //0 is triangle fans.				
		{
			p->value->get1at(i,index0); i+=stride;
		}
		if(M==0||M==-1||tristrip) //- is lines. 1 is linestrip.
		{
			p->value->get1at(i,index1); i+=stride;
		}
		for(bool alt=false;i<iN;index1=index[nN-1],alt=!alt)
		{
			for(int n=0;n<nN;n++,i+=stride)
			if(1!=p->value->get1at(i,index[n]))
			daeEH::Error<<"Index into <p> is out-of-range.\n"
			"(Is <vcount> to blame?)";

			#ifdef NDEBUG
			#error Get visuals. Do tristrips alternate?
			#endif		
			if(tristrip&&alt) std::swap(index0,index1);
			if(M==0||tristrip) buf.push_back(index0);
			if(M==0||M==-1||tristrip) buf.push_back(index1);				
			for(int n=0;n<nN;n++) buf.push_back(index[n]);	
			if(tristrip&&!alt) index0 = index1;
		}
	}		
};
RT::Geometry *RT::DBase::LoadGeometry(ColladaYY::const_geometry &in)
{
	if(in==nullptr||!RT::Main.LoadGeometry)
	return nullptr;
	RT::Geometry *out = GetGeometry(in);
	if(out!=nullptr) return out; //geometry was instanced
	
	if(!in->convex_mesh.empty()
	 &&!in->convex_mesh->convex_hull_of->empty())
	{
		//Physics processes convex_hull_of. The physics library is computing the
		//hull. In the unlikely event this is entered, it will just fail to show.
		daeEH::Warning<<"Not visualizing \"convex_hull_of\" geometry "<<in->id;
		return nullptr;
	}

	daeEH::Verbose<<"Adding new Geometry "<<in->id;

	//Assuming <source><asset> doesn't specify up/meter.
	RT::Main_Asset _RAII(in);

	if(!in->spline.empty())
	out = COLLADA_RT_new(RT::Spline);
	else
	out = COLLADA_RT_new(RT::Geometry);
	out->Id = in->id; out->DocURI = RT::DocURI(in);

	LoadGeometry_technique_common(out,in);	
		 
	out->SetRange(out->Positions,out->Vertices);

	Geometries.push_back(out); return out;
}

RT::Image *RT::DBase::LoadImage(ColladaYY::const_image &in, bool sRGB)
{	
	//LoadImages is being applied to the image data, so non OpenGL
	//applications can access the image data.
	if(in==nullptr) //||!RT::Main.LoadImages)
	return nullptr;
	RT::Image *out = GetImage(in);
	if(out!=nullptr) //image was instanced
	{
		if(sRGB&&!out->sRGB) goto refresh; return out; 
	}

	daeEH::Verbose<<"Adding new image "<<in->id;

	out = COLLADA_RT_new(RT::Image);
	out->Id = in->id; out->DocURI = RT::DocURI(in);

	//crate_2d is required to specify sRGB color space.
	//Unfortunately DevIL lacks an sRGB information API.
	#if YY==8
	if(!in->create_2d.empty()) //Scoping goto refresh;
	{
		ColladaYY::const_create_2d create_2d = in->create_2d;
		
		if(!create_2d->format.empty())		
		if("sRGB"==create_2d->format->hint->space->*RT::Name())
		sRGB = true;

		//If there isn't maybe try the other? There should be
		//a warning here that this is a render-target or uses
		//<extra> for example.
		if(!create_2d->init_from.empty())
		out->_SetURL_or_hex_COLLADA_1_5_0(create_2d->init_from);
	}
	#endif

	if(!in->init_from.empty()) //Scoping goto refresh;
	{
		//REMINDER: This init_from is NOT an array.
		ColladaYY::const_image::init_from init_from = in->init_from; 

		#if YY==5
		//FYI: This is a relative URI (relative to DocURI.)
		out->URL = in->init_from->value;
		#else
		out->_SetURL_or_hex_COLLADA_1_5_0(in->init_from);
		#endif
	}

	//NOTE: If the image failed to load, there's no sense in
	//continually trying to reload it.
	Images.push_back(out);

refresh:

	//load the actual image passing in the current file name 
	//to first look relative to the current <file>_Textures
	//for the textures 
	out->sRGB = sRGB; out->Refresh(); return out;
}

struct RT::DBase::LoadController_skin
{
	////LoadGeometry_technique_common() in miniature.
	int inputs_max_offset, JOINT_offset, WEIGHT_offset;
	
	RT::Skin *out; 
	LoadController_skin(RT::Name &src,
	RT::Controller *con, ColladaYY::const_skin in)
	:inputs_max_offset(),JOINT_offset(-1),WEIGHT_offset(-1),
	out(dynamic_cast<RT::Skin*>(con)),WEIGHT_array(in)
	{
		src = in->source__ATTRIBUTE;

		LoadTargetable_of(in->bind_shape_matrix,out->BindShapeMatrix);		
	
		RT::AccessorYY<> array(in);
		ColladaYY::const_Name_array SIDs;
		ColladaYY::const_IDREF_array IDREFs;		
		ColladaYY::const_joints joints = in->joints;						
		if(joints!=nullptr)
		for(size_t i=0;i<joints->input.size();i++)
		{
			RT::Name semantic = array.bind(joints->input[i]);

			if("INV_BIND_MATRIX"==semantic)
			{
				//Maybe these should just be pointers, so
				//there's the one std::vector/less memory.
				array.get16(out->Joints_INV_BIND_MATRIX);
			}
			else if("JOINT"==semantic)
			{
				SIDs = array->a<ColladaYY::Name_array>();
				IDREFs = array->a<ColladaYY::IDREF_array>();
				out->Using_IDs_to_FindJointNode = IDREFs!=nullptr;
				if(out->Using_IDs_to_FindJointNode||SIDs!=nullptr)	
				array.get(out->Joints,SIDs->value->*IDREFs->value);
			}
		}
		//<skin> is done is Y_UP/1 space.		
		{
			if(RT::Asset.Up!=RT::Up::Y_UP)
			{
				#ifdef NDEBUG //GCC doesn't do apostrophes.
				#error "Try to understand/document this."
				#error "It's not swapping/flipping basis vectors."
				#error "Is it necessary to swap/cancel signs as with <matrix>?"
				#endif
				RT::Matrix up;
				RT::MatrixLoadAsset(up,RT::Asset.Up);				
				//Two multiplies translates into a coordinate system
				//producing a mutant identity matrix with two -1s if
				//necessary that cancel each other out.
				//See RT::Transform_Type::MATRIX about this swap biz.
				std::swap(up[M10],up[M01]); //HACK 
				std::swap(up[M21],up[M12]); //HACK
				RT::MatrixMult(up,out->BindShapeMatrix);
				std::swap(up[M10],up[M01]); //HACK 
				std::swap(up[M21],up[M12]); //HACK
				RT::MatrixMult(out->BindShapeMatrix,up,out->BindShapeMatrix);				
				for(size_t i=0;i<out->Joints_INV_BIND_MATRIX.size();i++)
				{
					std::swap(up[M10],up[M01]); //HACK 
					std::swap(up[M21],up[M12]); //HACK
					RT::MatrixMult(up,out->Joints_INV_BIND_MATRIX[i]);
					std::swap(up[M10],up[M01]); //HACK 
					std::swap(up[M21],up[M12]); //HACK
					RT::MatrixMult(out->Joints_INV_BIND_MATRIX[i],up,out->Joints_INV_BIND_MATRIX[i]);
				}
			}
			if(RT::Asset.Meter!=1)
			{
				out->BindShapeMatrix[M30]*=RT::Asset.Meter;
				out->BindShapeMatrix[M31]*=RT::Asset.Meter;
				out->BindShapeMatrix[M32]*=RT::Asset.Meter;
				for(size_t i=0;i<out->Joints_INV_BIND_MATRIX.size();i++)
				{
					out->Joints_INV_BIND_MATRIX[i][M30]*=RT::Asset.Meter;
					out->Joints_INV_BIND_MATRIX[i][M31]*=RT::Asset.Meter;
					out->Joints_INV_BIND_MATRIX[i][M32]*=RT::Asset.Meter;
				}
			}
		}

		ColladaYY::const_vertex_weights
		vertex_weights = in->vertex_weights;				
		if(vertex_weights!=nullptr)
		{
			ColladaYY::const_vertex_weights::input input;
			for(size_t i=0;i<vertex_weights->input.size();i++)
			{
				input = vertex_weights->input[i];

				inputs_max_offset = std::max<int>(inputs_max_offset,input->offset);

				if(input->set!=0u)
				{
					//This will require some work to rectify.
					#ifdef NDEBUG
					#error In theory sets can reuse sources; but seem to be purposeless.
					#endif
					daeEH::Error<<"Unsupported <vertex_weights> set "<<+input->set;
					continue;
				}

				int *offset;
				if("JOINT"==input->semantic)
				{
					offset = &JOINT_offset;
				}
				else if("WEIGHT"==input->semantic)
				{
					offset = &WEIGHT_offset; WEIGHT_array.bind(input);
				}
				else continue;

				if(-1!=*offset)
				if(input->offset==(unsigned long)*offset)
				{
					daeEH::Warning<<"Duplicate "<<input->semantic<<" offset in set 0.\n"<<
					"(Assuming JOINT/WEIGHT sources are simply concatenated in order.)";
				}
				else daeEH::Error<<"Two or more "<<input->semantic<<" offsets in set 0.";
				else *offset = input->offset;
			}
		}
		if(0==inputs_max_offset||JOINT_offset<0||WEIGHT_offset<0||WEIGHT_array==nullptr)
		return;
		inputs_max_offset+=1; 
	  //template<> void Load_p_N<0>(const ColladaYY_XSD::polylist &in_i)
	  //{	
			GLuint restart = 0;
			ColladaYY::const_v v = vertex_weights->v;	
			ColladaYY::const_vcount vcount = vertex_weights->vcount;
			if(v==nullptr||0!=v->value->size()%inputs_max_offset)
			{
				daeEH::Error<<"<v> size is not a multiple of <input> offsets.";
				return;
			}		
			//<vcount> is not required, but the manual doesn't say what to do without it.
			if(vcount==nullptr) 
			{	
				GLuint restartN = (GLuint)v->value->size()/inputs_max_offset;
				GLuint vcount_i = restartN/(GLuint)vertex_weights->count;
				if(v->value->size()!=vcount_i*vertex_weights->count*inputs_max_offset)			
				daeEH::Warning<<"<vertex_weights><vcount> is absent and <vertex_weights count> does\n"
				"not agree with the <v> data stride calculated by <input offset>.\n";			
				while(restart<restartN)
				{
					Weight(v,vcount_i,restart); restart+=vcount_i;
				}
			}
			else for(size_t i=0;i<vcount->value->size();i++)
			{
				Weight(v,vcount->value[i],restart); restart+=vcount->value[i];
			}
	  //}
	}
	RT::AccessorYY<float> WEIGHT_array;
	void Weight(ColladaYY::const_v &v, GLuint count, GLuint restart=0)
	{
		std::vector<RT::Skin_Weight> &buf = out->Weights; 
		
		GLuint stride = inputs_max_offset;		
		assert(stride==2);
		GLuint j = JOINT_offset+restart*stride;
		GLuint i = WEIGHT_offset+restart*stride; 
		GLuint iN = i+count*stride, sum_n = 0;				
		RT::Skin_Weight w; float sum = 0, rcp;
		float epsilon_10 = 10*std::numeric_limits<float>::epsilon();
		for(int x;i<iN;i+=stride,j+=stride)
		{
			if(1!=v->value->get1at(j,w.Joint)
			 ||1!=v->value->get1at(i,x))
			{
				daeEH::Error<<"Index into <v> is out-of-range.\n"
				"(Is <vcount> to blame?)";
				continue;
			}
			else if(1!=WEIGHT_array->value->get1at(x,w.Weight.x))
			{
				daeEH::Error<<"Index into WEIGHT <source> is out-of-range.\n";
				continue;
			}
			if(!/*NaN?*/(w.Weight.x>epsilon_10))
			{
				daeEH::Warning<<"Not rendering small vertex-weight "<<w.Weight.x;
				continue;
			}
			buf.push_back(w); sum+=w.Weight.x; sum_n++;
		}

		//COLLADA says <vertex_weights> are to be normalized if they are to be displayed.		
		if(0==sum_n)
		{
			//The RT::Controller algorithm needs a sum of 1 per vertex without omissions.
			w.Joint = -1; w.Weight.x = 1; buf.push_back(w);
		}
		else if(sum!=1)
		{
			//Assuming floating-point determinism.
			rcp = 1/sum; sum = 0;
			for(size_t i=buf.size()-count;i<buf.size();i++)
			sum+=buf[i].Weight.x*=rcp;
			if(sum>1) buf.back().Weight.x-=(sum-1)*2;
		}
	}
};
struct RT::DBase::LoadController_morph
{
	RT::Morph *out;
	LoadController_morph(RT::Name &src, 
	RT::Controller *con, RT::DBase *db, ColladaYY::const_morph in)	
	:out(dynamic_cast<RT::Morph*>(con))
	{
		src = in->source__ATTRIBUTE;
															
		//Will this shorthand work?
		//if(ColladaYY::morph::method_MorphMethodType::RELATIVE==in->method)
		if(in->method==in->method->RELATIVE)
		out->Using_RELATIVE_method = true;

		//COLLADA never actually supported morph animation???
		//https://www.khronos.org/collada/wiki/Morph_weights_KHR_extension		
		ColladaYY::const_float_array array_target;
		ColladaYY::const_morph::source source_target;
		
		RT::AccessorYY<> array(in); 
		size_t weightsN = 0, targetsN = 0;
		ColladaYY::const_targets targets = in->targets;		
		if(targets!=nullptr)
		for(size_t i=0;i<targets->input.size();i++)
		{
			RT::Name semantic = array.bind(targets->input[i]);
			
			size_t *counter;
			if("MORPH_TARGET"==semantic)
			counter = &targetsN;			
			else if("MORPH_WEIGHT"==semantic)
			counter = &weightsN;			
			else
			{
				if(!semantic.empty())
				daeEH::Error<<"Morph semantic "<<semantic<<" not supported.";
				continue;
			}

			size_t j,jN = array.accessor->count;
			if(out->MorphTargets.size()<*counter+jN)
			{
				//Reminder: there can be any number of inputs.
				out->MorphTargets.resize(*counter+jN);
			}
			RT::Morph_Target *j0 =
			&out->MorphTargets[*counter]; *counter+=jN;			
			if(&weightsN==counter)
			{					
				//NOTE: hack_morph_weight_id makes more sense. But,
				//-its support is poor, because it relies on SID-less SIDREFs,
				//-and the ID must be in the same document as the <animation><channel>.				
				array_target = array->a<ColladaYY::float_array>();
				source_target = //Removing RT::AccessorYY::source.
				dae(array)->getAncestor<ColladaYY::morph::source>();
				if(array_target!=nullptr) 
				for(j=0;j<jN;j++) 
				if(1!=array.get1at(j,j0[j].Weight,array_target->value))
				break;
			}
			else if(&targetsN==counter)
			{
				ColladaYY::const_IDREF_array 
				IDREF_array = array->a<ColladaYY::IDREF_array>();
				if(IDREF_array!=nullptr)
				{
					const void *id; //Avoiding daeStringRef construction.
					//Assuming the IDs are local to the containing array.
					//COLLADA is haphazardly laid out, but IDREFs aren't.
					const daeDocument *doc2 = dae(IDREF_array)->getDocument();
					for(j=0;j<jN&&1==array.get1at(j,id,IDREF_array->value);j++)
					{
						ColladaYY::const_geometry yy = 
						doc2->idLookup<ColladaYY::geometry>(id);
						j0[j].Vertices = db->LoadGeometry(yy);
						if(nullptr==j0[j].Vertices)
						daeEH::Error<<"Missed <morph> target. Are <controller> valid targets?";						
					}
				}
			}
			else assert(0); 
		}
		
		//SCHEDULED FOR REMOVAL (EXPONENTIAL LOOK UP)
		//check if there are animation on morph weight
		//REMINDER: ANIMATIONS SHOULD HAVE BEEN PRE-LOADED.
		RT::Animator a; size_t &j = a.Channel;
		for(size_t i=0;i<db->Animations.size();i++)					
		for(a.Animation=db->Animations[i],j=0;j<a.Animation->Channels.size();j++)
		{
			RT::Target &t = a.Animation->Channels[j].Target;
			if(source_target==t.Fragment||array_target==t.Fragment)
			out->Animators.push_back(a);
		}
	}
};
RT::Controller *RT::DBase::LoadController(ColladaYY::const_controller &in)
{
	if(in==nullptr) return nullptr;
	RT::Controller *out = GetController(in);
	if(out!=nullptr) return out; //controller was instanced

	daeEH::Verbose<<"Adding new controller "<<in->id;
							   
	RT::Main_Asset _RAII(in);

	RT::Name src = nullptr; RT::Skin *skin = nullptr;
	if(!in->skin.empty())
	LoadController_skin(src,out=skin=COLLADA_RT_new(RT::Skin),in->skin);
	else if(!in->morph.empty()) 
	LoadController_morph(src,out=COLLADA_RT_new(RT::Morph),this,in->morph);	
	else return nullptr;
	
	out->Id = in->id; out->DocURI = RT::DocURI(in);
	
	xs::const_any source = xs::anyURI(src,in)->get<xs::any>();
	ColladaYY::const_geometry yy = source->a<ColladaYY::geometry>();
	if(yy==nullptr)
	{
		ColladaYY::const_controller yy = source->a<ColladaYY::controller>();
		out->Source = LoadController(yy);	
	}
	else out->Geometry = LoadGeometry(yy);
	if(out->Geometry==nullptr) 
	if(out->Source==nullptr)
	{
		RT::Assert("Controller source not found.");
		COLLADA_RT_delete(out);	return nullptr;
	}
	else out->Geometry = out->Source->Geometry;

	//The first skin transforms into Y_UP/1 space.
	//<morph> on <skin> is too exotic to consider.
	if(skin!=nullptr&&0!=out->Geometry->Asset
	&&dynamic_cast<RT::Skin*>(out->Source)==nullptr)
	{
		RT::Matrix m; RT::Up_Meter um = 
		RT::GetUp_Meter(out->Geometry->Asset);
		RT::MatrixLoadAsset(m,um.first,um.second);
		RT::MatrixMult(m,skin->BindShapeMatrix);
	}

	Controllers.push_back(out); return out;
}

struct RT::DBase::LoadScene_Physics
{
	//These implement COLLADA's ridiculous constraint
	//attachment binding semantics.
	int bodies_model, bodies_body, bodies_constraint;

	//This is a hierarchical body instance identifier.
	int body(){ return bodies_model<<16|bodies_body; }

	xs::anyURI URI; daeSIDREF SIDREF;
	RT::DBase *dbase;	
	ColladaYY::const_visual_scene visual_scene;
	ColladaYY::const_physics_model physics_model;
	std::vector<std::pair<int,RT::PhysicsModel*>> bodies;
	LoadScene_Physics(RT::DBase *db, ColladaYY::const_scene &in)
	:URI(in->instance_visual_scene->url->*"",in)
	,SIDREF("",in),dbase(db),have_bodies()
	{		
		visual_scene = URI.get<ColladaYY::visual_scene>();
	}
	void Load(ColladaYY::const_scene &in)
	{
		if(!RT::Physics::OK||in->instance_physics_scene.empty()) 
		return;

		RT::Main.Physics.Init(); 

		ColladaYY::const_physics_scene physics_scene;
		for(size_t i=0;i<in->instance_physics_scene.size();i++)
		{	
			URI = in->instance_physics_scene[i]->url;
			physics_scene = URI.get<ColladaYY::physics_scene>();			 
			if(physics_scene==nullptr) continue;

			RT::Main_Asset _RAII(physics_scene/*->asset*/);
			#ifdef NDEBUG
			#error This is per scene gravity
			#endif
			ColladaYY::const_gravity gravity = physics_scene->technique_common->gravity;
			if(gravity!=nullptr)
			{
				RT::Float x,y,z; if(3==gravity->value->get3at(0,x,y,z))
				{	
					RT::Asset.Mult(x,y,z); RT::Main.Physics.SetGravity(x,y,z);	
				}
			}

			//Collision geometry depends on LoadGeometry().
			bool lg = RT::Main.LoadGeometry;
			if(!lg) RT::Main.LoadGeometry = true;
			for(size_t i=0;i<physics_scene->instance_physics_model.size();i++)
			{
				LoadInstancePhysicsModel(physics_scene->instance_physics_model[i]);
			}
			if(!lg) RT::Main.LoadGeometry = false;
		}
	}
	RT::PhysicsModel *LoadPhysicsModel(ColladaYY::const_physics_model in)
	{
		if(in==nullptr) return nullptr;
		RT::PhysicsModel *out = dbase->GetPhysicsModel(in);
		if(out!=nullptr) return out; //physics model was instanced
	
		RT::Main_Asset _RAII(in);
		out = new RT::PhysicsModel();
		out->Id = in->id; out->DocURI = RT::DocURI(in); 
		
		out->RigidBodies.reserve(in->rigid_body.size());
		{
			for(size_t i=0;i<in->rigid_body.size();i++)	
			{
				out->RigidBodies.push_back(RT::RigidBody());
				out->RigidBodies.back().OverrideWith(*in->rigid_body[i],URI);
			}
		}
		//Reminder: These only exist so Find_NCName/Find_non_NCName
		//can be repurposed to double for rigid_constraint look-ups.
		out->RigidConstraints.reserve(in->rigid_constraint.size());
		{
			for(size_t i=0;i<in->rigid_constraint.size();i++)	
			{
				ColladaYY::const_rigid_constraint yy = in->rigid_constraint[i];
				RT::RigidConstraint c(COLLADA_RT_cast(rigid_constraint,yy));
				if(nullptr!=c.rigid_constraint)
				out->RigidConstraints.push_back(c);
			}
		}

		dbase->PhysicsModels.push_back(out); return out;
	}
	void LoadInstancePhysicsModel(ColladaYY::const_instance_physics_model in)
	{
		URI = in->url;  //ColladaYY::const_physics_model
		physics_model = URI.get<ColladaYY::physics_model>();
		RT::PhysicsModel *model =
		LoadPhysicsModel(physics_model);
		if(model==nullptr) return;

		RT::Main_Asset _RAII(model->Asset);
		RT::RigidBody *lv1; 
		ColladaYY::const_instance_rigid_body lv2;		
		bodies_constraint = -1;
		for(size_t i=0;i<in->instance_rigid_body.size();i++)
		{
			//body is NCName, so this should be impossible.
			//But COLLADA is a mess, especially Physics, and
			//seems to not realize it or the type was mistaken.
			//This also supports 1.5.0 and user-modified schemas.
			lv2 = in->instance_rigid_body[i];			
			lv1 = FindBody(lv2->body,model);
			if(lv1!=nullptr)
			{	
				RT::RigidBody ib = *lv1; 
				if(ib.OverrideWith(*lv2,URI)) 
				{
					if(BindNode(lv2->target,ib,lv2)) 
					continue;
				}
				else daeEH::Error<<"No collision shape for physics body.";
			}
			daeEH::Error<<"Failed to instance physics body "<<lv2->body;
		}	

		//Assuming any constraints refer to bodies internal to the
		//the same model that defines them, or to an animated node.
		for(size_t i=0;i<in->instance_rigid_constraint.size();i++)
		{
			//<instance_rigid_constraint> is almost pointless. Presumably
			//it's required, and selectively enables/disables constraints.
			//It has a "sid" but it seems useless outside of <extra> data.
			RT::Name constraint = in->instance_rigid_constraint[i]->constraint;
			bodies_constraint = -1;
			RT::RigidConstraint *p;
			if(constraint.getID_slashed()) //FindConstraint(constraint,model);
			p = Find_non_NCName(&RT::PhysicsModel::RigidConstraints,constraint,model);
			else p = Find_NCName(&RT::PhysicsModel::RigidConstraints,constraint,model);			
			if(p==nullptr) 
			{
				daeEH::Error<<"Failed to instantiate Physics constraint "<<constraint;
				continue;
			}

			//Not sure what <asset> space applies to <rigid_constraint>. 
			//1) Could be the body/node on either attachment (complicated.)
			//2) Here using the <asset> of the contraint's <physics_model>.
			RT::Main_Asset _RAII2((bodies_model<0?model:bodies[bodies_model].second)->Asset);

			//UNTESTED
			//Because <instance_rigid_constraint> cannot bind to anything
			//this is supposed to limit searches for bodies to containing
			//physics models.
			bodies_constraint = bodies_model;			
			#ifdef NDEBUG
			#error Implement <node> attachments. 
			#endif
			if(nullptr!=FindBody(p->rigid_constraint->ref_attachment->rigid_body->getID_id(),model))
			{
				int body1 = body();
				if(nullptr!=FindBody(p->rigid_constraint->attachment->rigid_body->getID_id(),model))
				{
					RT::Main.Physics.Bind_instance_rigid_constraint
					(body1,body(),COLLADA_RT_cast(rigid_constraint,p->rigid_constraint));	
				}
			}
		}

	//From here down, "bodies" holds subordinate physics models, which are not
	//looked for until a body cannot be found in the top-level's physics model.

		if(have_bodies) bodies.clear(); have_bodies = false;
	}
	bool have_bodies; void CacheBodies()
	{
		have_bodies = true; assert(bodies.empty());
		
		//bodies is sorted by depth to meet the SIDREF-like "beadth-first" requirement.
		//Note, that the scenarios in which it is used are not necessarily SIDREF-like.
		CacheBodies_recursive(physics_model,1); std::sort(bodies.begin(),bodies.end());
	}
	RT::PhysicsModel *CacheBodies_recursive(ColladaYY::const_physics_model in, int depth, int parent=0)
	{
		if(in!=nullptr) for(size_t i=0;i<in->instance_physics_model.size();i++)
		{
			RT::PhysicsModel *p; URI = in->instance_physics_model[i]->url; 
			p = CacheBodies_recursive(URI.get<ColladaYY::physics_model>(),depth+1,(int)i);
			if(p!=nullptr) bodies.push_back(std::make_pair(depth<<16|parent,p));
		}
		return LoadPhysicsModel(in);
	}
	RT::RigidBody *FindBody(const RT::Name &body, RT::PhysicsModel *p)
	{
		if(!body.getID_slashed())
		return Find_NCName(&RT::PhysicsModel::RigidBodies,body,p);
		return Find_non_NCName(&RT::PhysicsModel::RigidBodies,body,p);
	}
	template<class T> T *Find_NCName
	(std::vector<T> RT::PhysicsModel::*p2m, const RT::Name &body, RT::PhysicsModel *p)
	{
		size_t j = bodies_constraint;
		if(-1==bodies_constraint) goto p0; 
		for(;j<bodies.size();j++)
		{
			p = bodies[j].second; p0: //p0
			for(size_t i=0;i<(p->*p2m).size();i++)
			if(body.string==(p->*p2m)[i].Sid)
			{
				bodies_model = (int)j; bodies_body = (int)i;
				return &(p->*p2m)[i];
			}
			if(!have_bodies) CacheBodies();
		}
		return nullptr;
	}
	template<class T> T *Find_non_NCName
	(std::vector<T> RT::PhysicsModel::*p2m, const RT::Name &body, RT::PhysicsModel *pp)
	{
		if(!have_bodies) CacheBodies(); SIDREF.setSIDREF(body);
		if('.'==body[0]) SIDREF.setParentObject(physics_model);

		daeSIDREFRequest req;
		if(!SIDREF.get(req)||nullptr==req->a<typename T::type>())
		return nullptr;
		
		for(size_t i=req.SID_by_SID.size()-1;i-->0;)
		switch(req.SID_by_SID[i]->getElementType())
		{
		case DAEP::Schematic<Collada05::instance_physics_model>::genus:
		req.SID_by_SID[i] = req.SID_by_SID[i+1]->getAncestor<Collada05::physics_model>();		
		break;
		case DAEP::Schematic<Collada08::instance_physics_model>::genus:
		req.SID_by_SID[i] = req.SID_by_SID[i+1]->getAncestor<Collada08::physics_model>();		
		case DAEP::Schematic<Collada05::physics_model>::genus: break;
		case DAEP::Schematic<Collada08::physics_model>::genus: break;
		default: req.SID_by_SID[i] = nullptr;
		}
		req.SID_by_SID.pop_back();
		
		RT::PhysicsModel *p = pp; int depth = 0, parent = 0;
		for(int sid=0,j=bodies_constraint;j<(int)bodies.size();j++)
		{
			if(sid==(int)req.SID_by_SID.size())
			{
				for(size_t i=0;i<(p->*p2m).size();i++)
				if(body.string==(p->*p2m)[i].Sid)
				{
					bodies_model = p==pp?-1:(int)j; bodies_body = (int)i;
					return &(p->*p2m)[i];
				}
				return nullptr;
			}

			//SCHEDULED FOR REMOVAL
			//This merely converts a DOM element into an RT handle.
			RT::PhysicsModel *q =
			LoadPhysicsModel(req.SID_by_SID[sid++]->a<ColladaYY::physics_model>());
			if(q==p||q==nullptr) continue;

			//0xFFFF tells the compiler to use two-byte addressing.
			for(size_t jj=j;j<(int)bodies.size()&&bodies[j].first>>16==(depth&0xFFFF);j++)
			if(p==bodies[j].second&&(depth<<16)+parent==bodies[j].first)
			{
				parent = int(j-jj); break;
			}							
			depth++;
		}
		assert(0); return nullptr;
	}	 

	//COLLADA 1.4.1/1.5 make this ridiculous.
	bool BindNode(const daeStringRef &target,
	RT::RigidBody &ib, ColladaYY::const_instance_rigid_body &iv)
	{
		RT::Name fragment = target.getID_id();
		const void *id = fragment.string;
		const daeDocument *doc,*doc2 =
		dae(visual_scene)->getDocument();
		if('#'!=target[0])
		{
			URI.setURI_and_resolve(target);
			fragment = URI.erase_fragment();
			if(URI.isSameDocumentReference())
			doc = doc2; else doc = //?: //Poor GCC.
			RT::Main.DOM.getDoc(URI)->_getDocument();
		}
		else doc = doc2;
		if(doc==nullptr) return false;
		daeSIDREFRequest req;
		ColladaYY::const_node node;				
		if('#'!=target[0]||target.getID_slashed())
		{
			SIDREF.setSIDREF(fragment);
			SIDREF.setParentObject(doc==doc2?
			(const DAEP::Object*)visual_scene:doc);
			if(!SIDREF.get(req)) return false;
			for(size_t i=0;i<req.SID_by_SID.size();i++)		
			switch(req.SID_by_SID[i]->getElementType())
			{
			case DAEP::Schematic<ColladaYY::instance_node>::genus:
			req.SID_by_SID[i] = req.SID_by_SID[i]->getParentElement();			
			case DAEP::Schematic<ColladaYY::node>::genus: break;
			default: req.SID_by_SID[i] = nullptr;
			}
			node = req->a<ColladaYY::node>();
			req.SID_by_SID.pop_back();
		}
		else if(doc->idLookup(id,node)==nullptr) //optimizing
		{
			dae(visual_scene)->sidLookup(id,node);
		}
		#ifdef NDEBUG
		#error Does <instance_node> make sense?
		#endif
		if(node==nullptr) return false;
		
		bool initial_velocity = 
		!iv->technique_common.empty()
		&&(!iv->technique_common->velocity.empty()
		||!iv->technique_common->angular_velocity.empty());

		int o = 0;
		#ifdef NDEBUG
		#error This is an exponential lookup.
		#endif
		for(size_t i=0;i<RT::Main.Stack.Data.size();i++)
		{
			//Fragment could be removed if daeDatabse
			//is set up to integrate these RT classes.
			RT::Stack_Data &d = RT::Main.Stack.Data[i];			
			if(node!=d.Node->Fragment) 
			continue;
			
			//Eliminate the candidate if any SIDs are 
			//not among its parent nodes in the stack.
			RT::Stack_Data *p = d.Parent;
			for(size_t i=req.SID_by_SID.size();i-->0;)
			if(nullptr!=req.SID_by_SID[i])
			while(p!=nullptr&&p->Node->Fragment!=req.SID_by_SID[i])
			p = p->Parent;
			if(p==nullptr) continue;
			
			if(o++!=0)
			daeEH::Warning<<"Binding physics body to many instanced nodes "<<target;
			RT::Main.Physics.Bind_instance_rigid_body(body(),ib,d);
			if(initial_velocity)
			RT::Main.Physics.Init_velocity(d,COLLADA_RT_cast(instance_rigid_body,iv));
		}
		return o!=0;
	}
};
void RT::DBase::LoadScene(ColladaYY::const_scene &scene)
{	
	//Worried about initializing daeRef with a nullptr.
	if(scene==nullptr) return;
	//Just using the URI and Physics needs the visual_scene.
	LoadScene_Physics Physics(this,scene); 	
	ColladaYY::const_visual_scene &Visuals = Physics.visual_scene;
	if(nullptr==Visuals) return;

	//get the scene name 
	RT::Main.Scene.Name = Visuals->name;
	//Probably shouldn't.
	RT::Main.Scene.Id = Visuals->id;
	
	daeEH::Verbose<<"Loading Collada Scene "<<Visuals->name;

	RT::Main_Asset _RAII(Visuals->asset);
	//recurse through the scene, read and add nodes 
	for(size_t i=0;i<Visuals->node.size();i++)
	{
		ColladaYY::const_node yy = Visuals->node[i];
		LoadNode(yy,-1,&RT::Main.Scene);		
	}
	
	//Physics depends on this step.
	//The "stack" holds the scene-graph nodes-
	//which are distinct from the DOM's nodes.	
	RT::Main.Stack.Select(&RT::Main.Scene);
	{
		if(RT::Main.UsePhysics) Physics.Load(scene);	
	}
}

struct RT::DBase::LoadInstances_of
{
	RT::Node *out; RT::DBase *dbase;
	RT::Geometry *geom;
	LoadInstances_of(RT::DBase *db, RT::Node *out, ColladaYY::const_node in)	
	:out(out),dbase(db),geom()
	{
		//Process <instance_*> children.
		Load(out->Geometries,in->instance_geometry);
		Load(out->Controllers,in->instance_controller);
		Load(out->Lights,in->instance_light);
		Load(out->Cameras,in->instance_camera);
	}
	template<class S, class T> void Load(std::vector<S*> &iv, T &in)
	{
		for(size_t i=0;i<in.size();i++)
		{
			S *ii = Load_i(*in[i]); if(ii!=nullptr) iv.push_back(ii);
		}
	}
	RT::Light *Load_i(const ColladaYY_XSD::
	_if_YY(instance_light,instance_light_type) &in)
	{
		ColladaYY::const_light yy = 
		xs::anyURI(in.url)->get<ColladaYY::light>();
		return dbase->LoadLight(yy);
	}
	RT::Camera *Load_i(const ColladaYY_XSD::
	_if_YY(instance_camera,instance_camera_type) &in)
	{
		ColladaYY::const_camera yy = 
		xs::anyURI(in.url)->get<ColladaYY::camera>();
		return dbase->LoadCamera(yy);
	}	
	RT::Geometry_Instance *Load_i(const ColladaYY_XSD::
	_if_YY(instance_geometry,instance_geometry_type) &in)
	{
		ColladaYY::const_geometry yy = 
		xs::anyURI(in.url).get<ColladaYY::geometry>();
		geom = dbase->LoadGeometry(yy);
		if(geom==nullptr) 
		return nullptr;

		RT::Geometry_Instance *ig = COLLADA_RT_new(RT::Geometry_Instance);
		ig->Geometry = geom;

		ColladaYY::const_bind_material::technique_common
		technique_common = in.bind_material->technique_common;
		if(technique_common!=nullptr)
		for(size_t i=0;i<technique_common->instance_material.size();i++)
		Load_i_material(ig->Materials,technique_common->instance_material[i]);

		return ig;
	}
	RT::Controller_Instance *Load_i(const ColladaYY_XSD::
	_if_YY(instance_controller,instance_controller_type) &in)
	{
		xs::anyURI URI = in.url;
		ColladaYY::const_controller yy = URI.get<ColladaYY::controller>();
		RT::Controller *con = dbase->LoadController(yy);
		if(con==nullptr) //controller not found
		return nullptr;

		daeEH::Verbose<<"Instancing Controller "<<in.name;

		RT::Controller_Instance *ic = COLLADA_RT_new(RT::Controller_Instance);
		ic->Controller = con; geom = con->Geometry;
	
		ColladaYY::const_bind_material::technique_common 
		technique_common = in.bind_material->technique_common;
		if(technique_common!=nullptr)
		for(size_t j=0;j<technique_common->instance_material.size();j++)
		Load_i_material(ic->Materials,technique_common->instance_material[j]);

		#ifdef NDEBUG
		#error Support #SIDREF skeletons. And track <instance_node>?
		#endif	
		ic->Skeletons = in.skeleton.size();
		ic->Joints.resize(ic->Skeletons);
		for(size_t i=0;i<ic->Skeletons;i++)
		{
			URI = in.skeleton[i]->value;
			ColladaYY::const_node skel = URI.get<ColladaYY::node>();
			//This is cleaned up after.
			if(skel==nullptr) continue;

			ic->Joints[i] = dbase->LoadNode(skel);

			ColladaYY::const_node joint;
			const daeDocument *doc = dae(skel)->getDocument();

		//TODO? Warn if joints are bound more than once.
		//TODO? Warn if a skeleton has no joints inside.

			size_t jN = ic->Skeletons;
			for(RT::Controller *p=con;p!=nullptr;p=p->Source)
			{	
				RT::Skin *skin = dynamic_cast<RT::Skin*>(p);
				if(skin==nullptr) continue;

				//+1 and 1+ account for a <skeleton> entry.
				size_t j = jN; jN+=skin->Joints.size();
				if(jN>ic->Joints.size()) 
				ic->Joints.resize(jN);				
				for(size_t i=0;i<skin->Joints.size();i++)
				{
					//sidLookup does not follow <instance_node> joints.
					//These SIDs are xs:Name, so they cannot be SIDREFs.
					const void *ref = skin->Joints[i];
					if(skin->Using_IDs_to_FindJointNode)
					doc->idLookup(ref,joint);
					else dae(skel)->sidLookup(ref,joint,doc);
					RT::Node *found = dbase->LoadNode(joint);
					if(found==nullptr) continue;
					ic->Joints[j+i] = found;				
					daeEH::Verbose<<"Skin Controller "<<skin->Id<<" joint binding made to node "<<skin->Joints[i];
				}

				for(size_t i=0;i<skin->Joints.size();i++)
				if(nullptr==ic->Joints[j+i])
				daeEH::Warning<<"Failed to make joint binding for Controller "<<skin->Id<<" for Joint "<<skin->Joints[i];	
			}
		}
		for(size_t i=0;i<ic->Skeletons;i++) if(nullptr==ic->Joints[i])		
		{									
			ic->Joints.erase(ic->Joints.begin()+i); ic->Skeletons--; i--;
		}
		//YUCK: Fill in joints with nullptr so the system will not crash?
		if(ic->Joints.empty())	
		for(RT::Controller *p=con;p!=nullptr;p=p->Source)
		{	
			RT::Skin *skin = dynamic_cast<RT::Skin*>(p);
			if(skin!=nullptr&&!skin->Joints.empty()) 
			{			
				if(ic->Joints.empty())
				daeEH::Error<<"Skin controller did not find joints "<<out->Id;
				ic->Joints.resize(ic->Joints.size()+skin->Joints.size());
			}
		}
		return ic;
	}
	void Load_i_material(std::vector<RT::Material_Instance> &imv,
	//Reminder: <evaluate_scene> adds its own <instance_material>
	ColladaYY::const_bind_material::technique_common::instance_material in) 
	{
		RT::Material_Instance im;
		ColladaYY::const_material yy = 
		xs::anyURI(in->target).get<ColladaYY::material>();
		im.Material = dbase->LoadMaterial(yy);
		if(im.Material==nullptr) return;
		
		RT::Material &m = *im.Material;
		if(geom->Missing_Normals) //HACK
		if(!m.Diffuse.IsBlack()||!m.Specular.IsBlack()
		||m.FX!=nullptr&&m.FX->FindTechnique()->Generate.NORMAL)
		{
			geom->Generate_Normals();
		}
		im.Symbol = in->symbol; imv.push_back(im);

		//Adding this to help debug demo.dae.
		for(size_t i=0;i<geom->Elements.size();i++)			
		if(im.Symbol==geom->Elements[i].Material_Instance_Symbol)
		return;
		daeEH::Warning<<"Did not find material symbol "<<im.Symbol<<" in geometry "<<geom->Id;
	}
};
struct RT::DBase::LoadTransforms_of
{
	RT::DBase *dbase; RT::Node *out;
	LoadTransforms_of(RT::DBase *db, RT::Node *out, ColladaYY::const_node in)
	:dbase(db),out(out)
	{
		//This preserves the transforms' order of appearance and application.
		in->content->for_each_child(*this);

		//SCHEDULED FOR REMOVAL (EXPONENTIAL LOOK UP)
		//check if there are animation on the transform
		//REMINDER: ANIMATIONS SHOULD HAVE BEEN PRE-LOADED.
		RT::Animator a; size_t &j = a.Channel;		
		for(size_t i=0;i<dbase->Animations.size();i++)					
		for(a.Animation=dbase->Animations[i],j=0;j<a.Animation->Channels.size();j++)
		{
			RT::Target &t = a.Animation->Channels[j].Target;
			if(dae(in)!=&dae(t.Fragment)->getParentObject()) 
			continue;		
			//This is mapping the animations directly to the
			//packed transform data so the animation code can
			//ignore the transforms.
			short tf = 0;
			for(size_t i=0;i<out->Transforms.size();i++)			
			if(out->Transforms[i].Fragment==t.Fragment)
			{
				t.Min+=tf; 
				t.Max+=tf;
				out->Animators.push_back(a); break;
			}
			else tf+=out->Transforms[i].Size;
		}
	}
	void operator()(const xs::const_any &e)
	{
		switch(e->getElementType()) //ORDER-IS-IMPORTANT
		{
		#define _(x) \
		case DAEP::Schematic<Collada05::x>::genus:\
			out->Transforms.push_back\
			(RT::Transform_##x(e->a<Collada05::const_##x>()));\
			break;\
		case DAEP::Schematic<Collada08::x>::genus:\
			out->Transforms.push_back\
			(RT::Transform_##x(e->a<Collada08::const_##x>()));\
			break;
		_(rotate)_(translate)_(scale)_(lookat)_(skew)_(matrix)
		#undef _
		}			
	}
};
RT::Node *RT::DBase::LoadNode(ColladaYY::const_node &in, int i, RT::Node *node)
{
	if(in==nullptr) return nullptr;

	//REMINDER: GetNode is broken insofar as it is not document aware.
	RT::Node *out = nullptr;
	if(!in->id->empty())
	{
		out = GetNode(in); if(out!=nullptr)
		{
			//2017: A <skeleton> reference can cause a <node> to be loaded
			//before its root node will be. The <skeleton> may also come from
			//an outside document.
			if(node!=nullptr)
			node->Nodes.push_back(out);
			return out;
		}
	}

	RT::Main_Asset _RAII(in->asset);
	//If <instance_node> recursively retrieve
	//the <asset> <up_axis> and <unit> values.
	if(-1!=i) RT::Asset = in;	

	daeEH::Verbose<<"Loading Scene Node "<<in->id;	

	out = COLLADA_RT_new(RT::Node);	
	out->Name = in->name;
	out->Id = in->id;
	out->DocURI = RT::DocURI(in); 
	out->Sid = in->sid;	
	//SCHEDULED FOR REMOVAL
	out->Fragment = in;
	LoadTransforms_of(this,out,in);
	LoadInstances_of(this,out,in);

	//add to parent 
	if(node!=nullptr)
	node->Nodes.push_back(out);

	//read children 
	ColladaYY::const_node yy;
	for(size_t i=0;i<in->node.size();i++)
	{
		yy = in->node[i]; LoadNode(yy,-1,out);	
	}
	xs::anyURI URI; 
	for(size_t i=0;i<in->instance_node.size();i++)
	{
		URI = in->instance_node[i]->url;			
		yy = URI.get<ColladaYY::node>();
		LoadNode(yy,(int)i,out);	
	}

	Nodes.push_back(out); return out;
}

void RT::DBase::LoadCOLLADA(ColladaYY::const_COLLADA &COLLADA)
{	
	RT::Asset.Meter = COLLADA->asset->unit->meter->*RT::Float(1);
	RT::Asset.Up = COLLADA->asset->up_axis->value->*RT::Up::Y_UP;
	switch(RT::Asset.Up)
	{
	case RT::Up::X_UP: 				
	daeEH::Verbose<<"X"<<"-axis is Up axis!"; break;
	case RT::Up::Y_UP:
	daeEH::Verbose<<"Y"<<"-axis is Up axis!"; break;
	case RT::Up::Z_UP:
	daeEH::Verbose<<"Z"<<"-axis is Up axis!"; break;
	}			

	//SCHEDULED FOR REMOVAL?
	if(RT::Main.LoadAnimations)
	for(size_t i=0;i<COLLADA->library_animations.size();i++)
	{
		ColladaYY::const_library_animations l = COLLADA->library_animations[i];
		for(size_t i=0;i<l->animation.size();i++) 
		{
			ColladaYY::const_animation yy = l->animation[i]; LoadAnimation(yy);
		}
	}

	//This had loaded the <instance_visual_scene> but COLLADA only 
	//allows for one such scene that is also associated with physics
	//and so on, so this just loads the main scene. The end-user would
	ColladaYY::const_scene yy = COLLADA->scene; LoadScene(yy);

	//SCHEDULED FOR REMOVAL
	//HACK: This is setting up FX::Profile_COMMON.	
	//COURTESY: Don't initialize VBuffer1. (In case not rendering.)
	//RT::Main.Stack.Draw();
	FX::Profile_COMMON::Color_or_Texture::FinalizeBWTextureColors();
	//
	for(size_t i=0;i<Materials.size();i++)
	{
		RT::Material *m = Materials[i];
		if(m->FX==nullptr) continue;
	
		//Try to extract detailed profile_COMMON information
		//for the fixed-function mode (no-FX) or even just a
		//fallback texture if profile_COMMON isn't specified.
		m->FX->Apply();
		//
		if(m->FX->FindTechnique()->IsProfile_COMMON())
		{
			//This is simply for data visualization purposes.
			GLuint mono = 0;
			FX::Profile_COMMON.Diffuse.Mono(m->Diffuse,mono);
			FX::Profile_COMMON.Emission.Mono(m->Emission,mono);
			FX::Profile_COMMON.Transparent.Mono(m->Transparent,mono);
			FX::Profile_COMMON.Ambient.Mono(m->Ambient,mono);			
			FX::Profile_COMMON.Specular.Mono(m->Specular,mono);
			if(mono!=0) m->Mono_TexId = mono;
			m->Shininess = FX::Profile_COMMON.Shininess;
			m->Transparency = FX::Profile_COMMON.Transparency;						
		}
		else //TODO? Look for DIFFUSE semantics, etc. 
		{
			FX::Pass *p = m->FX->FindTechnique()->Passes[0];
			for(size_t i=0;i<p->ShaderParams.size();i++)				
			if(p->ShaderParams[i].Type==GL_TEXTURE0)
			{
				FX::DataSampler2D *s = 
				p->ShaderParams[i]->As<FX::Sampler>();
				if(s!=nullptr)
				m->Mono_TexId = s->Value.FindTexID(m->Mono_TexId);
				break;
			}			
		}		
	}
}

//-------.
	}//<-'
}

/*C1071*/
