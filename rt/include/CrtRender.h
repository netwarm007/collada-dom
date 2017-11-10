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
#include "CrtEffect.h"
#include "CrtCamera.h"
#include "CrtPhysics.h"
#include "CrtTexture.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'

/**
 * Clients can use this to get a running system time.
 * It supports Windows, Linux and PlayStation 3 time.
 */
RT::Float Time();

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
struct Stack_Draw : RT::Effect_Semantics
,
std::pair<RT::Geometry*,RT::Controller*>
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

	/**COURTESY	Complements @c RT::Stack::Draw_Instances().
	 */
	inline RT::Material *FindMaterial(xs::string symbol)
	{
		std::vector<RT::Material_Instance> &mats = GetMaterials();

		for(size_t i=0;i<mats.size();i++)
		if(mats[i].Symbol==symbol) return mats[i].Material; return nullptr;
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

	//This is some BS just to avoid searching.
	//Maybe incorporate cameras and lights too?
	std::vector<int> ShowHierarchy_Splines;

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
		}		
		Select_AddData_Controllers_and_finish_up();
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
	
COLLADA_(public)

	RT::Stack_Draw *CurrentDraw;

	RT::Material *CurrentMaterial;	

	static RT::Material DefaultMaterial;
	
	void SetMaterial(RT::Material *mat);

	void ResetMaterial(RT::Material *mat=&DefaultMaterial);

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
	 * @c Draw() calls @c Draw_Instances() internally.
	 * It first sets up lights and may do more passes.
	 */
	void Draw();

COLLADA_(public) //CUSTOM "DRAWING" FACILITIES

	//WARNING: This is courtesy to applications which
	//would like to use this code to render to a file.
	//To be safe, don't mix with RT::Frame::Refresh().

	template<class T>
	/**COURTESY
	 * @c Draw_Triangles() is implemented in terms of
	 * this logic. Users can use it to draw to memory.
	 * @c VBuffer2 MUST BE INITIALIZED if controllers
	 * are present. @c VBuffer1 may be 0 capacity, as
	 * OpenGL's draw arrays is probably not desirable.
	 * @see @c Init_VBuffers().
	 */
	inline void Draw_Instances(void callback(T*), T *context=nullptr)
	{
		return _Draw_Instances((void(*)(void*))callback,(void*)context);
	}
	void _Draw_Instances(void(*)(void*),void*_=nullptr);

	/**COURTESY, EXPERIMENTAL
	 * @c Draw_Instances() requires @c VBuffer2 to be
	 * able to fit the largest controller source data.
	 * @a which can be 1 or 2 to restrict to vbuffers
	 * 1 or 2.
	 */
	void Init_VBuffers(int which=0);

COLLADA_(public) //UTILITIES

	/** 
	 * Sets up the camera instance to be rendered from.
	 */
	void _ResetCamera();

	inline RT::Stack_Data *FindData(RT::Node *p)
	{
		for(size_t i=0;i<Data.size();i++)
		if(p==Data[i].Node) return &Data[i]; return nullptr;
	}
};

/**
 * Getting this out of @c RT::Frame ("CrtRender.:)
 * @see CrtCommongCg.cpp
 */
class Frame_FX : public FX::Loader
{
COLLADA_(public)

	bool Use, Initialized; 
		
	/**
	 * Updates global FX parameters except VIEW and VIEW_INVERSE
	 * which are used in the camera set up.
	 */
	void Reset_Context();

	/**
	 * Updates the WORLD family of parameters unless they are in
	 * the argument list in the case of the WORLD and WORLD_VIEW.
	 */
	bool SetWorld(RT::Effect_Semantics,RT::Matrix&,RT::Matrix&);

COLLADA_(private) //old "CrtRender" APIs

	friend class RT::Frame;
	friend class RT::Stack;
	void (*RestoreGL)();
	void Init(),Reset(),Destroy(bool=false);

	Frame_FX(){ RT::MatrixLoadIdentity(IDENTITY.Value); }

COLLADA_(public) //FX::Loader API.
	 
	enum{ LIGHT_MAX=16 };
	struct World_or_View
	{
		FX::DataFloat4 World,View;
	};
	FX::DataType<RT::Matrix> IDENTITY;	
	World_or_View LIGHT_POSITION[LIGHT_MAX];
	FX::DataType<RT::Matrix> PROJECTION;	
	World_or_View SPOT_DIRECTION[LIGHT_MAX];
	FX::DataType<RT::Float> TIME;
	FX::DataType<RT::Matrix> VIEW;
	FX::DataType<RT::Matrix> VIEW_INVERSE;
	FX::DataType<RT::Matrix> WORLD;
	FX::DataType<RT::Matrix> WORLD_INVERSE_TRANSPOSE;
	FX::DataType<RT::Matrix> WORLD_VIEW;
	FX::DataType<RT::Matrix> WORLD_VIEW_INVERSE_TRANSPOSE;
	FX::DataType<RT::Matrix> WORLD_VIEW_PROJECTION;
	template<int MAX>
	void Load_Slot(World_or_View (&v)[MAX], FX::NewParam *p)
	{
		World_or_View *el = v+p->Subscript; 
		if(el<v+MAX)
		p->ClientData = &(p->Space_IsView()?el->View:el->World);
	}
	virtual void Load_ClientData(FX::NewParam *p)
	{
		switch(p->Semantic)
		{
		#define _(x) case FX::x: p->ClientData = &x; break;		
		case FX::LIGHT_POSITION: Load_Slot(LIGHT_POSITION,p); 
		break;
		_(PROJECTION)
		_(TIME)
		case FX::SPOT_DIRECTION: Load_Slot(SPOT_DIRECTION,p);
		break;		
		_(VIEW)
		_(VIEW_INVERSE)
		_(WORLD)
		_(WORLD_INVERSE_TRANSPOSE)
		_(WORLD_VIEW)
		_(WORLD_VIEW_INVERSE_TRANSPOSE)
		_(WORLD_VIEW_PROJECTION)		
		#undef _
		default:; //-Wswitch
		}
		_Loading_Connect.Set(p->Semantic);
	}
	void Load_Connect(RT::Effect *mint)
	{
		std::swap(mint->Semantics(),_Loading_Connect.Semantics());
	}
	RT::Effect_Semantics _Loading_Connect;
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

	Camera_State():Parent(),Camera(),Pan(),Tilt(),X(),Y(),Z(),Zoom()
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
	int Left,Top,Width,Height;	

	//bool UseCg;	
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
	bool ShowCOLLADA_FX;
	int ShowTextures_Mask;	

	//Animation Controls 
	bool AnimationOn;
	bool AnimationPaused;
	bool AnimationLinear;
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
		_operator_YY(dae(cp),typename DAEP::Schematic<E>::schema());
	}
	//HACK: __NB__ is DAEP::Schematic<ColladaYY::COLLADA>::schema.
	void _operator_YY(const daeElement*,Collada05_XSD::__NB__ schema);
	void _operator_YY(const daeElement*,Collada08_XSD::__NB__ schema);
	#ifdef PRECOMPILING_COLLADA_RT //asset is incomplete.
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
	const RT::DBase *const DB; 
	/**
	 * This will be resolved, and it depends on the document.
	 */
	const RT::Name URL; const daeDOM DOM; 
	
	/**
	 * This is the URL's document or "doc" object.
	 */
	const const_daeDocRef Index;

	/**
	 * This is a makeshift way of recording samples/ options.
	 */
	std::vector<daeName> COLLADA_index_keywords;

	/**INTERNAL
	 * @c Stack houses the low-level rendering, etc. elements. 
	 */
	RT::Stack Stack; 	

	/**INTERNALS
	 * @c Asset changes inside subroutines. 
	 * @c Scene is a pseudo node at the root of the selection.
	 */
	RT::Frame_Asset Asset; RT::Node Scene; 
	
	/**INTERNAL
	 * The PlayStation 3 viewer has some code that plays with
	 * gravity.
	 * @see @c SetGravity().
	 */
	RT::Physics Physics;

	#ifdef NDEBUG
	#error Implement RT::Frame_State logic.
	#endif
	/**
	 * This checks the RT::Frame_State members before drawing
	 * each render-frame.
	 */
	bool Refresh();

	/**
	 * Set up @c FX.platform_Filter to target
	 * only specific FX platforms. The default is "PC"
	 * but the FX component prior to 2017 used PC-OGL.
	 * Post-2017 if unset all platform strings are go.
	 */
	RT::Frame_FX FX; RT::Image Missing_Image;

	//SCHEDULED FOR REMOVAL
	static GLuint Missing_Image_TexId();
		
	//SCHEDULED FOR REMOVAL
	void Init(void RestoreGL());
	void _InitDB(),_InitMembers(),_Destroy();	
	/**
	 * Singleton Constructor
	 * It will be a project to support multiple objects
	 * of this type.	 
	 * 
	 * @param OK is ensuring the order of intialization
	 * doesn't matter. Ignore it.
	 */
	Frame(daeOK=COLLADA::DOM_process_share.grant())
	:DB(),URL(""),Missing_Image("default.tga"),Loading()
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
	/**
	 * Negative values get closer. Kind of confusing.
	 */
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
