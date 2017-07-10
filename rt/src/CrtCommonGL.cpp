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
  
//APIENTRY is Windows specific.
void APIENTRY CrtCommonGL_ErrorCallback
(GLenum source, GLenum type, GLuint id, GLenum severity, 
GLsizei length, const GLchar *message, const void *userParam)
{
	if(type!=GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
	daeEH::Warning<<"OpenGL programmer error:\n"<<message;
}
COLLADA_(extern) GLDEBUGPROC
CrtCommonGL_DEBUGPROC = CrtCommonGL_ErrorCallback;
									   
//This is a complicated dance to avoid double setting
//the shader parameters of the WORLD semantics family.
static RT::Stack_Draw *CrtCommonGL_SetWorld = nullptr;

void RT::Stack::SetMaterial(RT::Material *mat)
{
	if(mat==CurrentMaterial) //Same material?
	{
		//But differently positioned instance?
		if(CrtCommonGL_SetWorld!=CurrentDraw)
		{
			CrtCommonGL_SetWorld = CurrentDraw;

			if(CurrentMaterial->FX!=nullptr)			  
			{
				if(RT::Main.ShowCOLLADA_FX)
				CurrentMaterial->FX->SetWorld(0);
			}
		}
	}
	else ResetMaterial(mat); //...
}
void RT::Stack::ResetMaterial(RT::Material *mat)
{
	FX::Material *fx = CurrentMaterial->FX;	

	if(RT::Main.ShowCOLLADA_FX) if(fx!=nullptr)
	{
		//SCHEDULED FOR REMOVAL
		//Switching to legacy OpenGL rendering?
		fx->ResetPassState(0); if(mat->FX==nullptr) 
		{							   
			//Reminder: this must undo all cumulative effects.
			//So while a lot of it is Cg specific, there isn't
			//a flag tracking the use of Cg effects.

			//UNDOES glPushAttrib(GL_LIGHTING_BIT|GL_TRANSFORM_BIT)
			//It seems like the depth-test is always or something??
			//WTF??? cgSetPassState wipes the projection matrix for
			//absolutely no reason?!
			glPopAttrib(); RT::Main.FX.RestoreGL(); 
			//I swear the Cg API resets the projection matrix, but
			//it's necessary to eat this because Draw_Triangles is
			//setting up the matrixes without consideration of the
			//materials coming up next.
			glMatrixMode(GL_PROJECTION); 
			GL.LoadMatrix(RT::Main.FX.PROJECTION.Value);
			glMatrixMode(GL_MODELVIEW); 
			GL.LoadMatrix(RT::Main.FX.WORLD_VIEW.Value);
		}
	}
	else if(mat->FX!=nullptr)
	{
		//SCHEDULED FOR REMOVAL
		//See ResetPassState concerns above.
		glPushAttrib(GL_LIGHTING_BIT);
	}

	CurrentMaterial = mat; fx = mat->FX; //!
	if(fx!=nullptr&&RT::Main.ShowCOLLADA_FX) 
	{
		CrtCommonGL_SetWorld = CurrentDraw;

		//Reminder: Only doing Apply once per change depends on
		//CG_IMMEDIATE_PARAMETER_SETTING.
		//Pushes the setparam values into cgFX for this material
		//(Avoiding connected parameters.)
		fx->Apply();
		fx->SetPassState(0); return;
	}
							  
	float shininess = mat->Shininess;
	//if the scale is 0 to 1, make it 0 to 100
	if(shininess<1) shininess = shininess*128; 

	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,&mat->Emission.r);
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,&mat->Ambient.r);
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,&mat->Diffuse.r);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,&mat->Specular.r);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shininess);

	GL.ActiveTexture(GL_TEXTURE0);
	if(mat->Mono_TexId!=0&&(1&RT::Main.ShowTextures_Mask))
	{			
		glEnable(GL_TEXTURE_2D);		
		glBindTexture(GL_TEXTURE_2D,mat->Mono_TexId);		
	}
	else glDisable(GL_TEXTURE_2D);
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
	else if(RT::Main.Zoom>0) //Zoomed out?
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
	
	//NEW: Added for GLUI location bar.
	int l = RT::Main.Left, t = RT::Main.Top; 
	//While not strictly necessary, cameras have aspect ratios, and whatever
	//is outside of the ratio is not part of the intended image. Distortions
	//are a large problem in graphics, and distortions are greatest at edges.
	int w = int(c->Aspect*RT::Main.Height+0.5f);
	if(w>RT::Main.Width)
	{
		int h = int(RT::Main.Width/c->Aspect+0.5f);
		glViewport(l,t+(RT::Main.Height-h)/2,RT::Main.Width,h);
	}
	else glViewport(l+(RT::Main.Width-w)/2,t,w,RT::Main.Height);
}

void RT::Stack::Draw()
{
	//The camera is always updated in case it's animated.
	RT::Frame_FX &FX = RT::Main.FX;
	RT::Main.Matrix(&FX.VIEW.Value,&FX.VIEW_INVERSE.Value);
	GL::LoadMatrix(FX.VIEW.Value);
	_ResetCamera(); 
	FX.Reset_Context();
				
	#ifdef NDEBUG
	#error Shaders may exceed GL_MAX_LIGHTS.
	#endif
	//reset all lights	
	int l=0,lN = 7;
	glGetIntegerv(GL_MAX_LIGHTS,&lN);
	if(glIsEnabled(GL_LIGHTING))
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
		ResetMaterial(); //In case client draws.

		Draw_Triangles();

		ResetMaterial(); //Reset the FX state.
	}

	//This should be last.	
	if(RT::Main.ShowHierarchy) 
	{
		GL::LoadMatrix(FX.VIEW.Value); Draw_ShowHierarchy();
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

			RT::Frame_FX &FX = RT::Main.FX;
			//<skin> is done in Y_UP/1 space.
			//<morph> is in geometry space if
			//there are no joints. Technically
			//the targets could be in different
			//spaces, but it seems very unlikely.			
			RT::Matrix &wv = FX.WORLD_VIEW.Value,&w = 			
			(ic==nullptr||ic->Joints.empty()?FX.WORLD:FX.IDENTITY).Value;
			if(&w!=&FX.IDENTITY.Value) //Non-skin?
			{			
				RT::MatrixMult(um_m,d.Data->Matrix,w);				
				RT::MatrixMult(w,FX.VIEW.Value,wv);
			}
			else RT::MatrixMult(d.Data->Matrix,FX.VIEW.Value,wv);

			if(!RT::Main.FX.SetWorld(d,w,wv)) GL::LoadMatrix(wv);

			//This is a backdoor signal to SetMaterial to 
			//tell it if an instance is reusing materials.
			CurrentDraw = &d;
			
			//SHOULD DO THIS HERE. See CrtGeometry.cpp.
			g->Draw_VBuffer(VBuffer1.Memory,d.GetMaterials());

		}while(++i<DrawData.size()&&g==DrawData[i].first);
	}

	CurrentDraw = nullptr;
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
	//THIS IS BETTER WITH NEWER LIBRARIES.
	//It would be good to visualize shapes.
	//The Bullet Physics visualizer insists
	//on unneeded axes that are not to scale.
	//#ifdef _DEBUG	
	{
		//Debug only. Visualize Physics sims.
		glColor3f(0.25,0.25,1); 
		RT::Main.Physics.VisualizeWorld();
	}
	//#endif		
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
			size_t stacked = i+cameras; if(stacked!=0)
			{
				int px = &d==RT::Main.Parent?2:5;

				glEnd(); //glPointSize is fixed.
				glPointSize(ps+(int)stacked*2*px);
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
		RT::MatrixMult(wv,RT::Main.FX.VIEW.Value,wv);
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
	if(lit) glEnable(GL_LIGHTING); glColor3f(1,1,1);
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
		if(l<8) if(RT::Light_Type::AMBIENT==light->Type)		
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

			if(l==(int)i&&RT::Light_Type::DEFAULT==light->Type)
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

			if(l<8) 
			{
			glLightfv(GL_LIGHT0+l,GL_POSITION,&vp.x);		
			glLightf(GL_LIGHT0+l,GL_CONSTANT_ATTENUATION,light->ConstantAttenuation);
			glLightf(GL_LIGHT0+l,GL_LINEAR_ATTENUATION,light->LinearAttenuation);
			glLightf(GL_LIGHT0+l,GL_QUADRATIC_ATTENUATION,light->QuadraticAttenuation);
			}

			if(l<RT::Frame_FX::LIGHT_MAX)
			RT::Main.FX.LIGHT_POSITION[l].World.Value = vp;
			if(l<RT::Frame_FX::LIGHT_MAX)
			glGetLightfv(GL_LIGHT0+l,GL_POSITION,&RT::Main.FX.LIGHT_POSITION[l].View.Value.x);
		}

		if(RT::Light_Type::POINT!=light->Type)
		{
			float sense = 1;	
			//This is somewhat nuts. GL_POSITION is -1 
			//just as spotlights are, but it's defined
			//in terms of the -z direction, defaulting
			//to 0,0,1,0 whereas GL_SPOT_DIRECTION has
			//only 3 components and is a normal vector.
			if(RT::Light_Type::SPOT==light->Type)
			sense = -sense;

			FX::Float3 z(0);
			switch(RT::GetUp_Meter(Node->Asset).first)
			{
			case RT::Up::X_UP: 
			//1 is consistent with FxComposer.			
			case RT::Up::Y_UP: z.z = sense; break;
			case RT::Up::Z_UP: z.y = sense; break; 
			}

			RT::MatrixRotate(Matrix,z,vp);
			//MatrixRotate is just a 3x3 multiplication.
			//OpenGL may normalize but a shader may not.
			vp.Normalize();
			
			if(RT::Light_Type::DIRECTIONAL==light->Type)
			{
				vp.w = 0; if(l<8) glLightfv(GL_LIGHT0+l,GL_POSITION,&vp.x);				

				if(l<RT::Frame_FX::LIGHT_MAX)
				RT::Main.FX.LIGHT_POSITION[l].World.Value = vp;
				if(l<RT::Frame_FX::LIGHT_MAX)
				glGetLightfv(GL_LIGHT0+l,GL_POSITION,&RT::Main.FX.LIGHT_POSITION[l].View.Value.x);
			}
			else 
			{
				if(l<8) glLightfv(GL_LIGHT0+l,GL_SPOT_DIRECTION,&vp.x);

				if(l<RT::Frame_FX::LIGHT_MAX)
				RT::Main.FX.SPOT_DIRECTION[l].World.Value = vp;
				if(l<RT::Frame_FX::LIGHT_MAX)
				glGetLightfv(GL_LIGHT0+l,GL_SPOT_DIRECTION,&RT::Main.FX.SPOT_DIRECTION[l].View.Value.x);
			}
		}		

		if(l<8) if(RT::Light_Type::SPOT==light->Type)
		{
			//The Conformance Test Suite suggests /2 (or a full angle.)
			glLightf(GL_LIGHT0+l,GL_SPOT_CUTOFF,light->FalloffAngle/2);
			glLightf(GL_LIGHT0+l,GL_SPOT_EXPONENT,std::max(0.00001f,light->FalloffExponent));
		}
		else glLightf(GL_LIGHT0+l,GL_SPOT_CUTOFF,180); //Necessary?
	}

	return l;
}

//-------.
	}//<-'
}

/*C1071*/
