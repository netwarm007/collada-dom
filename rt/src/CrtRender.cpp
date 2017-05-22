/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <RT.pch.h> //PCH

#include "CrtScene.h"
#include "CrtLight.h"
#include "CrtGeometry.h"
#include "CrtAnimation.h"
#include "CrtRender.h"

COLLADA_(namespace)
{
	//See cfxPCH.h
	struct GL GL; //extern
	//See cfxLoader.h
	//SCHEDULED FOR REMOVAL
	GLuint FX::Loader::GetID_TexId(Collada05::const_image image)
	{
		if(image!=nullptr)
		{
			int out = RT::Main.Data->FindImage(image)->TexId;
			if(out!=0) return out;
		}		
		static bool nonce = false; if(!nonce)
		{
			nonce = true;
			RT::Main.Missing_Image.Refresh();
			daeEH::Warning<<(0!=RT::Main.Missing_Image.TexId
			?"Using missing image texture: "
			:"Missing image image is missing: ")<<RT::Main.Missing_Image.URL;
		}
		return RT::Main.Missing_Image.TexId;
	}
	GLuint FX::Loader::GetID_TexId(Collada08::const_image image)
	{
		//This only has to pass a viable element pointer.
		return GetID_TexId(COLLADA_RT_cast(image,image));
	}
	RT::Image *RT::DBase::FindImage(Collada05::const_image &e)const
	{
		assert(RT::Main.Loading); //Delayed loading?
		return const_cast<RT::DBase*>(this)->LoadImage(e);
	}
	RT::Image *RT::DBase::FindImage(Collada08::const_image &e)const
	{
		assert(RT::Main.Loading); //Delayed loading?
		return const_cast<RT::DBase*>(this)->LoadImage(e);
	}

	namespace RT
	{//-.
//<-----'


//SCHEDULED FOR REMOVAL
extern std::vector<char> CrtTexture_buffer;

//!!!GAC temporary WINDOWS ONLY for performance timing
#ifdef _WIN32 
LARGE_INTEGER update_time,render_time;
#endif

//#ifdef SN_TARGET_PS3
//#include <sys/raw_spu.h>
//#include <sys/sys_time.h>
//#elif !defined(_WIN32)
//#include <sys/time.h>
//#endif
static RT::Float CrtRender_timer()
{
	#ifdef SN_TARGET_PS3
	{
		system_time_t t = sys_time_get_system_time();
		return t*RT::Float(0.000001);
	}
	#elif defined(_WIN32)
	{
		return timeGetTime()*RT::Float(0.001);
	}
	#else
	{
		struct timeval LTV;
		struct timezone LTZ;

		if(gettimeofday(&LTV,&LTZ)!= -1)
		{
			long long LTime = LTV.tv_sec;

			LTime*=1000000;
			LTime+=LTV.tv_usec;
			unsigned long time = LTime&0xFFFFFFFF;
			return (RT::Float)time/1000000;
		}
	}
	#endif
	return 0;
}

static void CrtRender_UpdateDelta()
{	
	//this function should be called once per frame 	
	static RT::Float fps = 0;
	static RT::Float time = 0;
	static RT::Float oldTime = CrtRender_timer();
	static int nbrFrames = 0;
	static bool UpdatedOnce = false;
	const RT::Float updateTime = 1;

	time = CrtRender_timer();

	nbrFrames++;

	if(time>(oldTime+updateTime))
	{
		fps = (RT::Float)nbrFrames;
		RT::Main.Delta = 1.0f/(fps)*updateTime;
		//If Delta is running less than 15 fps, stop animation and wait till fps come back to above 15 fps
		//if(RT::Main.Delta>0.04f) 
		//RT::Main.Delta = 0; 

		oldTime = time;
		nbrFrames = 0;

		UpdatedOnce = true;
		#if 0 
		daeEH::Verbose<<"FPS is "<<fps<<", Time is "<<time;
		#endif
		#ifdef _WIN32  //!!!GAC temporary windows only performance timing code
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		//daeEH::Verbose<<"update "<<(double)update_time.QuadPart/(double)frequency.QuadPart<<
		//", render "<<(double)render_time.QuadPart/(double)frequency.QuadPart<<
		//", frequency "<<frequency.QuadPart;
		#endif
	}

	//until first update delta = 0;
	if(!UpdatedOnce) RT::Main.Delta = 0;
}

//This is a table of <up_axis> & <unit> value pairs.
static std::vector<RT::Up_Meter> 
CrtRender_Up_Meter(1,std::make_pair(RT::Up::Y_UP,1.0f));
const RT::Up_Meter &RT::GetUp_Meter(size_t i)
{
	return CrtRender_Up_Meter[i];
}
RT::Asset_Index::Asset_Index()
{
	RT::Up_Meter cmp(RT::Asset.Up,RT::Asset.Meter);
	for(size_t i=0;i<CrtRender_Up_Meter.size();i++)	
	if(cmp==CrtRender_Up_Meter[i]) 
	{
		_i = i&0xFF; return;
	}	
	_i = 0xFF&CrtRender_Up_Meter.size();
	CrtRender_Up_Meter.push_back(cmp); 
}

void RT::Frame::Unload()
{
	Missing_Image.DeleteTexture();
	Physics.Clear(); Scene.Clear(); 	
	Stack.Select();
	delete Data; _InitData(); 
	const_cast<daeDOM&>(DOM).clear_of_content();
	Cg.Reset();
	UsePhysics = true;
}
void RT::Frame::_Destroy()
{
	Unload(); Cg.Destroy();

	#ifdef IL_VERSION
	ilShutDown();
	#endif

	DOM.~daeDOM(); //daeDOM_outstanding 
}

bool RT::Frame::Load(const xs::anyURI &URI)
{
	Unload(); assert(nullptr==Data);

	//Create a new scene and name it the same as the file being read.
	(void*&)Data = COLLADA_RT_new(RT::DBase);
	
	//in case of multithreaded loading 
	const_cast<bool&>(Loading) = true;

	//Setup the type of renderer and initialize Cg if necessary (we may need the context for loading)
	//!!!GAC this code used to come after the load, but now the load needs a Cg context.
	if(UseRender)
	{
		//try and initialize cg if we can as set the default shaders 
		if(Cg.Use) Cg.Init();

		if(ShadowMap.Use) ShadowMap.Init();
	}
	else Cg.Use = UseVBOs = ShadowMap.Use = false;

	daeEH::Verbose<<"COLLADA_DOM Load Started.";	

	daeDocRoot<> res = 
	const_cast<daeDOM&>(DOM).openDoc<void>(URI);
	if(res!=DAE_OK)
	{
		daeEH::Error<<
		"Failed to load document:\n"<<URI.getURI();
		Unload();
		daeEH::Error<<"Error loading the COLLADA file:"
		"\n"<<URI.getURI();			
		return const_cast<bool&>(Loading) = false;
	}
	else const_cast<daeName&>(URL) = res->getDocURI().getURI(); 

	daeEH::Verbose<<"COLLADA_DOM Runtime database initialized from:"
	"\n"<<URL;
	
	//SCHEDULED FOR REMOVAL
	daeElement *any = nullptr;
	daeDocument *doc = res->getDocument();
	if(doc!=nullptr) any = doc->getRoot();
	if(any==nullptr
	||!const_cast<RT::DBase*>(Data)->LoadCOLLADA_1_4_1(any->a<Collada05::COLLADA>())
	&&!const_cast<RT::DBase*>(Data)->LoadCOLLADA_1_5_0(any->a<Collada08::COLLADA>()))
	daeEH::Warning<<"Loaded document is not a COLLADA resource.";	
	
	CrtTexture_buffer.swap(std::vector<char>());
	
	//in case of multithreaded loading 
	const_cast<bool&>(Loading) = false;

	daeEH::Verbose<<"Done Loading "<<URL;	

	Time = RT::Asset.TimeMin; return true;
}
RT::Stack::Stack():CurrentMaterial()
{
	Data.resize(1); 
	Data[0].Parent = nullptr;
	Data[0].Node = &RT::Main.Scene;
	RT::MatrixLoadIdentity(Data[0].Matrix);
	RT::Main.SetDefaultCamera();
	Select(); //C++98/03 empty document fix.
}

bool RT::Frame::Refresh()
{
	if(Data==nullptr) return false;
		   
	//daeEH::Verbose<<"Rendering Data."; 

	//Update the scene, this traverses all the nodes and makes sure the transforms are up-to-date
	//It also sets up the hardware lights from the light instances in the scene.  (note the hardware
	//lights aren't used if you're rendering using Cg shaders.
	#ifdef _WIN32  //!!!GAC temporary windows only performance timing code
	LARGE_INTEGER temp_time;
	QueryPerformanceCounter(&temp_time);
	#endif
	{	
		bool reset = false;
		//set not to use the animation if 
		//animation is turned off 
		if(!AnimationOn) 
		{
			reset = Time!=-1; Time = -1;
		}
		else if(Time==-1) Time = Asset.TimeMin;

		for(size_t i=0;i<Data->Animations.size();i++)
		{
			RT::Animation *a = Data->Animations[i];
			if(Time>=a->TimeMin&&Time<=a->TimeMax+Delta)
			{
				a->NowPlaying = !AnimationPaused;
			}
			else a->NowPlaying = false;
		}	

		if(!AnimationPaused&&AnimationOn)
		{
			if(UsePhysics)
			Physics.Update(Delta);		
		}
		Stack.Update(reset);

		//too keep animation playback stable
		//no matter the frame rate 
		CrtRender_UpdateDelta();
		
		#ifdef NDEBUG
		#error This is old code, sorely in need of review.
		#endif
		//update animation if not paused 
		if(!AnimationPaused&&AnimationOn)
		{
			Time+=Delta;
			if(Time>RT::Asset.TimeMax)
			Time = RT::Asset.TimeMin+(Time-RT::Asset.TimeMax);
		}
	}
	#ifdef _WIN32  //!!!GAC temporary windows only performance timing code
	QueryPerformanceCounter(&update_time);
	update_time.QuadPart = update_time.QuadPart-temp_time.QuadPart;
	QueryPerformanceCounter(&temp_time);
	#endif

	if(UseRender) Stack.Draw();

	//!!!GAC temporary windows only performance timing code
	#ifdef _WIN32  
	QueryPerformanceCounter(&render_time);
	render_time.QuadPart = render_time.QuadPart-temp_time.QuadPart;
	#endif

	return true;
}

//!!!GAC This is a temporary fix to be removed by the refactoring, the contents of InitMembers and Init
//!!!GAC used to be all in one Init function that was called by the constructor AND by the mainlines
//!!!GAC of most of the samples.  This was causing Init to be called twice.  This wasn't a problem till
//!!!GAC we wanted to add Cg initialization which shouldn't be called in the global constructor and
//!!!GAC shouldn't be called twice.  To fix the problem I split the init function so the constructor
//!!!GAC now only initializes members and Init initializes members and Cg, this avoids the need to
//!!!GAC change every sample that calls Init.  I will clean this up later.
void RT::Frame::_InitData()
{
	const_cast<daeName&>(URL) = "";	
	const_cast<RT::DBase*&>(Data) = nullptr; 
}
void RT::Frame::_InitMembers()
{
	_InitData(); //2017

	Cg.Use = false;
	UseVBOs = false;
	UseRender = true;
	UsePhysics = true;
	  
	Cg.Initialized = false;
	Cg.Context = nullptr;

	Cg.SkinDefaultProgramId = -1;
	Cg.StaticDefaultProgramId = -1;
	Cg.PhongFragmentProgramId = -1;

	Cg.SkinShadowProgramId = -1;
	Cg.StaticShadowProgramId = -1;
	Cg.PhongFragmentShadowProgramId = -1;

	Cg.StaticNormalMapId = -1;
	Cg.SkinNormalMapId = -1;
	Cg.FragmentNormalMapId = -1;

	Width = 640;
	Height = 480;
	Delta = 0.03f;

	Asset.Up = RT::Up::Y_UP; Asset.Meter = 1;

	LoadAnimations = true;
	LoadImages = true;
	LoadEffects = true;
	LoadGeometry = true;
	ShowGeometry = true;
	ShowHierarchy = false;
	ShowTextures_Mask = -1;

//2017: Moved from old CrtScene
	
	//for animation runtime only
	Time = Asset.TimeMax = Asset.TimeMin = 0;

	//Animation Controls 
	AnimationOn = true;	AnimationPaused = false;

	//The viewer is turning this on by default.
	AnimationLinear = false;
}

void RT::Frame::Init()
{	
	_InitMembers();
			   
	#ifdef IL_VERSION
	ilInit();
	#endif

	//!!!GAC The new crt render path requires Cg to be initialized before anything is loaded
	Cg.Init(); 
	
	//FX needs this but cannot initialize it.
	//Reload the extensions one time just in case the DLLs are loaded in a funky order.
	new(&GL) struct COLLADA::GL;
	
	//SCHEDULED FOR REMOVAL
	//Unload is about to call Init regardless.
	//Physics.Init();
	//getGlobalPlatform() may be out of whack.
	DOM.~daeDOM(); new(const_cast<daeDOM*>(&DOM)) daeDOM;

	#ifdef SN_TARGET_PS3
	COLLADA_FX.platform_Filter.push_back("PS3");
	#endif

	daeEH::Verbose<<
	"------------------------------\n"
	"--- COLLADA-RT Initialized ---\n"
	"------------------------------";
}
		  
void RT::Frame::SetDefaultCamera()
{
	assert(!Stack.Data.empty());

	Parent = &Stack.Data[0]; 
	static RT::Camera default_camera;
	default_camera.Id = "COLLADA_RT_default_camera";
	Camera = &default_camera;
}
void RT::Frame::SetNextCamera()
{
	size_t i = Parent-&Stack.Data[0];
	for(;i<Stack.Data.size();i++)
	{
		std::vector<RT::Camera*> &ic = Stack.Data[i].Node->Cameras;
		if(ic.empty()) continue;

		size_t j = 0;
		if(Parent==&Stack.Data[i])
		{
			while(j<ic.size()&&Camera!=ic[j])
			j++;
			if(j>=ic.size()-1) continue;
		}
		Parent = &Stack.Data[i]; Camera = ic[j]; break;
	}
	if(i>=Stack.Data.size()) SetDefaultCamera(); 
	
	daeEH::Verbose<<"Active camera instance on node "<<
	Parent->Node->Id<<" is based on camera "<<Camera->Id;	
}

//SCHEDULED FOR REMOVAL
//This is keeping old/lousy code from breaking.
RT::Stack_Data &RT::Stack::FindAnyLight()
{
	//The Cg LIGHTPOSITION semantic is calling
	//this at a high data rate. This speeds up
	//very large scene graphs until it's fixed.
	if(_FoundAnyLight!=nullptr)
	return *_FoundAnyLight;
	
	//Empty default lights in case reloading??
	Data[0].Node->Lights.clear();

	size_t ambient = 0; //ambient
	for(size_t i=0;i<Data.size();i++)
	{
		std::vector<RT::Light*> &il = Data[i].Node->Lights;
		if(il.empty()) 
		continue;
		if(il[0]->Type==RT::Light_Type::AMBIENT)
		{
			ambient = i; 
			if(il.size()==1)
			continue;
			//Assuming client is not tracking this.
			std::swap(il[0],il[1]); 
		}
		_FoundAnyLight = &Data[i]; return *_FoundAnyLight;
	}
	//Assuming ambient is as good as any positioned at 0,0,0.
	if(0==ambient)
	{
		static RT::Stack_Data position;
		static RT::Light default_light[6];		
		for(int i=0;i<6;i++)
		{
			default_light[i].Id = "COLLADA_RT_default_light";
			Data[0].Node->Lights.push_back(default_light+i);
		}			
		position = Data[0];
		position.Matrix[M30] = position.Matrix[M31] = 
		position.Matrix[M32] = RT::Main.SetRange.Zoom;
		_FoundAnyLight = &position; 
	}
	else _FoundAnyLight = &Data[ambient]; return *_FoundAnyLight;
}
		  
bool RT::Stack::Select_AddData(RT::Node *p, RT::Stack_Data *pp, size_t depth)
{
	//Reminder: capacity is established by RT::Stack::Select().
	if(Data.size()==Data.capacity()) 
	{
		//This can happen because Select_AddData_Controllers adds controllers
		//but doesn't track depth. It means CountDescendants is depth limited.
		daeEH::Error<<"Truncated graph capacity reached. Truncating controller.";
		return false;
	}
	else if(depth>COLLADA_RT_MAX_DEPTH)
	{
		if(depth!=size_t(-1)) //Passing secret messages.
		{
			daeEH::Error<<"COLLADA_RT_MAX_DEPTH reached. Truncating graph.";
			return false;
		}
		depth = 0; assert(1==Data.size()); //YUCK: Don't overwrite Data[0].
	}
	else Data.push_back(RT::Stack_Data());

	RT::Stack_Data &d = Data.back(); d.Node = p; d.Parent = pp;
		
	for(size_t i=0;i<p->Geometries.size();i++)	
	Select_AddData_DrawData(p->Geometries[i],nullptr,&d);	
	for(size_t i=0;i<p->Controllers.size();i++)
	Select_AddData_DrawData(nullptr,p->Controllers[i],&d);

	for(size_t i=0;i<p->Nodes.size();i++) 
	Select_AddData(p->Nodes[i],&d,depth+1); return true;
}
void RT::Stack::Select_AddData_DrawData(void *g, void *c, RT::Stack_Data *d)
{
	RT::Stack_Draw draw; 
	draw.Data = d; draw.Instance = g!=nullptr?g:c;
	if(g!=nullptr) draw.first = ((RT::Geometry_Instance*)g)->Geometry;
	if(c!=nullptr) draw.second = ((RT::Controller_Instance*)c)->Controller;
	if(c!=nullptr) draw.first = draw.second->Geometry;
	DrawData.push_back(draw);
}
void RT::Stack::Select_AddData_Controllers_and_finish_up()
{	
	//<instance_controller> cannot create an instance of nodes.
	//Therefore if a controller must be independently animated
	//or bound to a physical system, it must be uniquely bound.
	for(size_t i=0;i<DrawData.size();)
	{
		RT::Stack_Draw &d = DrawData[i++];
		RT::Controller_Instance *ic = d.AsController();
		while(i<DrawData.size()&&d.second==DrawData[i].second)
		{
			//Skip instances of controllers sharing a skeleton.
			if(!DrawData[i].Advances_ControllerData(ic))
			i++;
			else break;
		}
		if(ic==nullptr) continue;

		for(size_t j=ic->Skeletons;j<ic->Joints.size();)
		{
			RT::Stack_Data *p = FindData(ic->Joints[j]);
			if(p!=nullptr) 
			{
				j++; ControllerData.push_back(p); 
			}
			else if(nullptr!=ic->Joints[j])
			{
				//Must add from a library_nodes.
				bool finished = true;				
				for(size_t k=0;k<ic->Skeletons;k++)				
				if(nullptr==FindData(ic->Joints[k]))
				finished = !Select_AddData(ic->Joints[k],&Data[0],0);
				if(finished) goto finish_up;
			}
			else do //Should be identity matrix.
			{
				j++; ControllerData.push_back(&Data[0]);

			}while(j<ic->Joints.size()&&nullptr==ic->Joints[j]);			
		}		
	}

	finish_up: //THE "_and_finish_up" BITS.
	{	
		//SCHEDULED FOR REMOVAL
		_FoundAnyLight = nullptr; FindAnyLight(); 
				
		//A reset is expected and is needed to approximate
		//the circumference of the visible world for below.
		Reset_Update(); 

		RT::Main.SetRange.Clear(); FX::Float3 x;
		for(size_t i=0;i<DrawData.size();i++) for(int j=0;j<2;j++)
		RT::Main.SetRange+=&RT::MatrixTransform
		(DrawData[i].Data->Matrix,DrawData[i].first->SetRange.Box[j],x).x;
		RT::Main.SetRange.FitZoom();		
		RT::Main.SetDefaultCamera(); RT::Main.Center(); 
		 
		//I feel guilty for scanning for these evert frame.
		ShowHierarchy_Splines.clear();
		RT::Geometry *f,*g = nullptr;
		for(size_t i=0;i<DrawData.size();i++)
		{
			f = DrawData[i].first; if(f==g) continue;
			g = f;
			if(g->IsSpline()&&ShowHierarchy_Splines.size()%2==0
			||!g->IsSpline()&&ShowHierarchy_Splines.size()%2==1)
			ShowHierarchy_Splines.push_back((int)i);
		}
		if(ShowHierarchy_Splines.size()%2==1)
		ShowHierarchy_Splines.push_back(DrawData.size());
	}
}

void RT::Stack::Update(bool resetting)
{
	//This resets physics simulations and 
	//intializes the transform data, so if
	//a node is transformed, it just has to
	//be reset.
	if(resetting) 
	{
		//This can go in the main loop, but
		//it makes it hard to see what's what.
		TransformData.clear();				
		for(size_t i=1;i<Data.size();i++)
		{
			std::vector<RT::Transform> &v = Data[i].Node->Transforms;						
			for(size_t j,i=0;i<v.size();i++)
			{
				for(j=0;j<v[i].Size;j++)
				TransformData.push_back(v[i].ResetData[j]);	
				if(j==16&&!RT::Matrix_is_COLLADA_order)
				RT::Matrix4x4Transpose(*(RT::Matrix*)(&TransformData.back()-15));
			}			
		}
		if(TransformData.empty()) //C++98/03
		TransformData.resize(1);

		for(size_t i=0;i<DrawData.size();i++)
		{
			RT::Stack_Draw &d = DrawData[i];
			for(RT::Controller *p=d.second;p!=nullptr;p=p->Source)
			{
				RT::Morph *m = dynamic_cast<RT::Morph*>(p);
				if(m!=nullptr) m->Reset_AnimatedWeights();				
			}
		}
	}
	RT::Float *td = &TransformData[0]; //C++98/03	 
	
	for(size_t i=1;i<Data.size();i++)
	{
		RT::Stack_Data &d = Data[i];		
		if(d.Physics==nullptr||resetting)
		td = d.Update_Matrix(td);
		else for(size_t i=0;i<d.Node->Transforms.size();i++)
		td+=d.Node->Transforms[i].Size;
	}

	if(resetting) RT::Main.Physics.Reset();
}
RT::Float *RT::Stack_Data::Update_Matrix(RT::Float *td)
{
	RT::Transform *tf;
	for(size_t i=0;i<Node->Animators.size();i++)
	Node->Animators[i].Animate(td);

	//NOTE: It's thought best to translate into a universal
	//coordinate system as soon as possible. Doing relative
	//transforms between them is complicated by controllers
	//and geometry level <up_axis> and <unit> prerequisites.

	size_t up_i = Node->Asset;
	RT::Up_Meter up_m = CrtRender_Up_Meter[up_i];
	RT::Up &up = up_m.first; RT::Float &m = up_m.second;

	//These are for matrix/lookat/skew.
	RT::Matrix lm; RT::Float *pd;

	RT::Float x,y,z;	
	RT::MatrixCopy(Parent->Matrix,Matrix);
	for(size_t i=0;i<Node->Transforms.size();i++,td+=tf->Size)
	{
		tf = &Node->Transforms[i];
																		  
		if(tf->Size<=4) //Rotate, Translate, Scale.
		{
			x = td[0]; y = td[1]; z = td[2];
			if(RT::Up::Y_UP!=up)
			std::swap(y*=-1,RT::Up::X_UP==up?x:z);
		}

		switch(tf->Type)
		{
		case RT::Transform_Type::ROTATE:

			if(td[3]!=0) //0 is very common for no reason.
			RT::MatrixRotateAngleAxis(Matrix,x,y,z,td[3]);
			break;

		case RT::Transform_Type::TRANSLATE: 

			RT::MatrixTranslate(Matrix,x*m,y*m,z*m);
			break;

		case RT::Transform_Type::SCALE:

			if(up!=RT::Up::Y_UP)
			{
				#ifdef NDEBUG
				#error Try to detect/respect negative scale?
				#endif
				//Don't know if this is technically correct
				//but negative scaling makes everything off.
				x = fabs(x); z = fabs(z);
			}
			RT::MatrixScale(Matrix,x,y,z);
			break;

		case RT::Transform_Type::LOOKAT: 
			
			//This is the gluLookAt documentation.
			//(Except without the inversion step.)
			
			RT::Normalize(td[6],td[7],td[8]); //Up

			lm[M20] = td[0]-td[3]; //Interest
			lm[M21] = td[1]-td[4];
			lm[M22] = td[2]-td[5];
			RT::Normalize(lm[M20],lm[M21],lm[M22]); 

			//The orthonormal rotation bivectors.
			RT::Cross<M20,M21,M22, 6,7,8, M00,M01,M02>(lm,td,lm);
			RT::Normalize(lm[M00],lm[M01],lm[M02]); 
			
			RT::Cross<M20,M21,M22, M00,M01,M02, M10,M11,M12>(lm,lm,lm);

			lm[M30] = td[0]; lm[M31] = td[1]; lm[M32] = td[2];			 
			lm[M03] = lm[M13] = lm[M23] = 0; lm[M33] = 1;

			pd = lm; goto finish_lookat_or_skew;

		case RT::Transform_Type::MATRIX:
		
			pd = td; finish_lookat_or_skew: //pd replaces td.

			if(0!=up_i)
			{
				//Node matrixes cannot factor <unit> into their scaling.
				//But should factor it into their translation component.
				if(up!=RT::Up::Y_UP)
				{	
					#ifdef NDEBUG
					#error Try to understand/document this.
					#error It's not swapping/flipping basis vectors.
					#endif
					RT::Matrix xz;
					RT::MatrixLoadAsset(xz,up);
				//EXPERIMENTAL
				//This is poorly understood, but the -1 values required
				//by X_UP and Z_UP cause scaling issues in matrices and
				//so must be cancelled out somehow. This was determined
				//to work experimentally. Like using abs() with <scale> 
				//it may or may not be correct.					
				std::swap(xz[M10],xz[M01]); std::swap(xz[M21],xz[M12]);
				{
					//Two multiplies translates into a coordinate system
					//producing a mutant identity matrix with two -1s if
					//necessary that cancel each other out.
					RT::MatrixMult(xz,*(RT::Matrix*)pd,lm);
				}
				//It might be possible to swap once, here, by swapping
				//the multiplication, but the compiler can sort it out.
				std::swap(xz[M10],xz[M01]); std::swap(xz[M21],xz[M12]);

					//THIS IS THE SECOND OF THE "Two multiplies" COMMENT.
					RT::MatrixMult(lm,xz,lm);
					pd = lm;
				}
				else //TODO? A CUSTOM API COULD AVOID SWAPPING.
				{
					x = pd[M30]; y = pd[M31]; z = pd[M32];
				}
				pd[M30]*=m; pd[M31]*=m; pd[M32]*=m;
			}

			RT::MatrixMult(*(RT::Matrix*)pd,Matrix);

			//TODO? A CUSTOM API COULD AVOID SWAPPING.
			if(pd==td&&0!=up_i) 
			{
				pd[M30] = x; pd[M31] = y; pd[M32] = z;
			}
			break;

		  case RT::Transform_Type::SKEW:

			//This is very dodgy. This transform comes from RenderMan.
			//RenderMan doesn't define it in numbers. Instead it uses
			//very squishy words.
			//This illustration comes from an OpenCOLLADA source code.
			//It's not clear OpenCOLLADA implements any math for this.
			//The illustration doesn't say which vector is 'd' or 'e'
			//either. I wish I was famliliar enough with skew mapping
			//to say. The large demo.dae uses <skew> to stretch flora 
			//out so it seems to vary.
			//
			//1) COLLADA uses the RenderMan standard:
			//[ 1+s*dx*ex   s*dx*ey   s*dx*ez  0 ]
			//[   s*dy*ex 1+s*dy*ey   s*dy*ez  0 ]
			//[   s*dz*ex   s*dz*ey 1+s*dz*ez  0 ]
			//[         0         0         0  1 ]
			//where s = tan(skewAngle), if the axes are normalized 

			RT::Normalize(td[1],td[2],td[3]);
			RT::Normalize(td[4],td[5],td[6]);
			x = tan(td[0]*RT::DEGREES_TO_RADIANS);

			//This matches COLLADA-CTS.
			lm[M00] = x*td[1]*td[4]+1; 
			lm[M10] = x*td[1]*td[5];
			lm[M20] = x*td[1]*td[6];
			lm[M30] = 0;
			lm[M01] = x*td[2]*td[4]; 
			lm[M11] = x*td[2]*td[5]+1;
			lm[M21] = x*td[2]*td[6];
			lm[M31] = 0;
			lm[M02] = x*td[3]*td[4]; 
			lm[M12] = x*td[3]*td[5];
			lm[M22] = x*td[3]*td[6]+1;
			lm[M32] = 0;
			lm[M33] = 1;

			pd = lm; goto finish_lookat_or_skew;
		}
	}

	return td;
} 

//These are not inline to make it easier to experimenting
//with changing them without dirtying precompiled headers.
void RT::Camera_State::Rotate(RT::Float p, RT::Float t)
{
	Pan+=p; if(Pan<-180) Pan+=360; if(Pan>180) Pan-=360;
		
	Tilt+=t; if(Tilt<-90) Tilt = -90; if(Tilt>90) Tilt = 90;
}
void RT::Camera_State::Fly(RT::Float xx, RT::Float yy, RT::Float zz)
{
	//Currently placed cameras can only adjust their position once they
	//are zoomed in or out. The distance is modulated by zoom so the 
	//default camera can adapt to different scales.
	//Placed cameras don't have a distance parameter so this is just
	//to fine tune zooming.
	if(abs(Zoom)<0.00001) return;
	if(Zoom<0){ xx = -xx; yy = -yy; zz = -zz; }

	#ifdef NDEBUG
	#error Do this with quaternions.
	#endif
	RT::Matrix m;
	RT::MatrixLoadIdentity(m);
	RT::MatrixRotateAngleAxis(m,0,1,0,Pan);
	RT::MatrixRotateAngleAxis(m,1,0,0,Tilt);
	RT::MatrixTranslate(m,xx*Zoom,yy*Zoom,zz*Zoom);
	
	X+=m[M30]; Y+=m[M31]; Z+=m[M32];
}
void RT::Camera_State::Walk(RT::Float zz, RT::Float xx, RT::Float yy)
{
	//See body of Fly for thoughts on this.
	if(abs(Zoom)<0.00001) return;
	if(Zoom<0){ xx = -xx; yy = -yy; zz = -zz; }

	RT::Float c = cos(Pan*RT::DEGREES_TO_RADIANS);
	RT::Float s = sin(Pan*RT::DEGREES_TO_RADIANS);

	zz*=Zoom; xx*=Zoom;
	X+=-c*xx+s*zz;
	Y+=yy*Zoom;
	Z+=s*xx+c*zz;
} 
void RT::Camera_State::Center()
{
	Pan = Tilt = X = Y = Z = 0; 
	
	//HACK: This is for models at ground level.
	if(Camera->Id=="COLLADA_RT_default_camera")
	{
		Y = RT::Main.SetRange.Y();
		Zoom = RT::Main.SetRange.Zoom;	
		if(Zoom!=0)
		daeEH::Verbose<<"Zoom is "<<Zoom;
	}
	else Zoom = 0; 
}
void RT::Camera_State::Matrix(RT::Matrix *view, RT::Matrix *inverseview)const
{
	bool invert = view!=nullptr;	
	if(!invert) view = inverseview; 
	if(inverseview==nullptr) inverseview = view;
	RT::MatrixCopy(Parent->Matrix,*inverseview);	

	#ifdef NDEBUG
	#error This isn't really understood.
	#endif
	//This works to make the camera interactive.
	RT::Matrix &i = *inverseview;
	switch(RT::GetUp_Meter(Parent->Node->Asset).first)
	{
	case RT::Up::X_UP:

		std::swap(i[M10],i[M00]*=-1);
		std::swap(i[M11],i[M01]*=-1);
		std::swap(i[M12],i[M02]*=-1); break;

	case RT::Up::Z_UP:

		std::swap(i[M10],i[M20]*=-1);
		std::swap(i[M11],i[M21]*=-1);
		std::swap(i[M12],i[M22]*=-1); break;
	}

	RT::MatrixTranslate(*inverseview,X,Y,Z);
	RT::MatrixRotateAngleAxis(*inverseview,0,1,0,Pan);
	RT::MatrixRotateAngleAxis(*inverseview,1,0,0,Tilt);
	RT::MatrixTranslate(*inverseview,0,0,Zoom);
	//*view is used for regular rendering. *inverseview is for lighting.
	if(invert) 
	{
		RT::Matrix3x4Invert(*inverseview,*view);
		(*view)[M03] = (*view)[M13] = (*view)[M23] = 0; (*view)[M33] = 1;
	}
}

//-------.
	}//<-'
}

/*C1071*/