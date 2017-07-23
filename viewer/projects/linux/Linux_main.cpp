/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include "viewer_base.pch.h" //PCH 

#include <GL/glut.h>
#ifdef FREEGLUT
#include <GL/freeglut.h>
#endif

#include "../../src/viewer_base.inl"

int main(int argc, char **argv)
{
	//cage.dae didn't even have its textures set up right??
	return COLLADA_viewer_main(argc,argv,"demo.dae"); //cage.dae
}					   	

