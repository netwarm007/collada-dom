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
#ifndef NO_GLUT
#if 1 //EXPERIMENTAL
#include <gl/glut.h>   
#else
#include "../../../viewer\external-libs/freeglut/include/GL/freeglut.h"
#endif
#ifndef FREEGLUT
#pragma comment(lib,"glut32.lib")
#ifdef NDEBUG
#pragma comment(lib,"freeglut_static.lib")
#else
#pragma comment(lib,"freeglut_staticd.lib")
#endif
#endif
#endif

#include "../../src/viewer_base.inl"

HWND Window = 0;
BOOL WINAPI Windows_CONSOLE_HandlerRoutine(DWORD dwCtrlType)
{
	switch(dwCtrlType)
	{
	//Reminder: Ctrl+C can't copy because there's not a selection.
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT: //return 0; //Terminates after breaking.
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:

		//HACK: This is happening on another thread that is causing an
		//assert() to be fired on shutdown that locks the debugger/IDE.
		//The assert is because atexit() is never called on the thread.
		SendMessage(Window,WM_DESTROY,0,0);
		return 1;
	}
	return 0;
}

static char *Windows_main_UTF8(const wchar_t *in)
{
	URI_to_ASCII_buf.reserve(128); top:
	char *out = URI_to_ASCII_buf.data();
	int len = WideCharToMultiByte(CP_UTF8,0,in,-1,out,URI_to_ASCII_buf.capacity(),0,0);
	//Error is not set upon success!?
	if(len==0) switch(GetLastError()) 
	{
	case ERROR_INSUFFICIENT_BUFFER: 

		URI_to_ASCII_buf.reserve(2*URI_to_ASCII_buf.capacity()); goto top;

	default: len = 1; 
	}
	out[len] = '\0'; return out;
}

//going behind GLUT to add WM_MOUSEWHEEL
static HHOOK Windows_main_GLUT_hook = 0;	
static LRESULT CALLBACK Windows_main_GLUT(HWND GLUT, UINT msg, WPARAM w, LPARAM l, UINT_PTR, DWORD_PTR)
{
	switch(msg)
	{
 	case WM_MOUSEWHEEL:
	{
		#ifndef FREEGLUT
		short delta =
		GET_WHEEL_DELTA_WPARAM(w);
		if(mouseDown[2]) delta/=WHEEL_DELTA/10;
		RT::Main.ZoomIn(-delta*MouseWheelSpeed);
		#endif
		break;
	}
	//GLUT has a bug when letting up outside its window.
	case WM_CAPTURECHANGED:	
		
		#ifndef FREEGLUT
		if(GLUT!=(HWND)l) 
		mouseDown[0] = mouseDown[1] = mouseDown[2] = false; 
		#endif
		break;

	case WM_DROPFILES:
	{
		//Don't grow stack inside window-procs!
		static wchar_t *buf = new wchar_t[MAX_PATH];
		DragQueryFileW((HDROP)w,0,buf,MAX_PATH);
		COLLADA_viewer_main2(Windows_main_UTF8(buf));
		break;
	}}		
	return DefSubclassProc(GLUT,msg,w,l);
}
static LRESULT CALLBACK Windows_main_GLUT_hook_proc(int code, WPARAM w, LPARAM l)
{
	LRESULT out = CallNextHookEx(Windows_main_GLUT_hook,code,w,l);
	if(code==HCBT_CREATEWND)
	{
		//Might want to do more here later. FreeGLUT doesn't require
		//anything. The CONSOLE handler needs the main window handle.
		Window = (HWND)w;
		DragAcceptFiles(Window,1);		
		SetWindowSubclass(Window,Windows_main_GLUT,0,0);		
		UnhookWindowsHookEx(Windows_main_GLUT_hook);

		//Give the CONSOLE window an icon. Assuming it's unattached!!
		PostMessage(GetConsoleWindow(),WM_SETICON,ICON_BIG,GetClassLong(Window,GCL_HICON));
		PostMessage(GetConsoleWindow(),WM_SETICON,ICON_SMALL,GetClassLong(Window,GCL_HICON));
	}
	return out;
}

//----------------------------------------------------------------------------------------------------
//Standard windows mainline, this is the program entry point
//
int main(int argc, char *argv[])
{	
	Windows_main_GLUT_hook = 
	SetWindowsHookEx(WH_CBT,Windows_main_GLUT_hook_proc,0,GetCurrentThreadId());
	
	int exit_status = COLLADA_viewer_main(argc,argv,"demo.dae");
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

	//Convert UTF16_LE to UTF8 and do CONSOLE stuff before main().
	int argc = 0;
	wchar_t **argw = CommandLineToArgvW(GetCommandLineW(),&argc);
	char **argv = new char*[argc];	
	for(int i=0;i<argc;i++)
	argv[i]=_strdup(Windows_main_UTF8(argw[i]));
	 
	//This is not necessary for CONSOLE applications, but it's better
	//to not be a console application since that tends to close doors.
	if(!AttachConsole(GetCurrentProcessId()))
	{
		AllocConsole(); FILE *C2143;
		freopen_s(&C2143,"CONOUT$","w",stdout);
		freopen_s(&C2143,"CONOUT$","w",stderr);
		//BLACK MAGIC: better to clear these together after reopening.
		clearerr(stdout); clearerr(stderr); 
		//The C-Runtime (CRT) in the DLL doesn't know it's redirected.
		if(Silence_console_if_DEBUG!=1) //If so it's already been set.
		daeErrorHandler::setErrorHandler(new daeStandardErrorHandler);		
	}
	//There are shutdown issues if the CONSOLE is used. Especially if
	//debugging.
	SetConsoleCtrlHandler(Windows_CONSOLE_HandlerRoutine,1);
	UINT cp = GetConsoleOutputCP(); SetConsoleOutputCP(65001);	
	int exit_code = main(argc,argv); SetConsoleOutputCP(cp); return exit_code;
}
