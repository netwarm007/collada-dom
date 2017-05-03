/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <RT.pch.h> //PCH

#include "CrtEffect.h"
#include "CrtLight.h"
#include "CrtGeometry.h"
#include "CrtMatrix.h"
#include "CrtNode.h"
#include "CrtRender.h"
#include "CrtTexture.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
  
void RT::Stack::SetMaterial(RT::Material *mat)
{
	RT::Effect *effect = mat->Effect;
	 //In COLLADA 1.4 a material is required to have a reference to an effect
	assert(effect!=nullptr);

	FLOAT shininess = effect->Shininess;
	//if the scale is 0.0 to 1.0, make it 0.0 to 100.0
	if(shininess<1.0) shininess = shininess*128; 

	//	diffuse.a	=	effect->Transparency;
	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,&effect->Emission.f0);
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,&effect->Ambient.f0);
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,&effect->Diffuse.f0);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,&effect->Specular.f0);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shininess);

	//This looped through the textures, but it's 
	//being used for profile_COMMON only and it's
	//SCHEDULED FOR REMOVAL also.
	if(!effect->Textures.empty()&&(1&RT::Main.ShowTextures_Mask))
	{
		int TexId = effect->Textures[0]->TexId;
		if(TexId==0) //HACK: Load missing texture?
		TexId = FX::Loader::GetID_TexId(nullptr);

		glEnable(GL_TEXTURE_2D);
		//GL.ActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D,TexId);		
	}
	else glDisable(GL_TEXTURE_2D); CurrentMaterial = mat;
}

void RT::Stack::ResetCamera()
{
	RT::Camera *c = RT::Main.Camera;

	//Reminder: near/far are macros.
	RT::Float nZ = c->ZNear, fZ = c->ZFar; 
	//HACK
	#ifdef NDEBUG
	#error COLLADA doesn't have an automatic aspect ratio mode.
	#endif
	if(c->Id=="COLLADA_RT_default_camera")
	{
		//Just force Refresh to use Width/Height.
		c->Aspect = (RT::Float)RT::Main.Width/RT::Main.Height;		
		c->Yfov = 36;

		//The default camera doesn't have a set zoom.
		#ifdef NDEBUG
		#error These factors are arbitrarily chosen.
		#endif
		nZ = RT::Main.Zoom/50; fZ = nZ*1000; //RT::Main.Zoom*10; 
	}
	else if(RT::Main.Zoom>0)
	{
		//Would like to adjust near/far to capture the zoom.
		//Historically "zoom" moves the view-volume instead
		//of the projection-matrix. For the way the default
		//camera works, it needs to get away from 0,0,0. It
		//would need some serious thought to change that up.			
		double r = nZ/fZ; fZ+=RT::Main.Zoom; nZ = RT::Float(fZ*r);
	}

	//Get the camera from the instance and set the projection matrix from it
	glMatrixMode(GL_PROJECTION); 
	//SCHEDULED FOR REMOVAL
	//These APIs call glMultMatrix
	glLoadIdentity(); switch(c->Refresh())
	{
	case 2: glOrtho(-c->Xmag,c->Xmag,-c->Ymag,c->Ymag,nZ,fZ); break;
	case 3: gluPerspective(c->Yfov,c->Aspect,nZ,fZ); break;
	}			

	//While not strictly necessary, cameras have aspect ratios, and whatever
	//is outside of the ratio is not part of the intended image. Distortions
	//are a large problem in graphics, and distortions are greatest at edges.
	int w = int(c->Aspect*RT::Main.Height+0.5f);
	if(w>RT::Main.Width)
	{
		int h = int(RT::Main.Width/c->Aspect+0.5f);
		glViewport(0,(RT::Main.Height-h)/2,RT::Main.Width,h);
	}
	else glViewport((RT::Main.Width-w)/2,0,w,RT::Main.Height);

	//REMINDER: SetupRenderingWithShadowMap depends on this being here, even
	//though it's not used by the viewer project or anything for that matter. 
	{
		glMatrixMode(GL_MODELVIEW);		
		//glLightfv uses this and the line
		//drawing code below needs it. The
		//lights are not changed afterward.
		GL::LoadMatrix(_View);
	}
}
		
void RT::Stack::Draw()
{
	//The camera is always updated in case it's animated.
	RT::Main.Matrix(&_View,&RT::Main.Cg._InverseViewMatrix);
	ResetCamera();

	//draw lines to each parent node?
	if(RT::Main.ShowHierarchy)
	{	
		//Draw blue lines.
		GLboolean lit = glIsEnabled(GL_LIGHTING);
		if(lit) glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);		
		glColor3f(1,0.5,1); 
		glLineWidth(1); 
		glBegin(GL_LINES);		
		for(size_t i=1;i<Data.size();i++)
		{
			Data[i].ShowHierarchy_glVertex3d();
			Data[i].Parent->ShowHierarchy_glVertex3d();
		}
		#ifdef _DEBUG
		//This has some huge axes lines.
		glColor3f(0.25,0.25,1); 
		RT::Main.Physics.VisualizeWorld();
		#endif
		glEnd(); glLineWidth(1);
		glEnable(GL_TEXTURE_2D);
		if(lit) glEnable(GL_LIGHTING);
	}		
	if(!RT::Main.ShowGeometry)
	return;

	//reset all lights
	int l=0,lN = 7;
	for(size_t i=0;i<Data.size();i++)
	if(!Data[i].Node->Lights.empty())
	{
		int ll = l; 
		l = std::min(lN,Data[i].Light(l));
		//Enable the light and lighting
		while(ll<l) glEnable(GL_LIGHT0+ll++);
		if(l==lN) break;
	}	
	while(l<lN) glDisable(GL_LIGHT0+l++);
				
	//Are we using shadow maps?
	//!!!GAC Shadow map code may not be functional right now
	if(RT::Main.ShadowMap.Use&&RT::Main.ShadowMap.Initialized)
	{
		//Render the shadow pass
		RT::Main.ShadowMap.SetupRenderingToShadowMap();
		Draw_Triangles();		
		//Now render the scene 
		RT::Main.ShadowMap.SetupRenderingWithShadowMap();
		Draw_Triangles();
	}
	else Draw_Triangles();
}
void RT::Stack::Draw_Triangles()
{	
	RT::Geometry *g = nullptr;

	//HACK: There's not a separate procedure for
	//initializing these vertex-buffers. VBuffer1
	//is cleared when Select() is called.
	if(0==VBuffer1.Capacity)
	{		
		if(DrawData.empty()) return;
		 
		//Vertex buffers.
		VBuffer2.Clear();		
		for(size_t i=0;i<DrawData.size();i++)
		{
			if(g!=DrawData[i].first)
			{	
				g = DrawData[i].first;
				VBuffer1.Capacity = std::max(VBuffer1.Capacity,g->Size_VBuffer());
			}
			if(DrawData[i].second!=nullptr) //3+3 is POSITION+NORMAL.
			{					
				VBuffer2.Capacity = std::max(VBuffer2.Capacity,g->Vertices*(3+3));
			}
		}
		assert(0!=VBuffer1.Capacity);
		VBuffer1.new_Memory = new float[VBuffer1.Capacity];		
		if(0!=VBuffer2.Capacity)
		VBuffer2.new_Memory = new float[VBuffer2.Capacity];
		
		//Default camera set up.
		//THIS NEEDS ITS OWN API.
		RT::Main.SetRange.Clear(); FX::Float3 x;
		for(size_t i=0;i<DrawData.size();i++) for(int j=0;j<2;j++)
		RT::Main.SetRange+=&RT::MatrixTransform
		(DrawData[i].Data->Matrix,DrawData[i].first->SetRange.Box[j],x).x;
		RT::Main.SetRange.FitZoom();		
		RT::Main.SetDefaultCamera(); RT::Main.Center(); 
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);

	RT::Stack_Data **cd = nullptr, **cd_end = nullptr;
	if(!ControllerData.empty()) //C++98/03 support
	{
		cd = &ControllerData.front(); cd_end = &ControllerData.back()+1;
	}

	RT::Matrix um_m; size_t um_i = -1;
	//This is drawing <instance_geometry> and <instance_controller>
	//in shared <geometry> and <skeleton> order. Not material order.
	for(size_t i=0;i<DrawData.size();)	
	{	
		RT::Controller_Instance *ic = nullptr;
		RT::Stack_Draw &d = DrawData[i]; g = d.first;		

		if(um_i!=g->Asset)
		{
			um_i = g->Asset;
			RT::Up_Meter um = RT::GetUp_Meter(um_i);
			RT::MatrixLoadAsset(um_m,um.first,um.second);
		}

		if(d.second!=nullptr)
		{				
			ic = d.AsController();
			if(ic->Advance_ControllerData(cd,cd_end))
			g->Fill_VBuffer(VBuffer2.new_Memory);
			else goto truncated;
		}
		else truncated: 
		{
			g->Fill_VBuffer(VBuffer1.Memory);
		}

		do //Loop on shared <geometry>.
		{			
			RT::Stack_Draw &d = DrawData[i];

			if(d.second!=nullptr&&ic!=d.Instance)
			if(d.Advances_ControllerData(ic)) 
			{
				ic = d.AsController();
				if(ic->Advance_ControllerData(cd,cd_end))
				g->OverrideWith_ControllerData(VBuffer2.new_Memory);	
			}

			//w is here for shader semantics.
			RT::Matrix w,wv;
			//<skin> is done in Y_UP/1 space.
			//<morph> is in geometry space if
			//there are no joints. Technically
			//the targets could be in different
			//spaces, but it seems very unlikely.
			if(ic==nullptr||ic->Joints.empty())			
			{			
				RT::MatrixMult(um_m,d.Data->Matrix,w);
				RT::Main.Cg._WorldMatrix = &w;
				RT::MatrixMult(w,_View,wv);
			}
			else
			{
				RT::Main.Cg._WorldMatrix = &d.Data->Matrix;
				RT::MatrixMult(d.Data->Matrix,_View,wv);
			}
			GL::LoadMatrix(wv);
			
			//SHOULD DO THIS HERE. See CrtGeometry.cpp.
			g->Draw_VBuffer(VBuffer1.Memory,d.GetMaterials());

		}while(++i<DrawData.size()&&g==DrawData[i].first);
	}
}

void RT::Stack_Data::ShowHierarchy_glVertex3d()
{
	glVertex3d(Matrix[M30],Matrix[M31],Matrix[M32]);	
}
int RT::Stack_Data::Light(int l)
{
	glPushMatrix(); GL::MultMatrix(Matrix);
	
	//This is alternatively the color black, 0 point, or a Z direction.
	float vp[4] = { 0,0,0,1 };

	//This handles the new lighting model, l is the GL light to set up.
	for(size_t i=0;i<Node->Lights.size();i++,l++)
	{
		RT::Light *light = Node->Lights[i];		
		
		//There's only one color in the light, try to set the GL parameters sensibly.		
		if(RT::Light_Type::AMBIENT==light->Type)		
		{
			glLightfv(GL_LIGHT0+l,GL_AMBIENT,&light->Color.r);
			glLightfv(GL_LIGHT0+l,GL_DIFFUSE,vp);
			glLightfv(GL_LIGHT0+l,GL_SPECULAR,vp);	
		}
		else
		{
			glLightfv(GL_LIGHT0+l,GL_AMBIENT,vp);
			glLightfv(GL_LIGHT0+l,GL_DIFFUSE,&light->Color.r);
			glLightfv(GL_LIGHT0+l,GL_SPECULAR,&light->Color.r);	
		}
		
		if(RT::Light_Type::DIRECTIONAL!=light->Type)
		{
			glLightfv(GL_LIGHT0+l,GL_POSITION,vp);		
			glLightf(GL_LIGHT0+l,GL_CONSTANT_ATTENUATION,light->ConstantAttenuation);
			glLightf(GL_LIGHT0+l,GL_LINEAR_ATTENUATION,light->LinearAttenuation);
			glLightf(GL_LIGHT0+l,GL_QUADRATIC_ATTENUATION,light->QuadraticAttenuation);
		}

		if(RT::Light_Type::POINT!=light->Type)
		{
			if(RT::Light_Type::DIRECTIONAL==light->Type)
			{
				vp[2] = 1; vp[3] = 0; glLightfv(GL_LIGHT0+l,GL_POSITION,vp);
				vp[2] = 0; vp[3] = 1;
			}
			else glLightfv(GL_LIGHT0+l,GL_SPOT_DIRECTION,vp);
		}

		if(RT::Light_Type::SPOT==light->Type)
		{
			//The Conformance Test Suite suggests /2 (or a full angle.)
			glLightf(GL_LIGHT0+l,GL_SPOT_CUTOFF,light->FalloffAngle/2);
			glLightf(GL_LIGHT0+l,GL_SPOT_EXPONENT,light->FalloffExponent);			
		}
		else glLightf(GL_LIGHT0+l,GL_SPOT_CUTOFF,180); //Necessary?
	}

	glPopMatrix(); return l;
}

/*REFERENCE
int RT::Frame::GenerateVBO()
{
	if(UseVBOs&&VBOsAvailable)
	{
		size_t vboID;
		//Generate And Bind The Vertex Buffer
		glGenBuffers(1,&vboID);

		return vboID;
	}
	return -1;
}
bool RT::Frame::_UpdateVBO(GLenum type, GLuint vboID, const void *data, GLsizeiptrARB size)
{
	if(UseVBOs&&VBOsAvailable&&vboID>0)
	{
		//Bind The Buffer
		glBindBuffer(type,vboID);
		//Load The Data
		glBufferData(type,size,data,GL_STATIC_DRAW);

		return true;
	}
	return false;
}
bool RT::Frame::BindVBO(GLenum type, GLuint vboID)
{
	if(UseVBOs&&VBOsAvailable)
	{
		glBindBuffer(type,vboID);
		return true;
	}
	else return false;
}
void RT::Frame::FreeVBO(size_t vboUID)
{
	//Delete VBO
	if(UseVBOs&&vboUID>0)
	{
		//Free The VBO Memory
		glDeleteBuffers(1,&vboUID);	assert(vboUID!=0);
	}
} */

void RT::Frame_ShadowMap::Init()
{
	if(Initialized) return;

	//setup the depth texture
	glGenTextures(1,&Id);
	glBindTexture(GL_TEXTURE_2D,Id);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE_ARB,GL_COMPARE_R_TO_TEXTURE_ARB);
	glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,Width,Height,0,GL_DEPTH_COMPONENT,GL_UNSIGNED_INT,nullptr);

	Initialized = true;
}

void RT::Frame_ShadowMap::SetupRenderingToShadowMap()
{
	//set tc flag so polygroups will know which shader to use 
	RenderingTo = true;
	RenderingWith = false;

	//gl stuff to setup rendering to the shadow map 
	glEnable(GL_CULL_FACE);
	glViewport(0,0,Width,Height);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.1f,4);

	//void RT::Frame::PushLightViewMatrix()
	{
		//should get the current light from the scene and get it 
		//position and what now but for just for now going to 
		//set a hard value 
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		//Not implemented yet: need to look into perspective range
		gluPerspective(35,(double)Width/Height,1,30);

		//setup light view
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		//light is directly above for now 
		gluLookAt(0,30,0, //eye (light)  position
		0,0,0, //target
		0,1,0); //up vector
	}
}	 
void RT::Frame_ShadowMap::SetupRenderingWithShadowMap()
{
	//daeEH::Verbose<<"Setting Rendering with shadowmap.";

	//set tc flag so polygroups will know which shader to use 
	RenderingWith = true;
	RenderingTo = false;

	glDisable(GL_CULL_FACE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0,0);
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	glBindTexture(GL_TEXTURE_2D,Id);
	
	glViewport(0,0,Width,Height);
	glClear(GL_DEPTH_BUFFER_BIT);

	//void RT::Frame::PopLightViewMatrix()
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	//need to get the current camera and reset it again because we have since over written 
	//the model view setup by SceneUpdate
	RT::Main.Stack.ResetCamera();
}

//-------.
	}//<-'
}

/*C1071*/