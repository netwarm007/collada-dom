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
	
void RT::RangeFinder::operator()
(RT::Geometry_Semantic &positions, size_t vertices)
{
	size_t s = positions.Stride;
	const RT::Float *it = &positions->value[positions.Offset];
	for(const RT::Float *itt=it+s*vertices;it<itt;it+=s)	
	operator+=(it);
		
	FitZoom(); Zoom*=RT::Asset.Meter;
	for(int i=0;i<2;i++) RT::Asset.Mult(Box[i][0],Box[i][1],Box[i][2]);
}

void RT::Geometry::Generate_Normals()
{
	if(!Missing_Normals) return; 

	Missing_Normals = false; assert(Normals==nullptr); 

	if(IsSpline()) return; 

	size_t ps = Positions.Stride;	
	if(ps==0||Positions==nullptr) return; //CRASH TEST

	daeEH::Error<<Id<<" IS NEEDING SURFACE-NORMAL INFORMATION!!";
	daeEH::Warning<<"Generating blanket smooth normals for "<<Id;
		
	const RT::Float *p = &Positions->value[Positions.Offset];		

	Collada05::float_array fa(const_cast<daeDOM&>(RT::Main.DOM));		
	fa->value->resize(Positions->value->size()/ps*3);	
	RT::Float *n = fa->value->data();
	Normals = fa; Normals.Stride = 3;	

	for(size_t i=0;i<Elements.size();i++)
	{
		RT::Geometry_Elements &e = Elements[i];		
		assert(e.Normals==nullptr);
		
		GLuint *ep = &ElementBuffer[e.Region];		
		if(GL_TRIANGLES==e.Mode) 
		for(size_t i=0;i<e.Width;i+=3)
		{			
			const RT::Float *a = p+ep[i+0]*ps;
			const RT::Float *b = p+ep[i+1]*ps;
			const RT::Float *c = p+ep[i+2]*ps;
			FX::Float3 sm(c[0],c[1],c[2]);
			FX::Float3 sn(b[0],b[1],b[2]);
			sm-=sn; 
			sn-=FX::Float3(a[0],a[1],a[2]);
			sn.Cross(sm);			
			{
				RT::Float *a = n+ep[i+0]*3;
				RT::Float *b = n+ep[i+1]*3;
				RT::Float *c = n+ep[i+2]*3;
				//Normalize is not needed if += is
				//not used. It's too much trouble to
				//decompose shared facets to produce a
				//correct lighting scheme for some files.
				sn.Normalize();
				a[0]+=sn.x; a[1]+=sn.y; a[2]+=sn.z;
				b[0]+=sn.x; b[1]+=sn.y; b[2]+=sn.z;
				c[0]+=sn.x; c[1]+=sn.y; c[2]+=sn.z;
			}				
		}
	}
}
void RT::Geometry::Generate_Positions(RT::Up sense)
{
	//The idea here is to convert non 3-D POSITION 
	//to 3-D because it doesn't seem warranted for
	//CrtGeometry_fill and skinning to manage this.
	if(Positions.Dimension>=3) return;

	//This tessellates the spline at the same time
	//as checking for 2-D or 1-D geometry.
	if(IsSpline())
	return ((RT::Spline*)this)->Generate_Positions();

	size_t ps = Positions.Stride;	
	size_t pd = Positions.Dimension;	
	const RT::Float *o = &Positions->value[Positions.Offset];		
	Collada05::float_array fa(const_cast<daeDOM&>(RT::Main.DOM));		
	fa->value->resize(Vertices*3);		
	Positions = fa;	
	Positions.Offset = 0;
	Positions.Stride = 3;
	Positions.Dimension = 3;	

	if(ps!=0&&Positions==nullptr) //CRASH TEST
	return;

	RT::Float *p = fa->value->data();
	if(pd==1) //sense is the represented dimension.
	{
		p+=sense;
		for(size_t i=0;i<Vertices;i++,o++,p+=3)
		*p = *o;
	}
	else if(pd==2) //sense is the absent dimension.
	{
		int a = 0; if(sense==a) a++;
		int b = a+1; if(sense==b) b++;
		for(size_t i=0;i<Vertices;i++,o+=2,p+=3)
		{
			p[a] = o[0]; p[b] = o[1];
		}
	}	
	else assert(0);	
}
void RT::Spline::Generate_Positions()
{
	if(Positions==nullptr)
	{
		Positions = Collada05::float_array(const_cast<daeDOM&>(RT::Main.DOM));		
		Positions.Stride = 3;
		Positions.Dimension = 3;	
	}

	int resolution = 20-1;
	if(Algorithms==RT::Spline_Type::LINEAR)
	resolution = 1;
	
	Collada05::float_array::value_ListOfFloats &fa = 
	const_cast<Collada05::float_array::value_ListOfFloats&>
	(*Positions->value.operator->());
	int length = Points+Open;
	Vertices = length==0?0:1+length*resolution;
	fa.clear();
	fa.resize(3*Vertices);
	
	if(length!=0)
	{
		RT::Spline_Point *sp = &SamplePoints[0];
		if(BSPLINE_Entry!=0) sp+=PointSize;
		const RT::Float sep = 1.0f/resolution;
		const RT::Float sep0 = resolution==1?1:sep;
	
		int dimens = std::min<int>(3,Parameters);
		RT::Float *p = fa.data();	
		if(1==dimens) p+=Sense; //1-D?
		if(2==dimens&&0==Sense) p+=1; //YZ?
		sp->Fill(p,0,dimens,sp+PointSize,0);
		p+=3;
		for(int i=0;i<length;i++,sp+=PointSize)
		{
			RT::Float s = sep0;
			for(int i=0;i<resolution;i++,s+=sep)
			{
				sp->Fill(p,0,dimens,sp+PointSize,s);
				p+=3;
			}
		}
		if(2==dimens&&1==Sense) //XZ?
		{
			p = fa.data();
			RT::Float *pN = p+fa.size();
			for(;p<pN;p+=3) std::swap(p[1],p[2]);
		}

		Elements.resize(1);
		Elements[0].Mode = GL_LINE_STRIP;
		Elements[0].Region = 0;
		Elements[0].Width = Vertices;
	}
	else Elements.clear();
		
	size_t i = ElementBuffer.size();
	ElementBuffer.resize(Vertices);
	while(++i<Vertices) ElementBuffer[i] = i;
}

//SCHEDULED FOR REMOVAL
static RT::Effect CrtGeometry_Effect = 1;
static RT::Material CrtGeometry_Material = &CrtGeometry_Effect;
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

			RT::Material *m = nullptr;
			for(size_t i=0;i<materials.size();i++)						
			if(symbol==materials[i].Symbol)
			{
				m = materials[i].Material;
				goto have_m;
			}
			//<spline> has no symbol to speak of.			
			if(!materials.empty()) m = materials[0].Material;
			//<bind_material> is optional.
			if(materials.empty()) m = &CrtGeometry_Material;			
			have_m: 
			fx = m->COLLADA_FX;
			if(fx==nullptr) RT::Main.Stack.SetMaterial(m);
			if(fx!=nullptr) RT::Main.Cg.SetPassState(fx,0);
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
	assert(3<=Positions.Dimension);
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