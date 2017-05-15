/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__RENDER_H__
#define __COLLADA_RT__RENDER_H__  
		 
#include "CrtNode.h"
#include "CrtCamera.h"
#include "CrtPhysics.h"
#include "CrtTexture.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
   
/**
 * Previously "CrtOrient/CrtNode."
 * This class implements complex instancing of <node>.
 * @see @c RT::Stack.
 */
class Stack_Data
{
COLLADA_(public)
	/**
	 * @c Node is this stack-frame's shared data.
	 *
	 * This could be removed, but it seems as if
	 * it's a useful bit of extra information to
	 * have alongside @c Physics.
	 */
	RT::Node *Node;

	/**
	 * This is used to remove multi-SID Physics
	 * binding candidates. It can be jettisoned
	 * after if passed to @c Update() or if the
	 * root-node is a dummy.
	 */
	RT::Stack_Data *Parent;	

	/**OPAQUE, PRIVATE-DATA
	 * This replaces an old "UpdatingMatrix" flag
	 * while also housing private simulation data.
	 *
	 * @todo
	 * Eventually <instance_physics_model parent>
	 * will have to be represented by this member.
	 */
	btRigidBody *Physics;

COLLADA_(public) 
	/**
	 * Fully animated world matrix for this node.
	 */	
	RT::Matrix Matrix;

COLLADA_(public) //RT::Stack usese these.
	
	Stack_Data():Physics(){}

	int Light(int); void ShowHierarchy_glVertex3d();	

	RT::Float *Update_Matrix(RT::Float*);
};
/**INTERNAL
 * These are sorted according to <geometry> and <controller>.
 * When the <skeleton> of a <skin> is equal it's passed over.
 * EVENTUALLY SORTING SHOULD DO A THIRD CHECK TO ENSURE THIS.
 */
struct Stack_Draw : std::pair<RT::Geometry*,RT::Controller*>
{
	RT::Stack_Data *Data; void *Instance;

	inline RT::Geometry_Instance *AsGeometry()
	{
		return second!=nullptr?nullptr:(RT::Geometry_Instance*)Instance;
	}
	inline RT::Controller_Instance *AsController()
	{
		return second==nullptr?nullptr:(RT::Controller_Instance*)Instance;
	}

	inline std::vector<RT::Material_Instance> &GetMaterials()
	{
		//FYI: There aren't any branches here if Materials is at the same offset.
		return second==nullptr?AsGeometry()->Materials:AsController()->Materials;
	}

	#ifdef NDEBUG
	#error Is RT::Animator instance aware?
	#endif
	/**
	 * This meets the requirements from the 
	 * manual's example under <skeleton>. 
	 * Either all animated controllers will
	 * do this, or the animations will work
	 * on all controller instances in chaos
	 * for purpose of generic visualization.
	 *
	 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.XML}
	 *	<node id="skel01"> 
	 *	   <instance_node url="#Skeleton1"/> 
	 *	</node> 
	 *	<node id="skel02"> 
	 *	  <instance_node url="#Skeleton1"/> 
	 *	</node> 
	 *	<node> 
	 *	  <instance_controller url="#skin"> 
	 *		<skeleton>#skel01</skeleton> 
	 *	  </instance_controller> 
	 *	</node> 
	 *	<node> 
	 *	  <instance_controller url="#skin"> 
	 *		<skeleton>#skel02</skeleton> 
	 *	  </instance_controller> 
	 *	</node> 
	 */
	inline bool Advances_ControllerData(RT::Controller_Instance *ic)
	{
		if(ic==nullptr||second!=ic->Controller)
		return true;		
		RT::Controller_Instance *iic = AsController();
		if(ic->Skeletons!=iic->Skeletons) return true;
		for(size_t i=0;i<ic->Skeletons;ic++)
		if(ic->Joints[i]!=iic->Joints[i]) return true; return false;
	}
};

/**INTERNAL
 * Trying to get low-level render data out of 
 * @c RT::Frame ("CrtRender.")
 */
class Stack
{
COLLADA_(public) //new APIs
	/**
	 * This replaces the instanced/animated
	 * parts of the old "CrtNode" class. The
	 * elements correspond to nodes. It isn't
	 * necessary to do a hiearchical traversal.
	 */
	std::vector<RT::Stack_Data> Data;
	/**
	 * This holds the instanced/animated node
	 * "transform" values like <rotate> and is
	 * probably going to hold optional matrices
	 * either alongside it or in the back, since
	 * @c RT::Stack_Data doesn't always use every
	 * one of its matrices.
	 */
	std::vector<RT::Float> TransformData;

	/**EXPERIMENTAL
	 * This is sorted according to geometry
	 * instances, also including controllers.
	 */
	std::vector<RT::Stack_Draw> DrawData;
	/**
	 * This is pointers into @c Data for each
	 * <instance_controller> is the @c DrawData.
	 * The pointers are in blocks of bound joints.
	 */
	std::vector<RT::Stack_Data*> ControllerData;

	Stack();
	/**INTERNAL
	 * This rebuilds the "stack." The stack-model
	 * implements the many-level instancing model.
	 */
	void Select(RT::Node *node=nullptr)
	{
		//This is in the header to illustrate
		//how the subroutines work.
		Data.resize(1); ControllerData.clear();
		VBuffer1.Clear();		
		TransformData.clear(); DrawData.clear();
		if(node!=nullptr)
		{
			Data.reserve(1+node->CountDescendants());	
			Select_AddData(node,nullptr,-1);
			std::sort(DrawData.begin(),DrawData.end());
			Select_AddData_Controllers_and_finish_up();
		}		
	}private:
	bool Select_AddData(RT::Node*,RT::Stack_Data*,size_t);
	void Select_AddData_DrawData(void*,void*,RT::Stack_Data*);
	void Select_AddData_Controllers_and_finish_up();	

COLLADA_(public) //EXPERIMENTAL
	/**
	 * These buffers may be juggled with @c std::swap. The
	 * buffers are both used if interpolating <controller>
	 * data. The memory may be alternatively used via swap.
	 */
	struct VBuffer
	{			  
		size_t Capacity; 
		/**
		 * @c Memory is for direct memory access.
		 * @c new_Memory is system memory.
		 */
		union{ float *new_Memory,*Memory; };

		VBuffer():Capacity(),Memory(){}
		~VBuffer(){ Clear(); }

		void Clear()
		{
			Capacity = 0; 
			COLLADA_RT_array_delete(new_Memory);
			assert(Memory==nullptr);
		}
		
	}VBuffer1,VBuffer2;

COLLADA_(public) //old "CrtRender" APIs

	//OBSOLETE
	RT::Material *CurrentMaterial;	
				
	void SetMaterial(RT::Material *mat);
	
	//SCHEDULED FOR REMOVAL
	//This is keeping old/lousy code from breaking.
	RT::Stack_Data &FindAnyLight(),*_FoundAnyLight;

COLLADA_(public) 
	/**
	 * This version of @c Update() is for starting
	 * physical simulations and animations without
	 * any residual effects. It's likely essential 
	 * for Physics; because the initial states are
	 * just left to chance if not used.
	 */
	void Reset_Update(){ Update(true); }
	/**
	 * This refreshes the world transform matrices.
	 * @see @c Reset_Update().
	 */
	void Update(bool=false);
	
	void Draw_ShowHierarchy(); 
	/**
	 * @c Draw() calls @c Draw_Triangles() internally.
	 * It first sets up lights and may do more passes.
	 */
	void Draw(),Draw_Triangles();
	
	/** 
	 * Sets up the camera instance to be rendered from.
	 */
	void _ResetCamera(); RT::Matrix _View;
};

/**UNUSED?
 * Getting this out of @c RT::Frame ("CrtRender.")
 */
struct Frame_ShadowMap
{	
	GLuint Id; int Width, Height;

	bool Use, RenderingTo, RenderingWith, Initialized;	

	Frame_ShadowMap():Width(512),Height(512),Id()
	,Use(),RenderingTo(),RenderingWith(),Initialized()
	{}

	void Init();
	void PushRenderingToShadowMap();
	void PopRenderingToShadowMap();	
};

/**
 * Getting this out of @c RT::Frame ("CrtRender.:)
 * @see CrtCommongCg.cpp
 */
struct Frame_Cg
{
	bool Use, Initialized; 
	
	CGcontext Context;

COLLADA_(public)
	/**NEW
	 * This is taken from "CrtGeometry.cpp" and so named
	 * because "fc->resetEffectPassState()" must be done
	 * after drawing/switching materials.
	 */
	void SetPassState(FX::Material*,int);

COLLADA_(public) //old "CrtRender" APIs

	void Init();
	void Reset();
	void Destroy(bool=false);
	//UNUSED //UNUSED //UNUSED //UNUSED
	void EnableProfiles(),DisableProfiles(); 
	
	//The rest is mostly clutter.

  #ifdef NDEBUG
  #error Are the "Set" APIs client APIs?
  #endif

  //UNUSED //UNUSED //UNUSED //UNUSED //UNUSED

	bool LoadProgram(int&,const char *fileName, CGprofile);

	int SkinDefaultProgramId;
	int StaticDefaultProgramId;
	int PhongFragmentProgramId;		

	int SkinShadowProgramId;
	int StaticShadowProgramId;
	int PhongFragmentShadowProgramId;
									
	int StaticNormalMapId;
	int SkinNormalMapId; //UNIMPLEMENTED
	int FragmentNormalMapId; 
	
	bool SetDefaultStaticProgram();
	bool SetDefaultSkinProgram();		
	bool SetDefaultFragmentProgram()
	{
		SetPhongFragmentProgram();
	}
	bool SetPhongFragmentProgram();
	
	bool SetShadowMapStaticProgram();
	bool SetShadowMapSkinProgram();
	bool SetShadowMapFragmentProgram();
	
	bool SetNormalMapStaticProgram();
	bool SetNormalMapSkinProgram();
	bool SetNormalMapFragmentProgram();

  //NO IDEA //NO IDEA //NO IDEA //NO IDEA //NO IDEA 
	
	//SCHEDULED FOR REMOVAL
	RT::Matrix *_WorldMatrix,_InverseViewMatrix;
};

/**
 * Previously "InstanceCamera."
 */
class Camera_State
{
COLLADA_(public)
	/**
	 * Node where this instance was instantiated.
	 */
	RT::Stack_Data *Parent;
	/**
	 * The abstract camera where the cam parameters are stored.
	 */
	RT::Camera *Camera;
	/**UNDOCUMENTED	 
	 */
	RT::Float Pan,Tilt,X,Y,Z,Zoom; 

COLLADA_(public)

	Camera_State():Parent(),Camera(),X(),Y(),Z(),Pan(),Tilt(),Zoom()
	{}

COLLADA_(public) //See CrtRender.cpp.

	//These are not inline to make it easier to experimenting
	//with changing them without dirtying precompiled headers.

	void Center();
	void Rotate(RT::Float,RT::Float);
	void Fly(RT::Float,RT::Float,RT::Float=0);
	void Walk(RT::Float,RT::Float,RT::Float);
	
	/**LEGACY-SUPPORT
	 * @param view can be @c nullptr if @a inverseview is desired.
	 * Traditionally the "view" is inverted, so its inverse is in
	 * fact a "world" matrix, not inverted in the typical meaning.
	 *
	 * @note This had been a data member, but this struct is best
	 * kept lightweight so it can be used to save cameras' states.
	 */
	void Matrix(RT::Matrix *view, RT::Matrix *inverseview=nullptr)const;
};
/**
 * To limit Get/SetBlah() APIs the idea is to check if
 * these have changed whenever @c Refresh() is invoked.
 */
struct Frame_State : RT::Camera_State
{	
	int Width,Height;	

	//Add these two.
	//bool UseCg;
	//bool UseShadowMap;
	bool UseVBOs;	
	bool UseRender;
	bool UsePhysics;
		
	//Requires Reload
	bool LoadAnimations;
	bool LoadImages;
	bool LoadEffects;
	bool LoadGeometry;
	bool ShowGeometry;	
	bool ShowHierarchy;
	int ShowTextures_Mask;
	

	//Animation Controls 
	bool AnimationOn;
	bool AnimationPaused;
	RT::Float Time,Delta;
};

/**
 * This tracks the <up_axis> and <unit> states recursively.
 */
struct Frame_Asset
{
	/**
	 * Until there is a class for representing this, these
	 * may be temporarily overriden by the loading process.
	 * @see @c RT::Main_Asset
	 */
	RT::Up Up; RT::Float Meter;
	/**
	 * For animation runtime. (Just extra asset-wide data.)
	 */
	RT::Float TimeMin,TimeMax;
		
	/** 
	 * @c SetGravity() uses this.
	 */
	void Mult(RT::Float &x, RT::Float &y, RT::Float &z)	
	{	
		x*=Meter; y*=Meter; z*=Meter;
		//   0,M01,  0 
		//-M10,  0,  0 
		//   0,  0,M22
		if(RT::Up::X_UP==Up) std::swap(y*=-1,x);
		//M00,  0,   0
		//  0,  0,-M12
		//  0,M21,   0
		if(RT::Up::Z_UP==Up) std::swap(y*=-1,z);
	}

	template<class E>
	/** 
	 * Assign @c Up and @c Meter according to 
	 * an <instance_*> reference.
	 */
	inline void operator=(const E &cp)
	{
		operator=(cp.operator->());
	}	
	template<class E>
	/** 
	 * Assign @c Up and @c Meter according to 
	 * an <instance_*> reference.
	 */
	inline void operator=(const E *cp)
	{
		_operator_YY(dae(cp),DAEP::Schematic<E>::schema()); 
	}
	#ifdef PRECOMPILING_COLLADA_RT
	inline void operator=(const Collada05_XSD::asset *cp)
	{
		Up = cp->up_axis->value->*Up;
		Meter = cp->unit->meter->*Meter;
	}
	inline void operator=(const Collada08_XSD::asset_type *cp)
	{
		Up = cp->up_axis->value->*Up;
		Meter = cp->unit->meter->*Meter;
	}	
	void _operator_YY(const daeElement*,Collada05_XSD::__NB__);
	void _operator_YY(const daeElement*,Collada08_XSD::__NB__);	
	#endif			 	 
	template<class T>
	/** 
	 * Implements @c operator=().
	 */
	void _OverrideWith(const daeElement*);
};

/**SINGLETON
 * @see @c RT::Main.
 * Previously "CrtRender."
 * @c RT::Main is this type of object. Various parts of the
 * the package assume their @c RT::Frame is @c RT::Main. It
 * is a bad idea to pass @c RT::Main around, since that can
 * create the appearance otherwise, and will make a rewrite
 * less straightforward.
 * @remark "Frame" is like an <iframe> or a picture's frame.
 * This is a humble user interface for the whole RT package.
 */
class Frame : public RT::Frame_State
{
COLLADA_(public)
	/**
	 * This had been "CrtScene *Scene;" 
	 * It should be a daeDB<RT::DBase>.
	 */
	const RT::DBase *const Data; 
	/**
	 * This will be resolved, and it depends on the document.
	 */
	const daeName URL; const daeDOM DOM;

	/**INTERNAL
	 * @c Stack houses the low-level rendering, etc. elements. 
	 */
	RT::Stack Stack; 	

	/**INTERNAL
	 */
	RT::Frame_Asset Asset; RT::Node Scene; RT::Physics Physics;

	#ifdef NDEBUG
	#error Implement RT::Frame_State logic.
	#endif
	/**
	 * This checks the RT::Frame_State members before drawing
	 * each render-frame.
	 */
	bool Refresh();

	/**UNUSED?
	 */
	RT::Frame_Cg Cg; RT::Frame_ShadowMap ShadowMap;	

	/**
	 * Set up @c COLLADA_FX.platform_Filter to target
	 * only specific FX platforms. The default is "PC"
	 * but the FX component prior to 2017 used PC-OGL.
	 * Post-2017 if unset all platform strings are go.
	 */
	FX::Loader COLLADA_FX; RT::Image Missing_Image;
		
	//SCHEDULED FOR REMOVAL
	void Init(),_InitData(),_InitMembers(),_Destroy();	
	/**
	 * Singleton Constructor
	 * It will be a project to support multiple objects
	 * of this type.
	 */
	Frame():Loading(),Data(),URL(""),Missing_Image("default.tga")
	{
		assert(this==&RT::Main); _InitMembers(); 
	}
	~Frame(){ _Destroy(); }
	
	const bool Loading;
	/**
	 * @param index_DAE displaces the contents of @c DOM.
	 * @c Load automatically selects the <scene> element
	 * of a COLLADA document.
	 * @c Loading is set @c true until @c Load completes.
	 * @c URL remembers @c index_DAE's resolved URI data.
	 */
	bool Load(const xs::anyURI &index_DAE);	

	void Unload();

	inline bool Reload(){ return Load(URL); }
		
COLLADA_(public) //old camera management APIs
	/**
	 * These change @c Camera. The cameras loop 
	 * back to the default camera, and so can be
	 * enumerated and managed this way. The state
	 * is shared. The client must keep track of it.
	 */
	void SetDefaultCamera(),SetNextCamera();
		
	//USED
	#ifdef NDEBUG
	#error Merge this with Camera_State.
	#endif
	RT::RangeFinder SetRange;
	inline void ZoomIn(RT::Float zoom)
	{
		Zoom+=SetRange.Zoom*0.2f*zoom;
	}	
};
//SHORTHAND
static RT::Frame_Asset &Asset = RT::Main.Asset;
//SCHEDULED FOR REMOVAL
/**RAII
 * This class is used to override the <asset> 
 * up/scale. It's not ideal, but it's alright.
 */
struct Main_Asset
{
	const RT::Up Up;
	const RT::Float Meter;	
	Main_Asset(RT::Up up, RT::Float meter)
	:Up(RT::Asset.Up),Meter(RT::Asset.Meter)
	{
		RT::Asset.Up = up; RT::Asset.Meter = meter;
	}
	template<class E>
	Main_Asset(const E &cp)
	:Up(RT::Asset.Up),Meter(RT::Asset.Meter)
	{
		RT::Asset = cp.operator->();
	}	
	Main_Asset(const RT::Asset_Index &cp)
	:Up(RT::Asset.Up),Meter(RT::Asset.Meter)
	{
		RT::Asset.Up = RT::GetUp_Meter(cp).first;
		RT::Asset.Meter = RT::GetUp_Meter(cp).second;
	}
	~Main_Asset()
	{
		RT::Asset.Up = Up; RT::Asset.Meter = Meter;
	}
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__RENDER_H__
/*C1071*/