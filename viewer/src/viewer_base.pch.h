/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include "CrtRender.h" //PCH

//This file was made separate from Crt/CrtRender.h because Visual Studio
//aggressively rebuilds the EXE if any of the static libraries are built.
//That said, it doesn't seem to help. Doing 'Project Only->Link only' is
//the only thing that seems to work. I have that bound to a mouse button.