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

//SetWindowSubclass
#ifdef _WIN32 
#define NOMINMAX
#include <windows.h> 
#include <commctrl.h> 
#pragma comment(lib,"Comctl32.lib")
#endif

#include "CrtRender.h"
#define FREEGLUT_STATIC