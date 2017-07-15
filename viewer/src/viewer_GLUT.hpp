/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include "viewer_base.pch.h"
																 
//This file is being inlined into viewer_base.inl. It exists only
//to divide its logical file up into digestible pieces.

//This is for IntelliSense like semantic highlighting products only.
#ifndef GLUT_API_VERSION
#include "../../../viewer\external-libs/freeglut/include/GL/freeglut.h"
#endif

static void COLLADA_viewer_GLUT_atexit()
{
	RT::Main._Destroy();
	//GDI segfaults with some DAE samples.
	#if defined(_WIN32) && defined(_DEBUG)
	TerminateProcess(GetCurrentProcess(),0);
	#endif
}

static void glutWindowStatusFunc_callback(int state)
{
	visible = state!=GLUT_HIDDEN&&state!=GLUT_FULLY_COVERED;
}

static void glutSpecialFunc_callback(int key, int, int)
{
	switch(key)
	{	
	case GLUT_KEY_F11:
	{
		if(fullscreen=!fullscreen)
		{
			//Save parameters
			Xpos = glutGet(GLUT_WINDOW_X);
			Ypos = glutGet(GLUT_WINDOW_Y);
			Xsize = glutGet(GLUT_WINDOW_WIDTH);
			Ysize = glutGet(GLUT_WINDOW_HEIGHT);
			glutFullScreen();
		}
		else //Restore parameters
		{
			glutReshapeWindow(Xsize,Ysize);
			glutPositionWindow(Xpos,Ypos);
			glutPostRedisplay();
		}
		break;
	}
	#ifdef FREEGLUT
	case GLUT_KEY_ALT_L: case GLUT_KEY_ALT_R:
	#endif
	case GLUT_KEY_F6: ProcessInput('\r'); break;
	case GLUT_KEY_LEFT: ProcessInput('A'); break;

	case GLUT_KEY_RIGHT: ProcessInput('D'); break;

	case GLUT_KEY_UP: ProcessInput(' '); break;
	case GLUT_KEY_DOWN: ProcessInput('X'); break;

	default: daeEH::Warning<<"unused (special) key : "<<key;
	}
}

//This is verbatim from the GLUI demo.
void glutIdleFunc_callback()
{	
	//According to the GLUT specification, the current window is 
	//undefined during an idle callback.  So we need to explicitly change
	//it if necessary.
	if(COLLADA_viewer!=glutGetWindow()) 
	glutSetWindow(COLLADA_viewer);  

	if(!visible)
	{
		#ifdef _WIN32
		Sleep(500);
		#else
		usleep(500*1000);
		#endif
		return;
	}

	//Does GLUI not need to do anything inside the idle callback?
	//If not is this even necessary?
	glutPostRedisplay();
}
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
	toggleMouse(button,state==GLUT_DOWN); assert((state&~1)==0);
}
static void glutMotionFunc_callback(int x, int y)
{
	moveMouseTo(x,y);
}
static void glutPassiveMotionFunc_callback(int x, int y)
{
	glutMotionFunc_callback(x,y);
}
static bool CreateGLUTWindow(int LArgC, char **LArgV, const char *title, int width, int height)
{
	#if defined(FREEGLUT) && defined(_DEBUG)
	//fghIsLegacyContextRequested wants OpenGL>2.1??
	glutInitContextVersion(3,0); //First version after 2.1.
	glutInitContextFlags(GLUT_DEBUG);
	#endif

	glutInit(&LArgC,LArgV);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_DEPTH|GLUT_RGB);	
	
	int cx = (glutGet(GLUT_SCREEN_WIDTH)-width)/2;
	int cy = (glutGet(GLUT_SCREEN_HEIGHT)-height)/2;
	glutInitWindowPosition(cx,cy);
	glutInitWindowSize(width,height);
	
	float r = 0.9f; //FreeGLUT background color hack...
	COLLADA_viewer = glutCreateWindow(title);
	{
		//FreeGLUT's windows pops up immediately, and so
		//fill its background so it's less distracting.
		glClearColor(r,r,r,1);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glutSwapBuffers();
	}
	InitGL(r,r,r,1);

	
	#ifdef GLUI_VERSION
	COLLADA::UI.Init(COLLADA_viewer);
	#define _(x) GLUI_Master.set_glut##x##Func(glut##x##Func_callback);
	#else
	#define _(x) glut##x##Func(glut##x##Func_callback);
	#endif	
	_(Idle)_(Display)_(Keyboard)_(Special)_(Reshape)
	_(Mouse)_(Motion)_(PassiveMotion)_(WindowStatus)
	#undef _

	glutShowWindow(); //FreeGLUT feels unresponsive.

	return true;
}

static int COLLADA_viewer_main(int argc, char **argv, const char *default_dae)
{	
	//SCHEDULED FOR REMOVAL
	//HACK: default_dae is most likely in the samples/ folder, which 
	//is unlikely to be the working-directory at this point. An online
	//document is a reliable target. This URL is where binaries are kept.
	//I'd rather set this here than in the Windows/Linx/OSX_main.cpp files.
	default_dae = "http://www.swordofmoonlight.net/holy/example.dae";

	//THIS MUST BE DONE BEFORE RT::Main.Init() OR Cg REPORTS "NO PROFILE."
	//Create an OpenGL Window
	if(!CreateGLUTWindow(argc,argv,"ColladaDOM 3 Reference Viewer",Xsize,Ysize))
	return 0;	

	//cgSetPassState messes up the global state.
	//This is also initialzing the RT component.
	RT::Main.Init(RestoreGL);

	#ifdef GLUI_VERSION	
	UI.site->text_x_offset = 0; //55???				
	UI.site->set_alignment(GLUI_ALIGN_CENTER);		
	#endif

	//Set the default screen size
	ResizeGLScreen(Xsize,Ysize);

	//Load the file name provided on the command line
	COLLADA_viewer_main2(argc>1?argv[argc-1]:default_dae,default_dae);
		  
	//This block of code shows how to enumerate all the effects, get their parameters and then
	//get their UI information.
	//InitSceneEffects();

	//GLUT's main loop calls exit() to exit.
	//This is so RT::Main::DOM is cleared before
	//Windows can destroy the schema metadata globals.
	//Force all schemas to initialize.
	extern daeMeta *InitSchemas(int=0);
	InitSchemas(); //daeGetModel<Collada05::COLLADA>();
	atexit(COLLADA_viewer_GLUT_atexit); 					  

	glutMainLoop(); return 0;
}			
