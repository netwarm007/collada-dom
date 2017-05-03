/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

//2017: For whatever reason the Windows version of the viewer
//was stubbornly holding onto non-GLUT code, while everything  
//else is using GLUT. It depended on the Cg package, which in
//its day may or may not have included a GLUT. There are more
//than one implementation of GLUT for Windows today that this
//viewer can use, as long as it is using GLUT.

#ifdef GLUT_API_VERSION
#error This file implements replacement logic for GLUT.
#endif

static HDC hDC = nullptr; //GDI Device Context	
static HGLRC hRC = nullptr; //Rendering Context
static HWND hWnd = nullptr; //Window Handle	
static HINSTANCE hInstance; //Application Instance 	

static bool active = true;
static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
static void DestroyWindow();

//Used to track which keys are held down, the index is the windows
static bool keys[256];
//Call ProcessInput once per frame to process input keys
static void ProcessKeys()
{
	if(keys[VK_F1])
	{
		RT::Main.Destroy();
		DestroyWindow();
		fullscreen = !fullscreen;
		if(!CreateWGLWindow(0,0,"Collada Viewer for PC",RT::Main.Width,RT::Main.Height))
		exit(1);		
		if(!RT::Main.Reload())
		exit(0);
		keys[VK_F1] = false; //Prevent repetition? As below.
	}	

	for(char i='A';i<='Z';i++) if(keys[i])
	{			
		//These keys that do a function as long as they are held down, so we don't clear "keys".
		//Remember to scale these functions by time!
		switch(i)
		{
		case 'W': case 'A': case 'S': case 'D':  
		case 'X': case ' ': break; default: keys[i] = false;
		}
		ProcessInput(i);
	}
	if(keys[' ']) ProcessInput(' ');
	if(keys['\t']){ ProcessInput('\t'); keys['\t'] = false; }
}
static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_ACTIVATE:
	
		active = !HIWORD(wParam);
		return 0;

	case WM_SYSCOMMAND:
	
		switch(wParam)
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER: return 0;
		}
		break;
	
	case WM_CLOSE:
	
		PostQuitMessage(0);
		return 0;
	
	case WM_KEYDOWN:
	
		//We only want to know which keys are down, so if this was an auto-repeat, ignore it
		if(!(HIWORD(lParam)&KF_REPEAT))
		{
			//Remember which keys are being held down
			keys[wParam] = true;
		}
		return 0;	

	case WM_KEYUP:
	
		keys[wParam] = false;
		return 0;
	
	case WM_SIZE:
	
		ResizeGLScreen(LOWORD(lParam),HIWORD(lParam));
		return 0;
	
	case WM_MOUSEWHEEL:
	{
		//2017: Adding /WHEEL_DELTA. Was this never tested???
		float delta = (short)HIWORD(wParam);
		if(mouseDown[2]) delta/=WHEEL_DELTA/10;
		RT::Main.ZoomIn(-delta*MouseWheelSpeed);		
		return 0;
	}	
	case WM_LBUTTONUP: toggleMouse(0,false); return 0;
	case WM_KBUTTONDOWN: toggleMouse(0,true); return 0;
	case WM_MBUTTONUP: toggleMouse(1,false); return 0;
	case WM_MBUTTONDOWN: toggleMouse(1,true); return 0;
	case WM_RBUTTONUP: toggleMouse(2,false); return 0;
	case WM_RBUTTONDOWN: toggleMouse(2,true); return 0;	
	case WM_MOUSEMOVE:
	
		moveMouseTo((short)LOWORD(lParam),(short)HIWORD(lParam));
		return 0;
	}
	//Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

static bool AbortGLWindow(const char *why)
{
	MessageBox(nullptr,why,"ERROR",MB_OK|MB_ICONEXCLAMATION);
	DestroyWindow(); return false;
}
//----------------------------------------------------------------------------------------------------
//Here Creating the Window based on the Width and Height parameters 
static bool CreateWGLWindow(int, char**, const char *title, int width, int height)
{
	const unsigned char depth = 32;

	GLuint PixelFormat;
	WNDCLASS wc;
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT WindowRect;
	WindowRect.left = 0;
	WindowRect.right = (long)width;
	WindowRect.top = 0;
	WindowRect.bottom = (long)height;

	hInstance = GetModuleHandle(nullptr);
	wc.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(nullptr,IDI_WINLOGO);
	wc.hCursor = LoadCursor(nullptr,IDC_ARROW);
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = "OpenGL";

	if(!RegisterClass(&wc))
	{
		MessageBox(nullptr,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	if(fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = depth;
		dmScreenSettings.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		//Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if(ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			//If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if(!MessageBox(nullptr,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","COLLADA_RT GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				//Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(nullptr,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return false;
			}
			else fullscreen = false;
		}
	}

	if(fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
		ShowCursor(0);
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW|WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(&WindowRect,dwStyle,0,dwExStyle);

	//Create The Window

	if(!(hWnd=CreateWindowEx(dwExStyle,
	"OpenGL",
	title,
	dwStyle|
	WS_CLIPSIBLINGS|
	WS_CLIPCHILDREN,
	0,0,
	WindowRect.right-WindowRect.left,
	WindowRect.bottom-WindowRect.top,
	nullptr,
	nullptr,
	hInstance,
	nullptr))
	return AbortGLWindow("Window Creation Error.","ERROR");
	static PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW|
		PFD_SUPPORT_OPENGL|
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		depth,
		0,0,0,0,0,0,
		0,
		0,
		0,
		0,0,0,0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0,0,0
	};
	if(!(hDC=GetDC(hWnd))
	return AbortGLWindow("Can't Create A GL Device Context.");
	if(!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))
	return AbortGLWindow("Can't Find A Suitable PixelFormat.");
	if(!SetPixelFormat(hDC,PixelFormat,&pfd))
	return AbortGLWindow("Can't Set The PixelFormat.","ERROR");
	if(!(hRC=wglCreateContext(hDC)))
	return AbortGLWindow("Can't Create A GL Rendering Context.");
	if(!wglMakeCurrent(hDC,hRC))
	return AbortGLWindow("Can't Activate The GL Rendering Context.");

	ShowWindow(hWnd,SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	ResizeGLScreen(width,height);

	InitGL(); /*2017
	if(!InitGL())
	{
		DestroyWindow();
		MessageBox(nullptr,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}*/

	return true;
}
static void DestroyWindow()
{
	if(fullscreen)
	{
		ChangeDisplaySettings(nullptr,0);
		ShowCursor(1);
	} 
	if(hRC)
	{
		if(!wglMakeCurrent(nullptr,nullptr))
		MessageBox(nullptr,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK|MB_ICONINFORMATION);
		if(!wglDeleteContext(hRC))
		MessageBox(nullptr,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK|MB_ICONINFORMATION);		
		hRC = nullptr;
	}
	if(hDC&&!ReleaseDC(hWnd,hDC))
	{
		MessageBox(nullptr,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK|MB_ICONINFORMATION);
		hDC = nullptr;
	}
	if(hWnd&&!DestroyWindow(hWnd))
	{
		MessageBox(nullptr,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK|MB_ICONINFORMATION);
		hWnd = nullptr;
	}
	if(!UnregisterClass("OpenGL",hInstance))
	{
		MessageBox(nullptr,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK|MB_ICONINFORMATION);
		hInstance = nullptr;
	}
}

static MSG msg;
static WPARAM &exit_status = msg.wParam;
static void Windows_wgl_main_loop()
{
	bool done = false; while(!done)
	{
		if(PeekMessage(&msg,nullptr,0,0,PM_REMOVE))
		{
			if(msg.message!=WM_QUIT)
			{
				TranslateMessage(&msg); DispatchMessage(&msg);
			}
			else done = true;
		}
		else //Draw The Scene. Watch For ESC Key And Quit Messages From DrawGLScene()
		{
			//Active? Was There A Quit Received?
			if((active&&!DrawGLScene())||keys[VK_ESCAPE]) 
			{
				done = true; 
				
				//2017: Adding for sanity.
				//THIS WHOLE APPROACH IS BAD PRACTICE.
				exit_status = 0;
			}
			else
			{
				SwapBuffers(hDC); ProcessKeys();
			}
		}
	}
	DestroyWindow();
}