/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include "viewer_base.pch.h" //IntelliSense

#include "../../dom/include/WARNING.HPP" //STFU (GCC)

//demo.dae floods the console.
//See Silencer implemented below.
#define Silence_console_if_DEBUG 0 //-Wunused-variable
					  
//This header is used by all of the system specific top-level
//translation units. It doesn't have to be "inl" at this point
//because the RT & FX subsystems are able to find their barings
//on their on. Various "hpp" headers are slipped into the middle
//of it. (It's a work in progress.)

//EXPERIMENTAL
//GLUI implements the location bar. There isn't much at all to it
//but that could change.
static int COLLADA_viewer = 0;
static std::string COLLADA_viewer_go;
#ifdef GLUT_API_VERSION
#ifndef NO_GLUI
#include "viewer_GLUI.hpp"
#endif
#endif
		  
using namespace COLLADA;  
					 
//Assuming PS3 uses Berkley Sockets.
#include "viewer_HTTP.hpp"

//SCHEDULED FOR REMOVAL?
//One advantage to linking these
//here (if there is any other) is
//it forces RT and FX to be rebuilt.
COLLADA_(namespace)
{
	struct GL GL; 

	namespace RT
	{
		::COLLADA::RT::Frame Main;
	}
}

static bool visible = true;
static bool fullscreen = false;
static bool togglewireframe = false;
static bool togglelighting = true;
static int togglecullingface = 2;
static void DrawGLScene()
{
	if(!visible) return; //assert(visible);

	//Clear The Screen And The Depth Buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//EXPERIMENTAL
	if(!COLLADA_viewer_go.empty())
	{
		COLLADA_viewer_main2(COLLADA_viewer_go.c_str());
		COLLADA_viewer_go.clear();		
	}

	RT::Main.Refresh();
} 						
static void RestoreGL()
{
	//Reminder: The fallback renderer will use these.
	//Assuming COLLADA wants behavior more like this.
	//Technically profile_COMMON should use technical shaders.
	//glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SINGLE_COLOR);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);	
	//THIS WOULD BE NICE, BUT IT GETS THE NORMALS WRONG WHEN
	//THEY ARE MIRRORED BY NEGATIVE SCALES. INSTEAD OF CHECKING 
	//IF IT'S A FRONT/BACK FACE IT USES THE DETERMINANT OF THE NORMAL
	//MATRIX TO SELECT A FRONT FACE UNRELATED TO WINDING.
	//(Profile_COMMON does it right in per-pixel rendering mode.)
	if(0) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
	//"The initial ambient scene intensity is (0.2, 0.2, 0.2, 1.0)."
	const GLclampf black[4] = {0,0,0,1};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,black);

	glShadeModel(GL_SMOOTH);
	//glClearColor(r,g,b,a); glClearDepth(1);	
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); //LEQUAL?

	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE); //"inverse transpose" needs post-normalization.
}
//GL Setup (for some reason PS3_main uses 0,0,1,0.5)
static void InitGL(float r=0.9f, float g=0.9f, float b=0.9f, float a=1)
{
	//SCHEDULED FOR REMOVAL
	//These states are clobbered by Cg.
	RestoreGL();

	glClearColor(r,g,b,a); glClearDepth(1);		
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

	//Sometimes models present incorrectly because back-faces obscure them.
	assert(2==togglecullingface);
	//glEnable(GL_CULL_FACE); glCullFace(GL_BACK); //1	
}
//Resize And Initialize The GL Window
static void ResizeGLScreen(int width, int height)
{
	RT::Main.Width = width; RT::Main.Height = height;

	#ifdef GLUI_VERSION	
	UI.site->set_w(RT::Main.Width-GLUI_XOFF*3-2);
	#endif
}

static float MouseWheelSpeed = 0.02f/1.5f;
static float MouseRotateSpeed = 0.5f;
static float MouseTranslateSpeed = 0.0025f;
static float KeyboardRotateSpeed = 5;
static float KeyboardTranslateSpeed = 10;
static void AdjustUISpeed(float x)
{
	if(x>1&&MouseRotateSpeed>=1.25
	 ||x<1&&MouseRotateSpeed<=0.25) return;

	//Dampening rotation. Increasing translation.
	float d = (x-1)/2; float rx = x-d; if(d>0) x+=d;
	KeyboardRotateSpeed*=rx; KeyboardTranslateSpeed*=x;
	MouseRotateSpeed*=rx; MouseTranslateSpeed*=rx; MouseWheelSpeed*=x;
}

static bool showSite = true;
static bool mouseSwap = false;
static int Xpos = 0, Ypos = 0; //F11
static int Xsize = 640, Ysize = 480; //F11
static void ProcessInput(unsigned char ASCII)
{			   
	//glutSpecialFunc_callback has more inputs.
	//It depends on GLUT enums that maybe GLUI
	//could define.

	switch(ASCII)
	{
	case '\x1b': //Esc
	case '\r': //Enter

		#ifdef GLUI_VERSION				
		if(ASCII=='\r'||!showSite)
		{
			showSite = true; UI.show();
			//UI.site->activate(GLUI_ACTIVATE_TAB);
			UI.activate_control(UI.site,GLUI_ACTIVATE_TAB);
		}
		else if(ASCII!='\r'&&showSite)
		{
			showSite = false; UI.hide(); 
		}
		#endif
		break;

	case 'z': case 'Z':

		mouseSwap = !mouseSwap;
		daeEH::Warning<<"[Z] swapped left and right buttons.";
		#ifdef GLUT_API_VERSION
		glutDetachMenu(mouseSwap?2:0);
		glutAttachMenu(mouseSwap?0:2);
		#endif
		break;

	case '\t':

		RT::Main.SetNextCamera();
		RT::Main.Center(); 
		break;

	case 'c': case 'C': 
		
		RT::Main.Center(); 
		break;

	case 'l': case 'L':
		
		if(!RT::Main.ShowCOLLADA_FX)
		{
			if(togglelighting=!togglelighting)
			{
				glEnable(GL_LIGHTING);		
				RT::Main.ShowCOLLADA_FX = true;				
				daeEH::Verbose<<"[L]ighting enabled. FX enabled.";
			}
			else 
			{
				glDisable(GL_LIGHTING);		
				daeEH::Verbose<<"[L]ighting disabled.";
			}
		}
		else 
		{
			RT::Main.ShowCOLLADA_FX = false;
			RestoreGL();
			daeEH::Verbose<<"[L]egacy OpenGL mode enabled. FX disabled.";
		}
		break;

	case 'm': case 'M':

		AdjustUISpeed(1.25);
		break;

	case 'n': case 'N':

		AdjustUISpeed(0.75);
		break;

	case 'o': case 'O':

		RT::Main.AnimationOn = !RT::Main.AnimationOn;
		break;

	case 'i': case 'I':	//INTERPOLATION

		//Reminder: COLLADA_viewer_main is turning this on
		//because the samples/ have bad animation data and
		//because COLLADA resources tend to be low quality.
		if(RT::Main.AnimationLinear=!RT::Main.AnimationLinear)
		daeEH::Verbose<<"[I]NTERPOLATION is off. LINEAR animation is on.";
		else
		daeEH::Verbose<<"[I]NTERPOLATION is on. LINEAR animation is off.";		
		break;

	case 'p': case 'P':

		if(!RT::Main.AnimationOn)
		{
			RT::Main.AnimationOn = true;
			RT::Main.AnimationPaused = false;			
		}
		else RT::Main.AnimationPaused = !RT::Main.AnimationPaused;
		break;

	case 'q': case 'Q':

		if(togglewireframe=!togglewireframe)
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);		
		else
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);		
		break;

	case 's': case 'S': //backward
	
		RT::Main.Walk(RT::Main.Delta*KeyboardTranslateSpeed,0,0);
		break;

	case 'w': case 'W': //forward
	
		RT::Main.Walk(-RT::Main.Delta*KeyboardTranslateSpeed,0,0);
		break;

	case ' ': //up

		RT::Main.Walk(0,0,RT::Main.Delta*KeyboardTranslateSpeed);
		break;

	case 'x': case 'X': //down
	
		RT::Main.Walk(0,0,-RT::Main.Delta*KeyboardTranslateSpeed);
		break;

	case 'd': case 'D': //right

		RT::Main.Walk(0,-RT::Main.Delta*KeyboardTranslateSpeed,0);
		break;

	case 'a': case 'A': //left

		RT::Main.Walk(0,RT::Main.Delta*KeyboardTranslateSpeed,0);
		break;
		
	case 'f': case 'F':

		if(togglecullingface==0) //turn it back
		{ 
			togglecullingface = 1; glEnable(GL_CULL_FACE); glCullFace(GL_BACK);			
		}
		else if(togglecullingface==1) //turn it both
		{ 
			togglecullingface = 2; glDisable(GL_CULL_FACE);			
		}
		else //turn it front
		{ 
			togglecullingface = 0; glEnable(GL_CULL_FACE); glCullFace(GL_FRONT);
		}
		break;

	case 'g': case 'G':

		RT::Main.ShowGeometry = !RT::Main.ShowGeometry;
		break;	

	case 'h': case 'H':

		RT::Main.ShowHierarchy = !RT::Main.ShowHierarchy;
		break;

	case 'y': case 'Y': //EXPERIMENTAL
	{
		bool refresh; GLboolean enabled;
		if(!glIsEnabled(GL_FRAMEBUFFER_SRGB))
		{
			glEnable(GL_FRAMEBUFFER_SRGB);
			enabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
			refresh = enabled==1;
		}
		else
		{
			refresh = true; enabled = 0;
			glDisable(GL_FRAMEBUFFER_SRGB);
		}
		if(refresh&&RT::Main.DB!=nullptr)
		{
			for(size_t i=0;i<RT::Main.DB->Images.size();i++)
			if(RT::Main.DB->Images[i]->sRGB)
			RT::Main.DB->Images[i]->Refresh();
		}

		if(enabled==1) //y looks like the Greek letter gamma.
		daeEH::Verbose<<"[y] sRGB gamma correct mode enabled.";
		if(enabled==0)
		daeEH::Verbose<<"[y] sRGB gamma correct mode disabled or is unavailable.";
		break;
	}
	case '1': case '2': case '3': case '4': case '5': case '6': 

		RT::Main.ShowTextures_Mask^=1<<(ASCII-'1'); break;

	case '~': //Invert the texture-mask that enables textures.
	case 96: //~

		//Was just going to let these be like '0' but this may be useful.
		//(Beside, having '~' do operator~ is kind of cute.)
		RT::Main.ShowTextures_Mask = ~RT::Main.ShowTextures_Mask; break;

	case '0': //Set the texture mask to 0 unless it's already 0.

		RT::Main.ShowTextures_Mask = RT::Main.ShowTextures_Mask!=0?0:-1; break;

	default: daeEH::Warning<<"unused key : "<<ASCII;
	}
}

static int lastx = 0;
static int lasty = 0;
static bool mouseDown[4] = {};
static void toggleMouse(unsigned int button, bool state)
{
	//Close location bar?
	if(state&&showSite) ProcessInput('\x1b');

	if(button>2) //FreeGLUT?
	{
		//WHEEL_DELTA on Windows is 120.
		short delta = 4==button?120:-120;
		if(mouseDown[2]) delta/=12;
		RT::Main.ZoomIn(-delta*MouseWheelSpeed);
	}
	else 
	{
		//This is in case input devices have one
		//button only or any other creative uses.
		if(mouseSwap)
		if(button==0) button = 2; else 
		if(button==2) button = 0;

		mouseDown[button] = state; 
	}	
}
static void moveMouseTo(int x, int y)
{	
	int dx = lastx-x, dy = lasty-y; lastx = x; lasty = y;

	if(mouseDown[0]) //Left button?
	{
		if(mouseDown[2]) goto middle; //Left+Right?
		
		RT::Main.Rotate(dx*MouseRotateSpeed,dy*MouseRotateSpeed);
	}
	else if(mouseDown[1]) middle: //Middle button?
	{
		RT::Main.ZoomIn(dy*MouseWheelSpeed);
	}
	else if(mouseDown[2]) //Right button?
	{
		RT::Main.Fly(dx*MouseTranslateSpeed,-dy*MouseTranslateSpeed);
	}
}

//TODO: This should have a thread created so it doesn't lock the X button.
static void COLLADA_viewer_main2(const char *dae, const void *default_dae)
{	
	if(dae[0]=='\0') return;

	#ifdef GLUI_VERSION	
	if(dae==COLLADA_viewer_go.c_str())
	{
		UI.site->scroll_history(-1);
	}
	#endif
		
	//Unloading can introduce quite a lot of delay.
	if(!RT::Main.DOM.getDocs().empty())
	{
		glDisable(GL_FRAMEBUFFER_SRGB);
		daeEH::Verbose<<"Unloading: deleting memory, etc...";
		RT::Main.Unload();
	}

	//WinMain had used clock(). It seems like it's
	//part of the standard library, but RT::Time()
	//has code for 3 systems, but one of them uses
	//midnight as the wraparound time. Maybe clock
	//is better and portable too.
	RT::Float timer = RT::Time();
	{
		daeEH::Verbose<<
		(dae==default_dae?"Loading default document \"":"Loading \"")		   
		<<dae<<"\"...";			
		if(!RT::Main.Load(dae))
		{
			//Let console print any errors.
			//exit(0); //!!!!!!!!!!
		}
	}
	//If the time is negative it wrapped around.
	daeEH::Verbose<<"LOAD TIME OF "<<RT::Main.URL;
	daeEH::Verbose<<"IS "<<RT::Time()-timer<<" SECONDS";

	bool sRGB = false, lerp = false; int cull = 2;
	for(size_t i=0;i<RT::Main.COLLADA_index_keywords.size();i++)
	{
		daeName word = RT::Main.COLLADA_index_keywords[i];
		if(word=="sRGB") sRGB = true;
		if(word=="cull") cull = 1;
		if(word=="lerp") lerp = true;
	}				

	//This is already set if in the first 4096 characters.
	if(sRGB!=(1==glIsEnabled(GL_FRAMEBUFFER_SRGB))) 
	ProcessInput('y');
	//These are for problem documents only.
	while(cull!=togglecullingface)
	ProcessInput('F');
	if(lerp!=RT::Main.AnimationLinear)
	ProcessInput('I'); 
	//This shows the first camera, but is it as nice as the
	//default camera?
	//ProcessInput('\t');
		  	
	#ifdef GLUI_VERSION	
	//SCHEDULED FOR REMOVAL
	//GLUI isn't Unicode (yet.)
	char *ASCII = URI_to_ASCII(RT::Main.URL);
	if(0!=strcmp(ASCII,UI.site->get_text()))
	{
		//add_to_history is a bogus internal API.
		//The argument is not added, the current text is 
		//if the argument is not empty.
		UI.site->set_text(ASCII);
		UI.site->add_to_history(ASCII);
		//"" is needed to prevent double-stuffing.
		UI.site->set_text("");
		UI.site->scroll_history(-1);
	}			
	#endif
}			

//TEMPORARY //TEMPORARY //TEMPORARY

//EXPERIMENTAL
HTTP_agent TestIO_HTTP_agent; //HACK

struct TestIO : daeIO //QUICK & DIRTY
{	
	#ifdef _WIN32
	typedef wchar_t Maxpath[MAX_PATH];
	#else
	typedef char Maxpath[260*4*(sizeof(char)==1)];
	#endif
		
	virtual daeError getError()
	{
		return OK;  
	}
	virtual size_t getLock(Range *rngI, Range *rngO)
	{
		if(OK==DAE_OK&&lock==size_t(-1))
		{	
			lock = 0; Maxpath maxpath;
			if(!I.getRequest().isEmptyRequest())
			{
				int i = 0; 
				if(_file_protocol(maxpath,I.getRequest().remoteURI))
				{
					#ifdef _WIN32
					r = _wfsopen(maxpath,L"rb",_SH_DENYWR);
					#else
					r = fopen(maxpath,"rb");
					#endif
				}
				else //HACK: It's already open to peek at <COLLADA xmlns>.
				{
					if(!TestIO_HTTP_agent.connected())
					new(&TestIO_HTTP_agent) HTTP_agent(&I.getRequest(),rngI);
					//i = TestIO_HTTP_agent.ContentLength();
					i = TestIO_HTTP_agent.ContentRange()[2];
				} 
				if(r!=nullptr)
				{
					fseek(r,0,SEEK_END); 					
					i = ftell(r); fseek(r,0,SEEK_SET);
					setRange(rngI,nullptr);
				}
				if(i>0) lock = i; else OK = DAE_ERROR;
			}
			if(!O.getRequest().isEmptyRequest())
			{
				if(_file_protocol(maxpath,O.getRequest().remoteURI))
				{
					//EXPERIMENTAL
					//HACK? Assuming if range is specified, the file 
					//is retained. r+b seems to be the way to do that.
					//TODO? should probably truncate it to rngO->second.
					bool r_b = rngO!=nullptr?1:0;
					#ifdef _WIN32
					w = _wfsopen(maxpath,r_b?L"r+b":L"wb",_SH_DENYRW);
					#else
					w = fopen(maxpath,r_b?"r+b":"wb");
					#endif
					setRange(nullptr,rngO);
				}
				if(w==nullptr) OK = DAE_ERROR; 
			}			
		}
		return lock;
	}
	virtual size_t setRange(Range *rngI, Range *rngO) 
	{
		if(lock==size_t(-1)) return getLock(rngI,rngO);

		if(rngI!=nullptr) if(TestIO_HTTP_agent.connected()) 
		{
			#ifdef NDEBUG
			#error Keep the resume_RFC1123 timestamp and provide a 
			#error way for to set the daeIO's timestamp in advance.
			#endif
			TestIO_HTTP_agent.~HTTP_agent();
			new(&TestIO_HTTP_agent) HTTP_agent(&I.getRequest(),rngI);
		}
		else if(r!=nullptr)
		{
			fseek(r,std::min(lock,rngI->first),SEEK_SET);
			rngI->limit_to_size(lock);
		}
		if(rngO) if(w!=nullptr)
		{
			fseek(w,0,SEEK_END); 					
			size_t i = ftell(w);
			fseek(w,std::min(i,rngO->first),SEEK_SET);
			rngO->limit_to_size(i);
		}

		return lock;
	}
	virtual daeOK readIn(void *in, size_t chars)
	{
		if(TestIO_HTTP_agent.connected())
		{
			if(!TestIO_HTTP_agent.readIn(in,chars)) OK = DAE_ERROR;
		}
		else if(1!=fread(in,chars,1,r)&&chars!=0) OK = DAE_ERROR; return OK;
	}
	virtual daeOK writeOut(const void *out, size_t chars)
	{
		if(1!=fwrite(out,chars,1,w)&&chars!=0) OK = DAE_ERROR; return OK;
	}

	//This is not the normal way to go about this.
	//It's just easy to set up for basic file I/O.	
	daeIOPlugin &I,&O; daeOK OK; FILE *r,*w; size_t lock;
	TestIO(std::pair<daeIOPlugin*,daeIOPlugin*> IO)
	:I(*IO.first),O(*IO.second),r(),w(),lock(-1)
	{}
	~TestIO()
	{
		if(r!=nullptr) fclose(r); 
		if(w!=nullptr) fclose(w); 
	}
	 		
	static bool _file_protocol(Maxpath &maxpath, const daeURI *URI)
	{							   
		if(URI==nullptr||"file"!=URI->getURI_protocol())
		return false;
		#ifdef _WIN32
		daeRefView v = URI->getURI_upto<'?'>(); int UNC = v[7]=='/'?8:5;
		maxpath[MultiByteToWideChar(CP_UTF8,0,v+UNC,v.extent-UNC,maxpath,MAX_PATH-1)] = 0;
		#else
		//ASSUMING NOT REMOTE FOR NOW
		daeRefView v = URI->getURI_path();
		if(v.extent+sizeof("/cygdrive")>=sizeof(maxpath))
		return false;
		int cygdrive = 0;
		#ifdef __CYGWIN__
		if(v[0]=='/'&&v[2]==':') //Open command line from outside Cygwin?
		{
			cygdrive = strlen(strcpy(maxpath,"/cygdrive/c/"));
			maxpath[sizeof("/cygdrive")] = v.view[1]; v.view+=3; v.extent-=3;
		}
		#endif
		memcpy(maxpath+cygdrive,v.view,v.extent); maxpath[cygdrive+v.extent] = '\0';
		#endif
		return true;
	}
};
static struct TestPlatform : daePlatform //SINGLETON
{
	HTTP_agent *HTTP_downloading;

	daeIOController::Stack<TestIO> IO_stack;
	
	virtual const daeURI &getDefaultBaseURI(daeURI &URI)
	{
		char UTF[4*260+8] = "file:///"; //MAX_PATH
		#ifdef _WIN32
		TestIO::Maxpath maxpath;
		GetCurrentDirectoryW(MAX_PATH,maxpath);
		int UNC = maxpath[0]=='//'?5:8;
		int len = UNC+WideCharToMultiByte(CP_UTF8,0,maxpath,-1,UTF+UNC,sizeof(UTF)-8,0,0);
		#else
		if(!getcwd(UTF+7,sizeof(UTF)-7))
		return URI;
		int len = strlen(UTF)+1; //WideCharToMultByte includes the 0 terminator.
		#endif
		//If the directory URI doesn't end in a slash it looks like a filename?
		switch(UTF[len-2]) 
		{
		case '\\': case '/': break; default: UTF[len-1] = '/'; UTF[len] = '\0';
		}
		URI.setURI(UTF); return URI;
	}
	virtual daeOK resolveURI(daeURI &URI, const daeArchive &DOM_or_ZAE)
	{
		//Getting this when running outside of Visual Studio?
		if(URI.empty()) return DAE_ERROR;

		//Try to handle non-URI paths.
		//NOTE: This might be different if the default-base
		//URI is "file:\\" instead of the current directory.
		if(URI.isUnparentedObject()&&URI.getURI_upto<'/'>().empty())
		{
			daeString d = URI.data(), base = nullptr;
			longpath:
			int alpha = isalpha(d[0]);
			switch(d[alpha]) //Not thoroughly tested.
			{
			#ifndef _WIN32
			//HACK: NEEDS TO BE EXPANDED.
			case '~': base = "file://"; //Home?
			#endif
			case ':': base = "file:///"; break;

			case '/': case '\\': if(alpha!=0) break;

				if(d[1]=='/'||d[1]=='\\')
				{
					if(d[2]=='?') //Windows long path?
					{
						d+=2; goto longpath;
					}
					base = "file:"; break;
				}
				base = "file://"; break;

			default: if(alpha==0) break; //Hostname?

				for(size_t i=0;i<URI.size();i++) if(':'==d[i])
				{
					base = "file://"; break;					
				}			
			}
			if(base!=nullptr) 
			{
				//passing "file:\\" as a base URI might work.
				//But just manually concatenating.
				COLLADA_SUPPRESS_C(4996)
				char cat[4*260+8]; strcpy(cat,base); //MAX_PATH
				memcpy(cat+strlen(base),d,sizeof(cat)-strlen(base));
				cat[sizeof(cat)-1] = '\0';
				URI.setURI(cat);
			}
		}
		//daeEH::Verbose<<"Resolving URI: "<<URI.getURI();
		daeOK OK = URI.resolve_RFC3986(DOM_or_ZAE);
		//daeEH::Verbose<<"Resolved URI: "<<URI.getURI();
		assert(URI.getURI_protocol().size()>=4||URI.empty()); return OK;
	}
	virtual daeOK openURI(const daeIORequest &req, daeIOPlugin *I, daeURIRef &URI)
	{
		daeDocRoot<> doc; req.resolve();		
		if(!req.localURI->getURI_extensionIs("dae"))		
		if(!req.localURI->getURI_extensionIs("zae"))
		{
			daeEH::Error<<"Unsupported file extension "<<req.localURI->getURI_extension();
			req.unfulfillRequest(doc); return doc;
		}
		else //ZAE
		{
			req.fulfillRequestI(nullptr,I,doc);
			daeArchive *zae = doc->a<daeArchive>();
			if(zae==nullptr) return doc;

			daeURI URI;
			daeDocumentRef manifest_xml =
			zae->openDoc<domAny>("manifest.xml")->getDocument();			
			if(manifest_xml!=nullptr
			&&!manifest_xml->getRoot().empty())
			{
				daeElementRef dae_root = manifest_xml->getRoot();
				if("dae_root"!=dae_root->getNCName())
				dae_root = dae_root->getDescendant("dae_root"); 
				if(dae_root!=nullptr)				
				URI = dae_root->getCharData();
			}
			if(!URI.empty())
			{
				daeDocumentRef index = zae->openDoc<void>(URI);
				if(index!=nullptr)
				{
					zae->setDocument(index);
					daeRefView fragment = req.localURI->getURI_fragment();
					if(fragment.empty()) fragment = URI.getURI_fragment();
					index->getFragment() = fragment;
				}
			}
			goto zae;			
		}
		extern daeMeta *InitSchemas(int);
		daeMeta *meta = InitSchemas(Peek_xmlns(req)=="http://www.collada.org/2005/11/COLLADASchema"?5:8);
		req.fulfillRequestI(meta,I,doc); 
zae:	if(doc==DAE_OK) URI = &doc->getDocURI(); return doc; 
	}
	virtual daeIO *openIO(daeIOPlugin &I, daeIOPlugin &O)
	{
		return IO_stack.pop(std::make_pair(&I,&O));
	}
	virtual void closeIO(daeIO *IO)
	{
		IO_stack.push(IO);

		if(TestIO_HTTP_agent.connected()) TestIO_HTTP_agent.~HTTP_agent();
	}		
	virtual int getLegacyProfile()
	{
		return LEGACY_LIBXML|
		LEGACY_SIDREF_RESOLVER|LEGACY_IDREF_RESOLVER|LEGACY_URI_RESOLVER;
	}

	daeName Peek_xmlns(const daeIORequest &req)
	{
		static std::string out; out.clear();
			
		char buf[4096] = {};
		TestIO::Maxpath path; 
		if(req.scope->isArchive()) //ZAE?
		{
			//This downloads twice, but maybe it disconnects in short order.
			daeEH::Warning << "(Peeking: will disconnect in short order...)";
			daeIOSecond<> I(req); daeIOEmpty O; 
			daeIO *peek = req.scope->getIOController().openIO(I,O);
			daeIO::Range r = {0,sizeof(buf)};
			if(0!=peek->getLock(&r))
			if(!peek->readIn(buf,r.size())) assert(0);
			req.scope->getIOController().closeIO(peek);		
		}
		else if(TestIO::_file_protocol(path,req.remoteURI)) 
		{	
			std::ifstream s(path); s.read(buf,sizeof(buf));
		}
		else //HACK: Testing
		{
			new(&TestIO_HTTP_agent) HTTP_agent(&req,nullptr);
			if(TestIO_HTTP_agent.connected())
			{
				TestIO_HTTP_agent.peekIn(buf,sizeof(buf));
			}
			else TestIO_HTTP_agent.~HTTP_agent();
		}
		
		if(buf[0]=='\0') return out;

		out.assign(buf,4096);
		
		//NO need to normalize. Not a URI per se.
		out.erase(0,out.find("xmlns=\"")+7);

		//EXPERIMENTAL
		//HACK: Looking for sRGB option so textures don't
		//have to be reloaded. The sRGB status applies to
		//to the entire DOM starting at an index document.		
		if(0==out.find("http://www.collada.org"))
		{
			if(!glIsEnabled(GL_FRAMEBUFFER_SRGB))
			if(out.npos!=out.find("sRGB"))
			{			
				daeEH::Verbose<<"Found sRGB keyword in COLLADA index document's first 4096 characters...";
				ProcessInput('y');
			}
		}

		out.erase(std::min(out.find('"'),out.size()),-1); 	
		return out; 
	}

	TestPlatform(){ daeDOM::setGlobalPlatform(this); }

}TestPlatform; //SINGLETON

//EXPERIMENTAL demo.dae floods the console.
#ifdef _DEBUG
#if 0!=Silence_console_if_DEBUG
static struct Silencer : public daeStandardErrorHandler
{
	bool Verbose; //True if receiving a Verbose message.

	std::string Silenced;

	Silencer():Verbose(){ daeErrorHandler::setErrorHandler(this); }
	
	virtual void handleWarning(const daeHashString &msg, enum dae_clear clear)
	{
		//Appending an empty message is how daeErrorHandler implements
		//the RT & FX package's Verbose output strings. It's a warning
		//without the "Warning: " announcement.
		if(!clear&&msg.empty())
		{
			Verbose = true; Silenced.clear();
		}		
		if(Verbose)
		{
			if(clear)
			{
				Verbose = false; 
			}
			else Silenced.append(msg.string,msg.extent);
		}
		else 
		{
			if(!Silenced.empty())
			{
				//This is provided for context.
				daeStandardErrorHandler::handleWarning("",dae_append);
				daeStandardErrorHandler::handleWarning("Silenced: ",dae_append);
				daeStandardErrorHandler::handleWarning(Silenced.c_str(),dae_clear);
				Silenced.clear();
			}
			daeStandardErrorHandler::handleWarning(msg,clear);
		}
	}

}*Silenced = Silence_console_if_DEBUG?new Silencer:nullptr;
#endif
#endif

//GLUT //GLUT //GLUT //GLUT //GLUT //GLUT //GLUT //GLUT //GLUT

#ifdef GLUT_API_VERSION
#include "viewer_GLUT.hpp"
#endif

//UNUSED //UNUSED //UNUSED //UNUSED //UNUSED //UNUSED //UNUSED 

/*UNUSED This works but is not used.
//#include "CrtScene.h"
//#include "CrtEffect.h"
//DUNNO WHY THIS CODE IS BEING KEPT AROUND.
//This was separately implemented by the Windows/Linux/OSX branches 
//It gets amplitudeGlobalParameter. Not clear it does anything else
static void InitSceneEffects()
{
	//This block of code shows how to enumerate all the effects, get their parameters and then
	//get their UI information.
	const RT::DBase *scene = RT::Main.DB;
	if(scene==nullptr) return;

	//Get the scene and setup to iterate over all the effects stored in the FX::Loader
	for(size_t i=0;i<scene->Effects.size();i++)
	{
		FX::Effect *fx = scene->Effects[i]->FX;
		if(fx==nullptr) continue;

		//DUNNO WHY THIS CODE IS BEING KEPT AROUND.
		//Really all techniques should be listed, but it could get very redundant?
		FX::Technique *Cg = nullptr;
		for(size_t i=0;i<fx->Techniques.size();i++)
		if(fx->Techniques[i]->IsProfile_CG())
		{
			Cg = fx->Techniques[i]; break;
		}
		if(Cg==nullptr) continue;

		//This is the effect name you would use in a UI
		daeEH::Verbose<<"Cg effect name (id) "<<scene->Effects[i]->Id;

		FX::Paramable *pass[3] = { fx };
		if(fx!=Cg->Parent_Paramable)
		{
			pass[1] = Cg->Parent_Paramable;
			pass[2] = Cg;
		}
		else pass[1] = Cg;

		for(int i=0;i<3&&pass[i]!=nullptr;i++)		
		for(size_t j=0;j<pass[i]->Params.size();j++)
		{
			FX::NewParam *p = (FX::NewParam*)pass[i]->Params[i];
			//YUCK
			if(!p->IsNewParam()||p->Annotations.empty())
			continue; 

			//This is the parameter name you would use in the UI
			daeName name = p->Name; 
			//This is for the example of how to tweek a parameter (doesn't work yet)
			//if(name=="Amplitude")
			{
				//Capture the parameter and save it in a global, in a GUI you would
				//save this handle in the widget so it would know what to tweek.
				//amplitudeGlobalParameter = p;
			}

			#if 0			
			//This is here for debugging, it iterates over all the annotations and prints them out
			//so you can see what's in them.  Normally this code will be turned off.
			daeEH::Verbose<<"Parameter name "<<name;
			for(size_t i=0;i<p->Annotations.size();i++)
			{				
				FX::Annotate *q = p->Annotations[i];
				daeEH::Verbose<<"Annotation: "<<q->Name;
				switch(q->GetType())
				{
				case: CG_STRING
				daeEH::Verbose<<"value: "<<q->Value<RT::Name>();
				break;				
				case CG_FLOAT:
				daeEH::Verbose<<"value: "<<q->Value<RT::Float>(); 
				break;
				default: assert(0); //daeEH::Verbose<<"";
				}
			}
			#endif
			
			//This code currently only collects the annotation values for defining sliders and color pickers.
			//UIName is the name to attach to the widget
			//UIWidget is the widget type			
			RT::Name UIName,UIWidget;
			RT::Float UIMin = -99999, UIMax = 99999, UIStep = 0;						
			p->Annotate("UIWidget",UIWidget);
			p->Annotate("UIName",UIWidget);
			p->Annotate("UIMin",UIName);
			p->Annotate("UIMax",UIMin);
			p->Annotate("UIStep",UIMax);
			p->Annotate("UIWidget",UIStep);			
			//Replace daeEH::Verbose with the code that generates the UI, remember the UI needs			
			if("Slider"==UIWidget)
			daeEH::Verbose<<"Parameter "<<name<<" needs a slider named "<<UIName<<" going from "<<UIMin<<" to "<<UIMax<<" with step "<<UIStep;
			if("Color"==UIWidget)
			daeEH::Verbose<<"Parameter "<<name<<" needs a color picker named "<<UIName;			
		}
	}
}*/
