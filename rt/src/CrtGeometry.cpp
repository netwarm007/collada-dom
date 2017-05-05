/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <RT.pch.h> //PCH

#include "CrtRender.h"
#include "CrtEffect.h"
#include "CrtGeometry.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
		
//SCHEDULED FOR REMOVAL 
//RT::Stack::Draw_Triangles() is managing this.
void RT::RangeFinder::SetZoom()
{
	RT::Main.Zoom = Zoom;	
	daeEH::Verbose<<"Zoom is "<<Zoom;
}
void RT::RangeFinder::operator()(RT::Geometry_Semantic &positions, size_t vertices)
{
	size_t s = positions.Stride;
	const RT::Float *it = &positions->value[positions.Offset];
	for(const RT::Float *itt=it+s*vertices;it<itt;it+=s)	
	operator+=(it);
		
	FitZoom(); Zoom*=RT::Asset.Meter;
	for(int i=0;i<2;i++) RT::Asset.Mult(Box[i][0],Box[i][1],Box[i][2]);
}

//SCHEDULED FOR REMOVAL
void RT::Geometry::Draw_VBuffer(float *p, std::vector<RT::Material_Instance> &materials)
{   	
	FX::Material *fx = nullptr;
	xs::string symbol = nullptr;
	const int format2 = GetFormat();
	int format = -1, stride = 0, first=0; //RTC
	for(size_t i=0;i<Elements.size();i++)
	{
		RT::Geometry_Elements &e = Elements[i];
		if(symbol!=e.Material_Instance_Symbol)
		{
			symbol = e.Material_Instance_Symbol;

			//Restore the old material state
			if(fx!=nullptr) 
			{
				fx->ResetPassState(0);
				fx = nullptr;
			}

			for(size_t i=0;i<materials.size();i++)			
			if(symbol==materials[i].Symbol)
			{
				RT::Material *m = materials[i].Material;
				fx = m->COLLADA_FX;				
				if(fx!=nullptr)
				RT::Main.Cg.SetPassState(fx,0);
				else RT::Main.Stack.SetMaterial(m);
				break;
			}
		}

		if(format!=e.GetFormat())
		{
			format = e.GetFormat();

			p+=first*stride; first = 0;			 		

			const int f2s[4] = {3,6,5,8};
			stride = f2s[format|format2];

			//OpenGL SPECIFIC
			//Have RT::Stack manage glVertexAttribPointer.
			glVertexPointer(3,GL_FLOAT,stride*sizeof(float),p);
			if(stride>=6) //HACK
			{
				glEnableClientState(GL_NORMAL_ARRAY);
				glNormalPointer(GL_FLOAT,stride*sizeof(float),p+3);
			}
			else glDisableClientState(GL_NORMAL_ARRAY);
			if(stride%3!=0) //HACK
			{
				GL.ClientActiveTexture(GL_TEXTURE0);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(2,GL_FLOAT,stride*sizeof(float),p+stride-2);
			}
			else glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		//OpenGL SPECIFIC
		glDrawArrays(e.Mode,first,e.Width); first+=e.Width;
	}
	//Restore the old material state
	if(fx!=nullptr) fx->ResetPassState(0);
}

template<int Fill> 
/**
 * Implements @c RT::Geometry::Fill_VBuffer() family of APIs.
 */
struct CrtGeometry_fill
{	
	template<int Pred, int N, class T> 
	void Override_if(const T *p)
	{
		if(Pred) for(size_t i=0;i<N;i++)
		o[i] = (float)p[i]; o+=N;
	}	
	void Override_up(const RT::Float *p)
	{
		//OpenGL doesn't provide a mechanism for
		//transforming normals separate from the
		//position inputs.
		o[0] = (float)p[0];
		o[1] = (float)p[1];
		o[2] = (float)p[2];
		std::swap(o[1]*=_1up,o[(int)up]); o+=3;
	}	
	template<int Format>
	void VBuffer2(RT::Geometry_Elements &e)
	{
		GLuint *ep = &g.ElementBuffer[e.Region];		
		//These are used according to Format.
		#define _(w,x,y) const RT::Float *x;\
		RT::Geometry_Semantic &x##s = e.y!=nullptr?e.y:g.y;\
		GLuint *e##x = ep+x##s.Index*e.Width;\
		if(Format&w) x = &x##s->value[x##s.Offset];
		_(1,n,Normals)
		_(2,t0,TexCoords)
		#undef _

		for(size_t i=0;i<e.Width;i++)
		{
			if(128&Format) //<controller>
			Override_if<Fill,3>(p2+ep[i]*3); //POSITION
			else if(256&Format)
			Override_if<Fill,6>(p2+ep[i]*6); //POSITION+NORMAL
			else //<geometry>
			Override_if<Fill,3>(p+ep[i]*ps); //POSITION
			if(1&Format) 		
			if(!(128&Format)||!(1&Fill))
			Override_if<1&Fill,3>(n+en[i]*ns.Stride); //NORMAL
			else
			Override_up(n+en[i]*ns.Stride); //Separated NORMAL
			if(2&Format) 
			Override_if<2&Fill,2>(t0+et0[i]*t0s.Stride); //TEXCOORD
		}
	}
	//p2 will hold <controller> interpolants.
	const RT::Float *p; size_t ps; float *p2;
	operator size_t() //VBuffer1()
	{
		p = &g.Positions->value[g.Positions.Offset];
		ps = g.Positions.Stride;		
		p2 = nullptr; if(Fill)
		{			
			p2 = RT::Main.Stack.VBuffer2.new_Memory;	
			if(b==p2) b = o = RT::Main.Stack.VBuffer1.Memory;
			else p2 = nullptr;		
		}

		const int format2 = 
		p2!=nullptr?g.Normals!=nullptr?256:128:g.GetFormat();

		for(size_t i=0;i<g.Elements.size();i++)
		{
			RT::Geometry_Elements &e = g.Elements[i];
			if(m==nullptr||m==e.Material_Instance_Symbol)
			switch(format2|e.GetFormat())
			{
			#define _(x) case x: VBuffer2<x>(e); break;
			_(0)_(1)_(2)_(3) //<geometry>
			_(128)_(128|1)_(128|2)_(128|3) //<controller> with POSITION
			_(256)_(256|2) //<controller> with POSITION and with NORMAL
			#undef _
			}
		}
		return o-b;
	}
	const RT::Up up; const float _1up;
	float *b,*o; RT::Geometry &g; xs::string m;
	CrtGeometry_fill(float *b, RT::Geometry &g, xs::string m)
	:b(b),o(b),g(g),m(m),up(RT::GetUp_Meter(g.Asset).first)
	,_1up(float(up==RT::Up::Y_UP?1:-1)){}
};
size_t RT::Geometry::Size_VBuffer()
{
	return CrtGeometry_fill<0>(nullptr,*this,nullptr); 
}
size_t RT::Geometry::Fill_VBuffer(float *b, xs::string m)
{
	return CrtGeometry_fill<~0>(b,*this,m==nullptr?nullptr:m); 
}
size_t RT::Geometry::OverrideWith_ControllerData(float *p2, xs::string m)
{
	return CrtGeometry_fill<128|256>(p2,*this,m==nullptr?nullptr:m); 
}

//-------.
	}//<-'
}

/*C1071*/