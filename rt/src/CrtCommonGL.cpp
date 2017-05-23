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

	FLOAT shininess = mat->Shininess;
	//if the scale is 0.0 to 1.0, make it 0.0 to 100.0
	if(shininess<1) shininess = shininess*128; 

	//	diffuse.a	=	effect->Transparency;
	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,&mat->Emission.f0);
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,&mat->Ambient.f0);
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,&mat->Diffuse.f0);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,&mat->Specular.f0);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shininess);

	//This looped through the textures, but it's 
	//being used for profile_COMMON only and it's
	//SCHEDULED FOR REMOVAL also.
	if(!effect->Textures.empty()&&(1&RT::Main.ShowTextures_Mask))
	{
		int TexId = effect->Textures[0]->TexId;
		if(TexId==0) //HACK: Load missing texture?
		{
			Collada05::const_image yy = nullptr;
			TexId = FX::Loader::GetID_TexId(yy);
		}

		glEnable(GL_TEXTURE_2D);
		//GL.ActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D,TexId);		
	}
	else glDisable(GL_TEXTURE_2D); CurrentMaterial = mat;
}

void RT::Stack::_ResetCamera()
{
	RT::Camera *c = RT::Main.Camera;

	//Reminder: near/far are macros.
	RT::Float nZ = c->ZNear, fZ = c->ZFar; 	
	if(c->Id=="COLLADA_RT_default_camera")
	{
		//The default camera doesn't have a set zoom.
		#ifdef NDEBUG
		#error 50/1000 are arbitrary. 
		#endif
		nZ = std::max(RT::Main.Zoom,RT::Main.SetRange.Zoom)/50;
		fZ = nZ*1000;
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
	glLoadIdentity(); //These APIs call glMultMatrix.
	switch(c->Refresh((RT::Float)RT::Main.Width/RT::Main.Height))
	{
	case 2: glOrtho(-c->Xmag,c->Xmag,-c->Ymag,c->Ymag,nZ,fZ); break;
	case 3: //gluPerspective(c->Yfov,c->Aspect,nZ,fZ); break;
		
		float top = nZ*tan(RT::DEGREES_TO_RADIANS*c->Yfov/2);
		float right = top*c->Aspect;
		glFrustum(-right,right,-top,top,nZ,fZ); break;
	}			
	glMatrixMode(GL_MODELVIEW);

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
}

void RT::Stack::Draw()
{
	//The camera is always updated in case it's animated.
	RT::Main.Matrix(&_View,&RT::Main.Cg._InverseViewMatrix);
	GL::LoadMatrix(_View);
	_ResetCamera();	
							 
	//reset all lights	
	int l=0,lN = 7;
	glGetIntegerv(GL_MAX_LIGHTS,&lN);
	for(size_t i=0;i<Data.size();i++)	
	if(!Data[i].Node->Lights.empty())
	{
		int ll = l; 
		l = std::min(lN,Data[i].Light(l));
		//Enable the light and lighting
		while(ll<l) glEnable(GL_LIGHT0+ll++);
		if(l==lN) break;
	}
	while(l<lN&&glIsEnabled(GL_LIGHT0+l))
	glDisable(GL_LIGHT0+l++);
		
	if(RT::Main.ShowGeometry)
	{
		//Are we using shadow maps?
		//!!!GAC Shadow map code may not be functional right now
		if(RT::Main.ShadowMap.Use&&RT::Main.ShadowMap.Initialized)
		{
			//Render the shadow pass
			RT::Main.ShadowMap.PushRenderingToShadowMap();
			Draw_Triangles();		
			//Now render the scene 
			RT::Main.ShadowMap.PopRenderingToShadowMap();
			Draw_Triangles();
		}
		else Draw_Triangles();
	}
	
	//This should be last.	
	if(RT::Main.ShowHierarchy) 
	{
		GL::LoadMatrix(_View); Draw_ShowHierarchy();
	}
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
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);

	RT::Stack_Data **cd = nullptr, **cd_end = nullptr;
	if(!ControllerData.empty()) //C++98/03 support
	{
		cd = &ControllerData.front(); cd_end = &ControllerData.back()+1;
	}

	RT::Up_Meter um;
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
			um = RT::GetUp_Meter(um_i);
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
void RT::Stack::Draw_ShowHierarchy()
{
	//Do set up.
	GLboolean lit = glIsEnabled(GL_LIGHTING);
	int df; glGetIntegerv(GL_DEPTH_FUNC,&df);
	if(lit) glDisable(GL_LIGHTING);
	glDepthFunc(GL_LESS);
	glDisable(GL_TEXTURE_2D);		
		
	//GL_LINES
	//Draw lines to each parent node.		
	glColor3f(1,0.5,1); 
	glLineWidth(1); 
	glBegin(GL_LINES);		
	for(size_t i=1;i<Data.size();i++)
	{
		Data[i].ShowHierarchy_glVertex3d();
		Data[i].Parent->ShowHierarchy_glVertex3d();
	}
	//It would be good to visualize shapes.
	//The Bullet Physics visualizer insists
	//on unneeded axes that are not to scale.
	#ifdef _DEBUG	
	{
		//Debug only. Visualize Physics sims.
		glColor3f(0.25,0.25,1); 
		RT::Main.Physics.VisualizeWorld();
	}
	#endif		
	glEnd(); glLineWidth(1);

	//GL_POINTS
	//Draw icons for lights & cameras.
	const float ps = 30; 
	//Don't obscure the camera with icon.
	if(RT::Main.Parent!=&Data[0])
	{
		//Make the inner icon a 2px line.
		//Don't worry about stacked ones.
		glPointSize(ps-4);
		//Carve out a hole in the middle.		
		//Don't worry about Z-sorting it.
		glColorMask(0,0,0,0);
		glBegin(GL_POINTS); 
		RT::Main.Parent->ShowHierarchy_glVertex3d();
		glEnd();
		glColorMask(1,1,1,1);
	}
	glPointSize(ps);
	glBegin(GL_POINTS);
	for(size_t i=0;i<Data.size();i++)
	{	
		RT::Stack_Data &d = Data[i];
		size_t cameras = d.Node->Cameras.empty()?0:1;		
		//This is drawn in front of the icons.
		if(cameras!=0)
		{	
			//Cameras are black and unstacked.
			glColor3f(0,0,0);				
			d.ShowHierarchy_glVertex3d();
		}
		for(size_t i=0;i<d.Node->Lights.size();i++)
		{
			//In the event that lights/cameras are stacked
			//on a common node:
			//(Note: If two nodes overlap, they just are.)
			int stacked = i+cameras; if(stacked!=0)
			{
				int px = &d==RT::Main.Parent?2:5;

				glEnd(); //glPointSize is fixed.
				glPointSize(ps+stacked*2*px);
				glBegin(GL_POINTS);
			}

			FX::Float3 hue = d.Node->Lights[i]->Color; 
			//NaN is undefined and may appear like cameras.
			hue+=0.00001f; hue.Normalize();
			glColor3f(hue.r,hue.g,hue.b);			
			d.ShowHierarchy_glVertex3d();

			if(stacked!=0) //glPointSize is fixed.
			{
				glEnd(); glPointSize(ps);
				glBegin(GL_POINTS);
			}
		}
	}
	glEnd(); glPointSize(1);

	//Draw <spline> control shapes.	
	glColor3f(1,1,1); glLineStipple(2,0xAAAA);
	glLineWidth(2); glEnable(GL_LINE_STIPPLE);
	for(size_t i=0;i<ShowHierarchy_Splines.size();i++)	
	for(int j=ShowHierarchy_Splines[i++];j<ShowHierarchy_Splines[i];j++)
	{
		RT::Spline *g = DrawData[j].first->AsSpline();
		if(0==g->Points) continue;
		RT::Up_Meter um = RT::GetUp_Meter(g->Asset);
		RT::Matrix wv;
		RT::MatrixLoadAsset(wv,um.first,um.second);
		RT::MatrixMult(wv,DrawData[j].Data->Matrix,wv);		
		RT::MatrixMult(wv,_View,wv);
		GL::LoadMatrix(wv);		
		glBegin(GL_LINE_STRIP);
		//TODO: Try to generalize this to an animation display.
		int kN = g->BSPLINE_Entry+g->Points+g->Open;
		for(int k=g->BSPLINE_Entry;k<=kN;k++)
		{
			RT::Spline_Point &pt = g->SamplePoints[k*g->PointSize];
			RT::Float *p = pt.GetParameters();
			for(int l=0;l<(pt.IsBezier()&&k!=kN?3:1);l++,
			p+=g->Parameters+(l==2?g->PointSize:0))
			if(1==g->Parameters) switch(g->Sense)
			{
			case RT::Up::X_UP: glVertex3f(p[0],0,0); break;
			case RT::Up::Y_UP: glVertex3f(0,p[0],0); break;
			case RT::Up::Z_UP: glVertex3f(0,0,p[0]); break;
			}
			else if(2==g->Parameters) switch(g->Sense)
			{
			case RT::Up::X_UP: glVertex3f(0,p[0],p[1]); break;
			case RT::Up::Y_UP: glVertex3f(p[0],0,p[1]); break;
			case RT::Up::Z_UP: glVertex3f(p[0],p[1],0); break;
			}
			else glVertex3d(p[0],p[1],p[2]);
		}
		glEnd();
	}	
	glLineWidth(1); glDisable(GL_LINE_STIPPLE);

	//Undo set up.
	glEnable(GL_TEXTURE_2D);
	glDepthFunc(df);
	if(lit) glEnable(GL_LIGHTING);
}

int RT::Stack_Data::Light(int l)
{	
	//This handles the new lighting model, l is the GL light to set up.
	for(size_t i=0;i<Node->Lights.size();i++,l++)
	{
		//This is alternatively the color black, 0 point, or a Z direction.
		FX::Float4 vp(0,1);

		RT::Light *light = Node->Lights[i];		
		
		//There's only one color in the light, try to set the GL parameters sensibly.		
		if(RT::Light_Type::AMBIENT==light->Type)		
		{
			glLightfv(GL_LIGHT0+l,GL_AMBIENT,&light->Color.r);
			glLightfv(GL_LIGHT0+l,GL_DIFFUSE,&vp.x);
			glLightfv(GL_LIGHT0+l,GL_SPECULAR,&vp.x);	
		}
		else
		{
			glLightfv(GL_LIGHT0+l,GL_AMBIENT,&vp.x);
			glLightfv(GL_LIGHT0+l,GL_DIFFUSE,&light->Color.r);
			glLightfv(GL_LIGHT0+l,GL_SPECULAR,&light->Color.r);	
		}
		
		if(RT::Light_Type::DIRECTIONAL!=light->Type)
		{	
			vp.x = Matrix[M30]; vp.y = Matrix[M31]; vp.z = Matrix[M32];

			if(l==i&&RT::Light_Type::DEFAULT==light->Type)
			{
				switch(i)
				{
				case 0: glLightfv(GL_LIGHT0,GL_AMBIENT,&light->Color.r);
				        vp.x = +RT::Main.SetRange.Zoom; break;
				case 1: vp.y = +RT::Main.SetRange.Zoom; break;
				case 2: vp.z = +RT::Main.SetRange.Zoom; break;
				case 3: vp.x = -RT::Main.SetRange.Zoom; break;
				case 4: vp.y = -RT::Main.SetRange.Zoom; break;
				case 5: vp.z = -RT::Main.SetRange.Zoom; break;
				}
				vp.y+=RT::Main.SetRange.Y();
			}

			glLightfv(GL_LIGHT0+l,GL_POSITION,&vp.x);		
			glLightf(GL_LIGHT0+l,GL_CONSTANT_ATTENUATION,light->ConstantAttenuation);
			glLightf(GL_LIGHT0+l,GL_LINEAR_ATTENUATION,light->LinearAttenuation);
			glLightf(GL_LIGHT0+l,GL_QUADRATIC_ATTENUATION,light->QuadraticAttenuation);
		}

		if(RT::Light_Type::POINT!=light->Type)
		{
			FX::Float3 z(0);
			//It seems like Z_UP might want -1 instead, but +1 just works.
			//glLight says 1 is -1:
			//"The initial position is (0,0,1,0); thus, the initial light 
			//source is directional, parallel to, and in the direction of 
			//the -z axis."
			switch(RT::GetUp_Meter(Node->Asset).first)
			{
			case RT::Up::X_UP: 
			//1 is consistent with FxComposer.
			//(COLLADA-CTS is too cool for surface normals.)
			case RT::Up::Y_UP: z.z = 1; break;
			case RT::Up::Z_UP: z.y = 1; break; //Why not -1?
			}
			RT::MatrixRotate(Matrix,z,vp);
			
			if(RT::Light_Type::DIRECTIONAL==light->Type)
			{
				vp.w = 0; glLightfv(GL_LIGHT0+l,GL_POSITION,&vp.x);				
			}
			else glLightfv(GL_LIGHT0+l,GL_SPOT_DIRECTION,&vp.x);
		}

		if(RT::Light_Type::SPOT==light->Type)
		{
			//The Conformance Test Suite suggests /2 (or a full angle.)
			glLightf(GL_LIGHT0+l,GL_SPOT_CUTOFF,light->FalloffAngle/2);
			glLightf(GL_LIGHT0+l,GL_SPOT_EXPONENT,light->FalloffExponent);			
		}
		else glLightf(GL_LIGHT0+l,GL_SPOT_CUTOFF,180); //Necessary?
	}

	return l;
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

void RT::Frame_ShadowMap::PushRenderingToShadowMap()
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
		//glPopMatrix is done by PopRenderingToShadowMap().
		glPushMatrix();
		glLoadIdentity();
		//Not implemented yet: need to look into perspective range
		//gluPerspective(35,(double)Width/Height,1,30);
		float top = 1*tan(RT::DEGREES_TO_RADIANS*35/2);
		float right = top*Width/Height;
		glFrustum(-right,right,-top,top,1,30);

		//setup light view
		glMatrixMode(GL_MODELVIEW);
		//glPopMatrix is done by PopRenderingToShadowMap().
		glPushMatrix();		
		//light is directly above for now 
		//gluLookAt(0,30,0, 0,0,0, 0,1,0);
		RT::Matrix m;
		RT::MatrixLoadIdentity(m);
		RT::MatrixRotateAngleAxis(m,1,0,0,180);
		m[31] = 30;
		glLoadMatrixf(m);
	}
}	 
void RT::Frame_ShadowMap::PopRenderingToShadowMap()
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
}

//-------.
	}//<-'
}

/*C1071*/