/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#include <GL/glut.h>

#include "../../src/viewer_base.inl"

int main(int argc, char **argv)
{
	//cage.dae didn't even have its textures set up right??
	return COLLADA_viewer_main(argc,argv,"demo.dae"); //cage.dae
}					   	

