/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

//This file was made separate from Crt/CrtRender.h because Visual Studio
//aggressively rebuilds the EXE if any of the static libraries are built.
//That said, it doesn't seem to help. Doing 'Project Only->Link only' is
//the only thing that seems to work. I have that bound to a mouse button.

#ifndef __viewer_base_pch_h__
#define __viewer_base_pch_h__

#include <fstream> 

//viewer_HTTP prerequisites.
#ifndef _WIN32
#include <unistd.h> //close
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> //???
#include <netinet/tcp.h> //IPPROTO_TCP
#include <netdb.h>  //getaddrinfo
#endif			  

#ifdef _WIN32 
#define NOMINMAX
#include <Winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
//First ^ Second v (WTF!?)
#include <windows.h> 
#pragma comment(lib,"winmm.lib")
//SetWindowSubclass
#include <commctrl.h> 
#pragma comment(lib,"Comctl32.lib")
#endif

#define FREEGLUT_STATIC

//IMPORTANT: NEED TO INCLUDE FX.pch.h AT A MINIMUM TO
//RESOLVE ISSUES WITH glext.h BEFORE GLUT IS INCLUDED.
#include "CrtScene.h"
#include "CrtRender.h"

//Need to predeclare something?
//static void InitSceneEffects();
static int COLLADA_viewer_main(int,char**,const char*default_dae);
static void COLLADA_viewer_main2(const char*,const void*_=nullptr);  
	  
#endif //__viewer_base_pch_h__
