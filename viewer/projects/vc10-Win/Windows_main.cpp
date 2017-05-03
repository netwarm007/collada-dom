/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include "viewer_base.pch.h" //PCH 

#include <zmouse.h>
#if _DEBUG //#define HEAP_DEBUG will use the older _CrtSetDbgFlag params
#include <crtdbg.h>
#endif
	  
//2017 adds poorly worded NO_GLUT macro
#ifdef NO_GLUT
#include <gl\gl.h>   
#include <gl\glu.h>   
#else
#include <gl\glut.h>   
#endif

#include "../../src/viewer_base.inl"

//2017: This really should be obsolete. Even the old Cg package includes
//an implementation of GLUT on Windows.
#ifndef GLUT_API_VERSION
#include "Windows_wgl.inl"
#else
struct MouseWheel_etc
{
	HHOOK hook;	
	static LRESULT CALLBACK proc(int code, WPARAM w, LPARAM l)
	{
		if(w==WM_MOUSEWHEEL)
		{
			short delta =
			GET_WHEEL_DELTA_WPARAM(((MSLLHOOKSTRUCT*)l)->mouseData);
			if(mouseDown[2]) delta/=WHEEL_DELTA/10;
			RT::Main.ZoomIn(-delta*MouseWheelSpeed);
		}		
		return CallNextHookEx(Windows_for_GLUT.hook,code,w,l);
	}
	MouseWheel_etc():hook(SetWindowsHookEx(WH_MOUSE_LL,proc,0,0)){}
}Windows_for_GLUT;
#endif

//----------------------------------------------------------------------------------------------------
//Standard windows mainline, this is the program entry point
//
int main(int argc, char *argv[])
{	
	#ifndef GLUT_API_VERSION
	//#include "Windows_wgl.inl"
	CreateGLWindow = CreateWGLWindow;
	COLLADA_viewer_main_loop = Windows_wgl_main_loop; 
	#else
	int exit_status;
	#endif
	exit_status = 0;
	//cage.dae didn't even have its textures set up right??
	exit_status|=COLLADA_viewer_main(argc,argv,"demo.dae"); //cage.dae	 
	return (int)exit_status;
}
int __stdcall wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	#ifdef _DEBUG //compiler
	_set_error_mode(_OUT_TO_MSGBOX); 
	//Turns on windows heap debugging
	#if HEAP_DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_CHECK_CRT_DF/*|_CRTDBG_DELAY_FREE_MEM_DF*/);
	#else
	//_CRTDBG_LEAK_CHECK_DF floods output on debugger termination
	_crtDbgFlag = _CRTDBG_ALLOC_MEM_DF; //|_CRTDBG_LEAK_CHECK_DF;
	//_crtDbgFlag|=_CRTDBG_CHECK_ALWAYS_DF; //careful
	#endif
	#endif

	//Convert UTF16_LE to UTF8.
	int argc = 0;
	wchar_t **argw = CommandLineToArgvW(GetCommandLineW(),&argc);
	char **argv = new char*[argc];
	std::vector<char> buffer(MAX_PATH);
	for(int i=0;i<argc;argv[i++]=_strdup(buffer.data())) top:
	{
		WideCharToMultiByte(CP_UTF8,0,argw[i],-1,buffer.data(),buffer.size(),0,0);
		if(GetLastError()!=ERROR_INSUFFICIENT_BUFFER) continue;
		buffer.resize(2*buffer.size()); goto top;
	}
	//Leave this even though this happens to be a console app.
	AttachConsole(GetCurrentProcessId()); 
	UINT cp = GetConsoleOutputCP(); SetConsoleOutputCP(65001);	
	int exit_code = main(argc,argv); SetConsoleOutputCP(cp); return exit_code;
} 