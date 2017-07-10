/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include "viewer_base.pch.h" //PCH 

#include <GLUT/glut.h>

#include "../../src/viewer_base.inl"

int main(int argc, char **argv)
{
	//dominoes.dae is a physics demo.
	return COLLADA_viewer_main(argc,argv,"demo.dae"); //dominoes.dae
}
