/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#include <time.h>  
#include <fstream> 

//These are not really used.
#include "CrtScene.h"
#include "CrtEffect.h"

using namespace COLLADA;

//demo.dae floods the console.
enum{ Silence_console_if_DEBUG=0 };

//SCHEDULED FOR REMOVAL
COLLADA_(namespace)
{
	//There's really no reason to 
	//define this here versus the
	//RT static library. It's not
	//like there is any advantage.
	namespace RT
	{
		::COLLADA::RT::Frame Main;
	}
}
static void COLLADA_viewer_GLUT_atexit()
{
	RT::Main._Destroy();
	//GDI segfaults with some DAE samples.
	#if defined(_WIN32) && defined(_DEBUG)
	TerminateProcess(GetCurrentProcess(),0);
	#endif
}

//!!!GAC for demo of how to hookup the UI to a param
static CGparameter amplitudeGlobalParameter = 0;

static bool fullscreen = false;
static bool togglewireframe = false;
static bool togglelighting = true;
static int togglecullingface = 0;
				
//Main Render
static void DrawGLScene()
{
	//Clear The Screen And The Depth Buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	RT::Main.Refresh();
} 
//GL Setup (for some reason PS3_main uses 0,0,1,0.5)
static void InitGL(float r=0.9f, float g=0.9f, float b=0.9f, float a=1)
{
	//Assuming COLLADA wants behavior more like this.
	//Technically profile_COMMON should use technical shaders.
	//
	//glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SINGLE_COLOR);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);

	glShadeModel(GL_SMOOTH);
	glClearColor(r,g,b,a); glClearDepth(1);	
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); //LEQUAL?
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE); //"inverse transpose" needs post-normalization.

	//Sometimes models present incorrectly because back-faces obscure them.
	assert(0==togglecullingface);
	glEnable(GL_CULL_FACE); glCullFace(GL_BACK); //0
}
//Resize And Initialize The GL Window
static void ResizeGLScreen(int width, int height)
{
	RT::Main.Width = width; RT::Main.Height = height;
}

//GLUT can now emulate a wheel by draggin the middle
//button along the vertical dimension.
static float MouseWheelSpeed = 0.02f/1.5f;
static float MouseRotateSpeed = 0.5f;
static float MouseTranslateSpeed = 0.0025f;
static float KeyboardRotateSpeed = 5;
static float KeyboardTranslateSpeed = 10;
static void AdjustUISpeed(float x)
{
	if(x>1&&MouseRotateSpeed>=1.25f
	 ||x<1&&MouseRotateSpeed<=0.25f) return;

	//Dampening rotation. Increasing translation.
	float d = (x-1)/2; float rx = x-d; if(d>0) x+=d;
	KeyboardRotateSpeed*=rx; KeyboardTranslateSpeed*=x;
	MouseRotateSpeed*=rx; MouseTranslateSpeed*=rx; MouseWheelSpeed*=x;
}

static int Xpos = 0, Ypos = 0;
static int Xsize = 640, Ysize = 480;
static void ProcessInput(unsigned char ASCII)
{			   
	switch(ASCII)
	{
	case '\t':

		RT::Main.SetNextCamera();
		RT::Main.Center(); 
		break;

	case 'c': case 'C': 
		
		RT::Main.Center(); 
		break;

	case 'l': case 'L':
		
		if(togglelighting=!togglelighting)
		glEnable(GL_LIGHTING);		
		else
		glDisable(GL_LIGHTING);		
		break;

	case 'm': case 'M':

		AdjustUISpeed(1.25f);
		break;

	case 'n': case 'N':

		AdjustUISpeed(0.75f);
		break;

	case 'o': case 'O':

		RT::Main.AnimationOn = !RT::Main.AnimationOn;
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

	case 'e': case 'E':

		if(amplitudeGlobalParameter)
		{
			float value;
			cgGetParameterValuefc(amplitudeGlobalParameter,1,&value);
			value+=0.1f;
			cgSetParameter1f(amplitudeGlobalParameter,value);			
		}
		break;
	
	case 'r': case 'R':

		if(amplitudeGlobalParameter)
		{
			float value;
			cgGetParameterValuefc(amplitudeGlobalParameter,1,&value);
			value-=0.1f;
			cgSetParameter1f(amplitudeGlobalParameter,value);
		}
		break;		
		
	case 'f': case 'F':

		if(togglecullingface==0) //turn it front
		{ 
			togglecullingface = 1; glEnable(GL_CULL_FACE); glCullFace(GL_FRONT);			
		}
		else if(togglecullingface==1) //turn it both
		{ 
			togglecullingface = 2; glDisable(GL_CULL_FACE);			
		}
		else //turn it back
		{ 
			togglecullingface = 0; glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
		}
		break;

	case 'g': case 'G':

		RT::Main.ShowGeometry = !RT::Main.ShowGeometry;
		break;	

	case 'h': case 'H':

		RT::Main.ShowHierarchy = !RT::Main.ShowHierarchy;
		break;

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
static void glutSpecialFunc_callback(int key, int, int)
{
	switch(key)
	{
	case GLUT_KEY_F11:
	{
		if(fullscreen=!fullscreen)
		{
			/* Save parameters */
			Xpos = glutGet(GLUT_WINDOW_X);
			Ypos = glutGet(GLUT_WINDOW_Y);
			Xsize = glutGet(GLUT_WINDOW_WIDTH);
			Ysize = glutGet(GLUT_WINDOW_HEIGHT);
			glutFullScreen();/* Go to full screen */
		}
		else
		{
			/* Restore parameters */
			glutReshapeWindow(Xsize,Ysize);
			glutPositionWindow(Xpos,Ypos);
			glutPostRedisplay();
		}
		break;
	}
	case GLUT_KEY_LEFT: ProcessInput('A'); break;
	case GLUT_KEY_RIGHT: ProcessInput('D'); break;
	case GLUT_KEY_UP: ProcessInput(' '); break;
	case GLUT_KEY_DOWN: ProcessInput('X'); break;

	default: daeEH::Warning<<"unused (special) key : "<<key;
	}
}

static int lastx = 0;
static int lasty = 0;
static int mouseWheel = 0;
static bool mouseDown[3] = {};
static void toggleMouse(unsigned int button, bool state)
{
	if(button<3) mouseDown[button] = state; 
	else assert(0); //FreeGLUT?
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

#ifdef GLUT_API_VERSION
static void glutDisplayFunc_callback()
{
	DrawGLScene(); glutSwapBuffers();
}
static void glutReshapeFunc_callback(int width, int height)
{
	ResizeGLScreen(width,height); 
}
static void glutKeyboardFunc_callback(unsigned char ASCII, int, int)
{			   
	ProcessInput(ASCII);
}
static void glutMouseFunc_callback(int button, int state, int x, int y)
{
	toggleMouse(button,state==GLUT_DOWN); assert(button<3); //Wheel support?
}
static void glutMotionFunc_callback(int x, int y)
{
	moveMouseTo(x,y);
}
static bool CreateGLUTWindow(int LArgC, char **LArgV, const char *title, int width, int height)
{
	glutInit(&LArgC,LArgV);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_DEPTH|GLUT_RGB);	
	
	int cx = (glutGet(GLUT_SCREEN_WIDTH)-width)/2;
	int cy = (glutGet(GLUT_SCREEN_HEIGHT)-height)/2;
	glutInitWindowPosition(cx,cy);

	glutCreateWindow(title);
	InitGL();

	glutIdleFunc(glutDisplayFunc_callback);
	glutDisplayFunc(glutDisplayFunc_callback);
	glutKeyboardFunc(glutKeyboardFunc_callback);
	glutSpecialFunc(glutSpecialFunc_callback); 
	glutMotionFunc(glutMotionFunc_callback); glutPassiveMotionFunc(glutMotionFunc_callback);
	glutMouseFunc(glutMouseFunc_callback);
	glutReshapeFunc(glutReshapeFunc_callback); glutReshapeWindow(width,height); return true;
}
#endif //GLUT_API_VERSION
//The old Windows_wgl.inl defines these
//The PS3 client leaves them as nullptr
static bool (*CreateGLWindow)(int,char**,const char*,int,int) = 
#ifdef GLUT_API_VERSION
CreateGLUTWindow
#else 
nullptr
#endif
;
static void (__stdcall*COLLADA_viewer_main_loop)() = 
#ifdef GLUT_API_VERSION
glutMainLoop
#else
nullptr
#endif
;

//This was separately implemented by the Windows/Linux/OSX branches 
//It gets amplitudeGlobalParameter. Not clear it does anything else
static void InitSceneEffects()
{
	//This block of code shows how to enumerate all the effects, get their parameters and then
	//get their UI information.
	const RT::DBase *scene = RT::Main.Data;
	if(scene==nullptr) return;

	//Get the scene and setup to iterate over all the effects stored in the FX::Loader
	for(size_t i=0;i<scene->Effects.size();i++)
	{
		FX::Effect *fx = scene->Effects[i]->COLLADA_FX;
		if(fx==nullptr) continue;

		//This is the effect name you would use in a UI
		daeEH::Verbose<<"Effect name "<<scene->Effects[i]->Id;
		for(CGparameter p=cgGetFirstEffectParameter(fx->Cg);p!=nullptr;p=cgGetNextParameter(p))
		{
			//This is the parameter name you would use in the UI
			daeName name = cgGetParameterName(p);
			//This is for the example of how to tweek a parameter (doesn't work yet)
			if(name=="Amplitude")
			{
				//Capture the parameter and save it in a global, in a GUI you would
				//save this handle in the widget so it would know what to tweek.
				amplitudeGlobalParameter = p;
			}

			//(Cg doesn't have an API for multivalue annotation. CgFX doesn't seem to allow it.)			
			CGannotation q; int one = 1;

			#if 0			
			//This is here for debugging, it iterates over all the annotations and prints them out
			//so you can see what's in them.  Normally this code will be turned off.
			daeEH::Verbose<<"Parameter name "<<name;
			for(q=cgGetFirstParameterAnnotation(p);q!=nullptr;q=cgGetNextAnnotation(q))
			{				
				daeEH::Verbose<<"Annotation: "<<cgGetAnnotationName(q);
				switch(cgGetAnnotationType(anno))
				{
				case: CG_STRING
				daeEH::Verbose<<"value: "<<cgGetStringAnnotationValue(q);
				break;				
				case CG_FLOAT:
				daeEH::Verbose<<"value: "<<cgGetFloatAnnotationValues(q,&one)[0]; 
				break;
				default: assert(0); //daeEH::Verbose<<"";
				}
			}
			#endif
			
			//This code currently only collects the annotation values for defining sliders and color pickers.
			//UIName is the name to attach to the widget
			//UIWidget is the widget type			
			daeName UIName,UIWidget;
			//UIMin is the minimum value for a slider widget
			//UIMax is the maximum value for a slider widget
			//UIStep is the step (minimum change) for a slider widget
			float UIMin = -99999.0f, UIMax = 99999.0f, UIStep = 0;
			//Iterate over all the annotations
			for(q=cgGetFirstParameterAnnotation(p);q!=nullptr;q=cgGetNextAnnotation(q))
			{
				daeName label = cgGetAnnotationName(q);
				if("UIWidget"==label)
				UIWidget = cgGetStringAnnotationValue(q);
				if("UIName"==label)
				UIName = cgGetStringAnnotationValue(q);				
				if("UIMin"==label)
				UIMin = cgGetFloatAnnotationValues(q,&one)[0];				
				if("UIMax"==label)
				UIMax = cgGetFloatAnnotationValues(q,&one)[0];				
				if("UIStep"==label)
				UIStep = cgGetFloatAnnotationValues(q,&one)[0];				
			}			
			//Replace daeEH::Verbose with the code that generates the UI, remember the UI needs			
			if("Slider"==UIWidget)
			daeEH::Verbose<<"Parameter "<<name<<" needs a slider named "<<UIName<<" going from "<<UIMin<<" to "<<UIMax<<" with step "<<UIStep;
			if("Color"==UIWidget)
			daeEH::Verbose<<"Parameter "<<name<<" needs a color picker named "<<UIName;			
		}
	}
}

static int COLLADA_viewer_main(int argc, char **argv, const char *default_dae)
{	
	//THIS MUST BE DONE BEFORE RT::Main.Init() OR Cg REPORTS "NO PROFILE."
	//Create an OpenGL Window
	if(!CreateGLWindow(argc,argv,"ColladaDOM 3 Reference Viewer",Xsize,Ysize))
	return 0;	

	RT::Main.Init();
					
	//Set the default screen size
	RT::Main.Width = Xsize; RT::Main.Height = Ysize;
	


	//TESTING
	//TESTING
	//TESTING
	//TESTING
	//RT::Main.LoadImages = false;
	//RT::Main.LoadEffects = false;
	//RT::Main.LoadAnimations = false;



	//These are from WinMain. They may not be portable?
	time_t seconds = time(nullptr);
	clock_t clocka = clock();
	{
		//Load the file name provided on the command line
		const char *dae = argc>1&&argv[1]?argv[1]:default_dae;
		daeEH::Verbose<<
		(dae==default_dae?"Loading default document \"":"Loading \"")		   
		<<dae<<"\"...";			
		if(!RT::Main.Load(dae))
		{
			//Let console print any errors.
			//exit(0); //!!!!!!!!!!
		}
	}
	daeEH::Verbose<<"LOAD TIME OF "<<RT::Main.URL;
	daeEH::Verbose<<"IS "<<int(time(nullptr)-seconds)<<" SECONDS";
	daeEH::Verbose<<"IS "<<int(clock()-clocka)<<" CLOCK TICKS\n\n"; //sic

	//This block of code shows how to enumerate all the effects, get their parameters and then
	//get their UI information.
	InitSceneEffects();

	//GLUT's main loop calls exit() to exit.
	//This is so RT::Main::DOM is cleared before
	//Windows can destroy the schema metadata globals.
	//Force all schemas to initialize.
	extern daeMeta *InitSchemas(int=0);
	InitSchemas(); //daeGetModel<Collada05::COLLADA>();
	atexit(COLLADA_viewer_GLUT_atexit); 

	COLLADA_viewer_main_loop(); //glutMainLoop();
		
	return 0;
}			

//TEMPORARY //TEMPORARY //TEMPORARY

struct TestIO : daeIO //QUICK & DIRTY
{	
	#ifdef _WIN32
	typedef wchar_t Maxpath[MAX_PATH];
	#else
	typedef char Maxpath[260*4*(sizeof(char)==1)];
	#endif
		
	virtual daeOK getError()
	{
		return OK;  
	}
	virtual size_t getLock()
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
					r = CRT.r->fopen(maxpath,L"rb",_SH_DENYWR);
					#else
					r = CRT.r->fopen(maxpath,"rb");
					#endif
				}
				if(r!=nullptr)
				{
					i = CRT.r->stat(r).size;
				}
				if(i>0) lock = i; else OK = DAE_ERROR;
			}
			if(!O.getRequest().isEmptyRequest())
			{
				if(_file_protocol(maxpath,O.getRequest().remoteURI))
				{
					#ifdef _WIN32
					w = CRT.w->fopen(maxpath,L"wb",_SH_DENYRW);
					#else
					w = CRT.w->fopen(maxpath,"wb");
					#endif
				}
				if(w==nullptr) OK = DAE_ERROR; 
			}
		}
		return lock;
	}
	virtual daeOK readIn(void *in, size_t chars)
	{
		if(1!=CRT.r->fread(in,chars,1,r)&&chars!=0) OK = DAE_ERROR; return OK;
	}
	virtual daeOK writeOut(const void *out, size_t chars)
	{
		if(1!=CRT.w->fwrite(out,chars,1,w)&&chars!=0) OK = DAE_ERROR; return OK;
	}
	virtual FILE *getReadFILE(int=0)
	{
		if(r==nullptr) getLock(); return r; 
	}
	virtual FILE *getWriteFILE(int=0)
	{
		if(w==nullptr) getLock(); return w; 
	}

	//This is not the normal way to go about this.
	//It's just easy to set up for basic file I/O.
	struct{ const struct daeCRT::FILE *r,*w; }CRT;
	daeIOPlugin &I,&O; daeOK OK; FILE *r,*w; size_t lock;
	TestIO(std::pair<daeIOPlugin*,daeIOPlugin*> IO)
	:I(*IO.first),O(*IO.second),r(),w(),lock(-1)
	{
		CRT.r = &I.getCRT_default().FILE; 
		CRT.w = &O.getCRT_default().FILE;
	}
	~TestIO()
	{
		if(r!=nullptr) CRT.r->fclose(r); 
		if(w!=nullptr) CRT.w->fclose(w); 
	}
	 		
	static bool _file_protocol(Maxpath &maxpath, const daeURI *URI)
	{							   
		if(URI!=nullptr&&"file"==URI->getURI_protocol())
		{
			#ifdef _WIN32
			daeRefView v = URI->getURI_upto<'?'>(); int UNC = v[7]=='/'?8:5;
			maxpath[MultiByteToWideChar(CP_UTF8,0,v+UNC,v.extent-UNC,maxpath,MAX_PATH-1)] = 0;
			#else
			if(URI->size()<sizeof(maxpath)) 
			memcpy(maxpath,URI->data(),URI->size()+1);
			else return false;
			#endif
			return true;
		}assert(URI->empty()); return false; 
	}
};
static struct TestPlatform : daePlatform //SINGLETON
{
	std::vector<TestIO> IO_stack;

	virtual const daeURI &getDefaultBaseURI(daeURI &URI)
	{
		TestIO::Maxpath maxpath;
		char UTF[4*MAX_PATH+8] = "file:///";		
		#ifdef _WIN32
		GetCurrentDirectoryW(MAX_PATH,maxpath);
		int UNC = maxpath[0]=='//'?5:8;
		int len = UNC+WideCharToMultiByte(CP_UTF8,0,maxpath,-1,UTF+UNC,sizeof(UTF)-8,0,0);
		#else
		if(!getcwd(UTF+7,sizeof(UTF)-7))
		return URI;
		int len = strlen(UTF);
		#endif
		//If the directory URI doesn't end in a slash it looks like a filename?
		switch(UTF[len-2]) 
		{
		case '\\': case '/': break; default: UTF[len-1] = '/'; UTF[len] = '\0';
		}
		URI.setURI(UTF); return URI;
	}
	virtual daeOK resolveURI(daeURI &URI, const daeDOM &DOM)
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
				char cat[4*MAX_PATH+8]; strcpy(cat,base);
				memcpy(cat+strlen(base),d,sizeof(cat)-strlen(base));
				cat[sizeof(cat)-1] = '\0';
				URI.setURI(cat);
			}
		}
		//daeEH::Verbose<<"Resolving URI: "<<URI.getURI();
		daeOK OK = URI.resolve_RFC3986(DOM);
		//daeEH::Verbose<<"Resolved URI: "<<URI.getURI();
		assert(URI.getURI_protocol().size()>=4||URI.empty()); return OK;
	}
	virtual daeOK openURI(const daeIORequest &req, daeIOPlugin *I, daeURIRef &URI)
	{
		daeDocRoot<> doc; req.resolve();
		if(!req.localURI->getURI_extensionIs("dae"))
		{
			daeEH::Error<<"Unsupported file extension "<<req.localURI->getURI_extension();
			req.unfulfillRequest(doc); return doc;
		}
		#ifdef NDEBUG
		#error WORK IN PROGRESS
		#endif
		//TODO: Must scan for version="1.5.0" and switch to 8.
		extern daeMeta *InitSchemas(int);
		daeMeta *meta = InitSchemas(Peek_xmlns(req.localURI)=="http://www.collada.org/2005/11/COLLADASchema"?5:8);
		req.fulfillRequestI(meta,I,doc);
		if(doc==DAE_OK) URI = &doc->getDocURI(); else assert(0); return doc; 
	}
	virtual daeIO *openIO(daeIOPlugin &I, daeIOPlugin &O)
	{
		IO_stack.emplace_back(std::make_pair(&I,&O)); return &IO_stack.back();
	}
	virtual void closeIO(daeIO *IO)
	{
		assert(IO==&IO_stack.back()); IO_stack.pop_back();
	}		
	virtual int getLegacyProfile()
	{
		return LEGACY_LIBXML|
		LEGACY_SIDREF_RESOLVER|LEGACY_IDREF_RESOLVER|LEGACY_URI_RESOLVER;
	}

	daeName Peek_xmlns(const daeURI *URI)
	{
		static std::string out; out.clear();

		TestIO::Maxpath path; 
		if(!TestIO::_file_protocol(path,URI)) return out;

		std::ifstream s(path);
		char buf[4096]; s.read(buf,4096);
		out.assign(buf,4096);
		out.erase(0,out.find("xmlns=\"")+7); //GOOD ENOUGH?
		out.erase(out.find('"'),-1); 		
		return out; //NO need to normalize. Not a URI per se.
	}

	TestPlatform(){ daeDOM::setGlobalPlatform(this); }

}TestPlatform; //SINGLETON

//EXPERIMENTAL demo.dae floods the console.
#ifdef _DEBUG 
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
