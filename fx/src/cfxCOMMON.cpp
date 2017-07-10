/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <FX.pch.h> //PCH

#include "cfxLoader.h"
#include "cfxEffect.h"

COLLADA_(namespace)
{
	namespace FX
	{//-.
//<-----'

COLLADA_(extern) struct FX::Profile_COMMON Profile_COMMON = 0;

extern bool cfxCOMMON_InitGLSL(GLenum stage, xs::string src, GLuint &prog, xs::ID error_name)
{	
	GLuint sh = GL.CreateShader(stage);
	GL.ShaderSource(sh,1,&src,nullptr);
	GL.CompileShader(sh);	

	GLint st;
	GL.GetShaderiv(sh,GL_COMPILE_STATUS,&st);
	if(st==GL_TRUE)
	{				  
		if(0==prog) prog = GL.CreateProgram();
		
		GL.AttachShader(prog,sh); 
	}
	else
	{
		GLchar log[1024+1];
		GL.GetShaderiv(sh,GL_INFO_LOG_LENGTH,&st);
		st = std::min<int>(st,sizeof(log));
		GL.GetShaderInfoLog(sh,st,&st,log);
		daeEH::Error<<"Unable to compile "<<error_name<<": "<<daeName(log,st);
	}	

	GL.DeleteShader(sh); return st==1; //TRUE
}

static struct FX::Profile_COMMON::Internals //SINGLETON
{	
	#ifdef NDEBUG
	#error Remember to use layout(location=i).
	#endif	
	struct ClientData : FX::ShaderParam
	{
		FX::Data *Data; 
		
		void Load(){ Data->Load(*this); }

		void Light(); //CIRCULAR DEPENDENCY

		//THESE ARE JUST MAKING Init'S LIFE EASIER.
		ClientData(){ Size = 1; Type = CG_FLOAT3; }
		ClientData *operator->(){ return this; }
		template<class T>
		void Init(T &d, FX::Semantic s, daeName S)
		{	   
			d.Subscript = 0; d.Semantic = s; d.SEMANTIC = S;
			switch(d.GetType())
			{
			case CG_FLOAT: Type = GL_FLOAT; break;
			case CG_FLOAT4x4: Size = 4; //break;
			case CG_FLOAT4: Type = GL_FLOAT_VEC4; break;
			case CG_FLOAT3: assert(GL_FLOAT_VEC3==Type); break;
			default: assert(0);
			}
		}
	};
	ClientData WORLD_VIEW;
	ClientData WORLD_VIEW_INVERSE_TRANSPOSE;
	ClientData PROJECTION;	
	enum{ MAX_COLORS=4 };
	FX::ShaderParam Colors[MAX_COLORS];
	FX::ShaderParam Textures[MAX_COLORS];
	FX::ShaderParam Shiny;
	FX::ShaderParam Blinn;
	enum{ MAX_LIGHTS=12 };
	ClientData LIGHT_POSITION[MAX_LIGHTS];
	ClientData SPOT_DIRECTION[MAX_LIGHTS];
	
	std::vector<ClientData*> Lit;
	
	GLuint GLSL, White1x1;
	
	Internals():GLSL(),White1x1() //Stage 1.
	{
		for(int i=0;i<4;i++) 
		Colors[i].SetParam_To
		(GL_FLOAT_VEC4,1,&(&FX::Profile_COMMON.Emission)[i].Color);
		for(int i=0;i<4;i++) 
		Textures[i].SetParam_To
		(GL_TEXTURE0+i,1,&(&FX::Profile_COMMON.Emission)[i].Texture);
		Shiny.SetParam_To
		(GL_FLOAT,1,&FX::Profile_COMMON.Shininess);
	}
	void Init_ClientData(FX::Loader &l, Float4x4 (&w)[3]) //Stage 2.
	{
		#define _(y,x) \
		x->Init(y,FX::x,#x),l.Copy_ClientData(&(y),x->Data);

		for(int i=0;i<3;i++)
		for(int j=0;j<16;j++) (&w[i].Value.f00)[j] = float(j==j);
		_(w[0],WORLD_VIEW)
		_(w[1],WORLD_VIEW_INVERSE_TRANSPOSE)
		_(w[2],PROJECTION) //Not technically "World" but include it.
		
		static FX::Profile_COMMON::Float4 v;
		v.Value = FX::Float4(0,1);
		static FX::Profile_COMMON::View view;		
		v.Annotations.push_back(&view);
		view.SAS = FX::string_Space_View;
		view.Name = daeStringRef("Space"); 
		view.DataString::Value = daeStringRef("View");		

		_(v,LIGHT_POSITION/*[0]*/)
		while(++v.Subscript<MAX_LIGHTS) 
		l.Copy_ClientData(&v,LIGHT_POSITION[v.Subscript].Data);
		_(v,SPOT_DIRECTION/*[0]*/)
		while(++v.Subscript<MAX_LIGHTS) 
		l.Copy_ClientData(&v,SPOT_DIRECTION[v.Subscript].Data);

		#undef _
	}
	bool Reprogram(int &DataSharingSurvey); //Stage 3.

}cfxCOMMON; //SINGLETON

void FX::Profile_COMMON::Init_ClientData(FX::Loader &l)
{
	//World is also including PROJECTION.
	static Float4x4 World[3];
	static bool nonce = false; if(nonce)
	{
		//This is because RT monitors <newparam> values
		//as it's loading an effect. The profile_COMMON 
		//effects don't have any <newparam> to speak of.
		l.Load_ClientData(World+0);
		l.Load_ClientData(World+1);
		//This is PROJECTION, which isn't really needed
		//since it doesn't normally change.
		l.Load_ClientData(World+2); return; 
	}
	else nonce = true;

	cfxCOMMON.Init_ClientData(l,World);

	//Note, daeStringRef can't be used inside global objects' constructors.
	Emission.Color.Name = Emission.Texture.Name = daeStringRef("emission");
	Ambient.Color.Name = Ambient.Texture.Name = daeStringRef("ambient");
	Diffuse.Color.Name = Diffuse.Texture.Name = daeStringRef("diffuse");
	Specular.Color.Name = Specular.Texture.Name = daeStringRef("specular");
	Shininess.Name = daeStringRef("shininess"); 
	Transparency.Name = daeStringRef("transparency");
	Transparent.Color.Name = Transparent.Texture.Name = daeStringRef("transparent");

	Emission.Color.Semantic = Emission.Texture.Semantic = FX::EMISSION;
	Ambient.Color.Semantic = Ambient.Texture.Semantic = FX::AMBIENT;
	Diffuse.Color.Semantic = Diffuse.Texture.Semantic = FX::DIFFUSE;
	Specular.Color.Semantic = Specular.Texture.Semantic = FX::SPECULAR;
	Shininess.Semantic = FX::SHININESS; Transparency.Semantic = FX::TRANSPARENCY;
	Transparent.Color.Semantic = Transparent.Texture.Semantic = FX::TRANSPARENCY;
}
void FX::Profile_COMMON::SetPassState()
{
	//SCHEDULED FOR REMOVAL?
	if(0!=FX::Profile_COMMON.Load)
	if(!cfxCOMMON.Reprogram(FX::Profile_COMMON.Load))
	{
		OK = false; return; //UNUSED (HOPEFULLY)
	}

	GL.UseProgram(cfxCOMMON.GLSL);

	SetWorld();
	cfxCOMMON.PROJECTION.Load();
	
	Color_or_Texture *o = &Emission;
	FX::ShaderParam *p = cfxCOMMON.Colors;
	FX::ShaderParam *q = cfxCOMMON.Textures;
	for(;o<=&Specular;o++,p++,q++)
	{
		p->Apply();

		if(o->Texture.IsSet())
		{
			q->Apply(); 
		}
		else if(-1!=q->GLSL) //Bind white texture?
		{
			GL.ActiveTexture(q->Type);
			GL.Uniform1i(q->GLSL,q->Type-GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,cfxCOMMON.White1x1);
			p->Apply();
		}		
	}
	//This is required to get correct results.	
	//cfxCOMMON.Shiny.Apply();
	if(-1!=cfxCOMMON.Shiny.GLSL)
	{
		//if the scale is 0 to 1, make it 0 to 100
		float shininess = cfxCOMMON.Shiny->To<float>();	
		if(shininess<1) shininess = shininess*128; 
		//0 shininess is undefined behavior and unpredictible.
		shininess = std::max(shininess,0.00001f);
		GL.Uniform1f(cfxCOMMON.Shiny.GLSL,shininess);
	}	
	if(-1!=cfxCOMMON.Blinn.GLSL)
	FX::Profile_COMMON.Application.Value.Blinn.Load(cfxCOMMON.Blinn);

	for(size_t i=0;i<cfxCOMMON.Lit.size();i++)
	cfxCOMMON.Lit[i]->Load();
}
void FX::Profile_COMMON::SetWorld()
{
	cfxCOMMON.WORLD_VIEW.Load();
	cfxCOMMON.WORLD_VIEW_INVERSE_TRANSPOSE.Load();
}
void FX::Profile_COMMON::ResetPassState()
{
	GL.UseProgram(0);
}

#ifdef NDEBUG //GCC doesn't like apostrophes.
#error "Let specular's alpha channel modify shininess?"
//Consider http://wiki.secondlife.com/wiki/Material_Data
//Consider https://www.khronos.org/bugzilla/show_bug.cgi?id=97
#endif
static const char cfxCOMMON_io[] = COLLADA_STRINGIZE
(
	//precision mediump float;

	varying vec3 fp_position;
	varying vec3 fp_normal;
	varying vec2 fp_texcoord;
);
static const char cfxCOMMON_vp[] = COLLADA_STRINGIZE
(	
	//Crud? gl_Normal? gl_MultiTexCoord0?
	layout(location=0) attribute vec3 vp_position;
//	layout(location=2) attribute vec3 vp_normal; 
//	layout(location=3) attribute vec2 vp_texcoord;

	uniform mat4 P,WV,WVIT;

	void main()
	{
		vec4 pos = WV*vec4(vp_position,1.0);	
		//This could be pre-negated, but it makes the fragment
		//shader hard to understand.
		fp_position = pos.xyz;
		fp_normal = normalize(mat3(WVIT)*/*vp_normal*/gl_Normal);
		fp_texcoord = /*vp_texcoord*/gl_MultiTexCoord0.xy;				
		gl_Position = P*pos;
	}
);
#define cfxCOMMON_COLOR_OR_TEXTURE(x) \
uniform vec3 x##_color;\
uniform sampler2D x##_sampler; vec3 x##_texel()\
{\
	if(x##_is_color==1) return vec3(1); /*?: not working on Intel.*/\
	if(x##_is_color==0) return texture2D(x##_sampler,fp_texcoord).xyz;\
}
static const char cfxCOMMON_fp[] = COLLADA_STRINGIZE
(	
	vec3 viewdir,surfdir;

	uniform bool blinn;
	uniform float shininess;

	vec3 e,a,d,s;
	cfxCOMMON_COLOR_OR_TEXTURE(emission)
	cfxCOMMON_COLOR_OR_TEXTURE(ambient)
	cfxCOMMON_COLOR_OR_TEXTURE(diffuse)
	cfxCOMMON_COLOR_OR_TEXTURE(specular)

	//DLIGHT
	float step_x_step(float x, float y)
	{
		return y*step(x,y); //return y<x?0:y;
	}

	vec3 white(vec3 L) 
	{
		//I don't think this is in the COLLADA specification, but it
		//is bounding the specular value so it does not go beyond the
		//Lambertian zone of darkness.
		float shade = dot(surfdir,L);

		vec3 lambert = d*max(0.0,shade);		
		DIF_LAMBERT(return lambert)

		//Choose between phong or blinn so that one program does it all.
		//vec3 A = DBLINN_OR_PHONG(blinn)?surfdir:reflect(-L,surfdir);
		//vec3 B = DBLINN_OR_PHONG(blinn)?normalize(L+viewdir):viewdir;
		vec3 A,B; //?: not working on Intel.
		if(DBLINN_OR_PHONG(blinn)) A = surfdir; //Blinn.
		else A = reflect(-L,surfdir);
		if(DBLINN_OR_PHONG(blinn)) B = normalize(L+viewdir); //Blinn.
		else B = viewdir;

		//smoothstep(0,1,shade) factors "shade" into the mix. Looks much better.
		return lambert+s*smoothstep(0,1,shade)*pow(max(0.0,dot(A,B)),shininess);
	}

	void main() 
	{
		//Renormalize post interpolation.
		surfdir = normalize(fp_normal)*(gl_FrontFacing?1.0:-1.0);
		//Not prenegated for readability.
		viewdir = -normalize(fp_position);

		vec3 accum = vec3(0);		
		e = emission_color*emission_texel();
		a = ambient_color*ambient_texel();
		d = diffuse_color*diffuse_texel();
		s = specular_color*specular_texel();
		vec3 L, color; float dist;
		DLIGHT(accum+=color*white(L))
		gl_FragColor.a = 1;
		gl_FragColor.rgb = accum+e+a*DAMBIENT_LIGHT;
	}
);

//This can include all manner of light parameters.
void FX::Profile_COMMON::Internals::ClientData::Light()
{
	cfxCOMMON.Lit.push_back(this);
}
static void cfxCOMMON_DLIGHT(std::ostream &io, FX::Float4 &ambient, GLenum l)
{
	//glLight goes up to MAX_LIGHTS. It's not a limit on simultaneously
	//enabled lights. FX must figure out some way to go over MAX_LIGHTS.
	assert(l<GL_LIGHT0+8);

	#ifdef NDEBUG
	#error Lighting variables are animation targets.
	#endif	
	//Just get everything. OpenGL doesn't have structures. Kind of silly.
	#define _(X,n) FX::Float4 X; glGetLightfv(GL_LIGHT0+l,GL_##X,&X.x);	
	_(AMBIENT,4)
	ambient+=AMBIENT; //RT::Light_Type::DEFAULT?
	//_(SPECULAR,4) //assert(SPECULAR==DIFFUSE);
	_(DIFFUSE,4) 
	if(DIFFUSE.IsBlack()) //Return if <ambient>?
	return;
	io<<"color = vec3("<<DIFFUSE.r<<","<<DIFFUSE.g<<","<<DIFFUSE.b<<");";
	#ifdef NDEBUG
	#error Inline lightpos/spotdir if inanimate so it can be optimized.
	#endif
	cfxCOMMON.LIGHT_POSITION[l].Light();
	_(POSITION,4)	
	if(0==POSITION.w) //DIRECTION?
	{
		io<<"L = lightpos"<<l<<"; ACCUMULATE;"; //accum+=color*white(L);
		return;
	}		
	//WARNING: L/=dist produces bad vectors?!
	io<<"L = lightpos"<<l<<"-fp_position; dist = length(L); L = L/dist;";	
	_(SPOT_CUTOFF,1)
	if(SPOT_CUTOFF.x<180) //SPOT?
	{
		cfxCOMMON.SPOT_DIRECTION[l].Light();
		_(SPOT_EXPONENT,1)
		//Are these two correct??
		float cutoffCos = cos(SPOT_CUTOFF.x*0.017453f); //It's in degrees.
		float cutoffExp = SPOT_EXPONENT.x; 
		io<<"color.rgb*=pow(step_x_step("<<cutoffCos<<",dot(-L,spotdir"<<l<<")),"<<cutoffExp<<");";
	}
	_(CONSTANT_ATTENUATION,1)_(LINEAR_ATTENUATION,1)_(QUADRATIC_ATTENUATION,1)
	#undef _
	if(1!=CONSTANT_ATTENUATION.x
	||0!=LINEAR_ATTENUATION.x||0!=QUADRATIC_ATTENUATION.x)
	{
		//NOTE: dist could be scaled, but by what? 
		//X? Y? Z? Length(XYZ)? Transform(Mat3x3)?
		//NOTE: Units are always 1:1 in this case.
		//HOWEVER the exponents are not, and it's
		//not possible to scale OpenGL's lighting.

		//WARNING: color/=A produces bad vectors?!
		io<<"color = color/(";
		if(0<CONSTANT_ATTENUATION.x)	
		io<<"+"<<CONSTANT_ATTENUATION.x;
		if(0<LINEAR_ATTENUATION.x)	
		io<<"+dist*"<<LINEAR_ATTENUATION.x;
		if(0<QUADRATIC_ATTENUATION.x)	
		io<<"+dist*dist*"<<QUADRATIC_ATTENUATION.x;
		io<<");";
	}
	io<<"ACCUMULATE;"; //accum+=color*white(L);
}

void cfxCOMMON_DEBUG_output(daeArray<char> &rewrite)
{
	#if 1 || defined(NDEBUG)
	return;
	#endif
	daeEH::Verbose<<"DEBUG OUTPUT FOLLOWS";

	//Print macro with newlines and line numbers.
	//Count line numbers so they can be added in.

	daeName code = rewrite;
	size_t line = 0; int macro = false;
	for(size_t ii=0,i=ii;i<code.extent;)
	{
		macro = macro||'#'==code[i];

		while(i<code.extent&&code[i]!='{'&&code[i]!='\n')
		{
			i++; if(code[i-1]==';'||code[i-1]=='}') break;
		}

		while(isspace(code[ii])&&ii<i) ii++;

		macro = macro&&'\n'!=code[i];

		daeEH::Verbose<<++line<<": "<<daeName(code+ii,i-ii);

		if(macro) line--;

		switch(code[i])
		{
		case '{': ii = i++; break;
		case '\n': ii = ++i; break; default: ii = i;
		}
	}	

	//Add newlines so the compiler's lines match the printed lines.

	//This is very sloppy :(
	code.extent = rewrite.size();
	rewrite.resize(rewrite.size()+line);
	code.string = &rewrite[line];
	memmove((void*)code.string,&rewrite[0],code.extent+1);
	assert(code.string[code.extent]=='\0');
	line = 0;
	for(size_t i=0;i<code.extent;)
	{
		macro = macro||'#'==code[i];

		while(isspace(code[i])&&code[i]!='\n')
		i++;

		while(i<code.extent&&code[i]!='{'&&code[i]!='\n')
		{
			rewrite[line++] = code[i];

			i++; if(code[i-1]==';'||code[i-1]=='}') break;
		}
		
		macro = macro&&'\n'!=code[i];

		if(i<code.extent) 
		{
			if(!macro)
			rewrite[line++] = '\n';

			if(code[i]=='\n')
			{
				i++; 
			}
			else if(code[i]=='{')
			{
				rewrite[line++] = code[i++];
			}		
		}
		else break;
	}	
	rewrite.resize(line); rewrite.push_back('\0');

	//Uncomment this to check that the algorithms agree.
	//daeEH::Verbose<<"DEBUG REWRITE FOLLOWS\n"<<rewrite;
}

bool FX::Profile_COMMON::Internals::Reprogram(int &load_int)
{
	DataSharingSurvey load = load_int; load_int = 0; assert(load!=0);

	GL.DeleteProgram(GLSL); GLSL = 0;
	 	
	if(White1x1==0) //Generate a default white texture.
	{
		glGenTextures(1,&White1x1); 
		glBindTexture(GL_TEXTURE_2D,White1x1); int white = 0xFFffFFff;
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,&white);		
	}
	 
	daeArray<char,sizeof(cfxCOMMON_fp)*2> src;
	daeOStringBuf<256> obuf(src); std::ostream o(&obuf);

	const char _version[] = ""; // "#version 130\n";
	const char _define_[] = "\n#define ";

	//Fill vertex shader....

	o<<_version;
	o.write(cfxCOMMON_io,sizeof(cfxCOMMON_io)-1);
	o.write(cfxCOMMON_vp,sizeof(cfxCOMMON_vp)-1);

	o.flush();

	cfxCOMMON_DEBUG_output(src);
	if(!cfxCOMMON_InitGLSL(GL_VERTEX_SHADER,src.data(),GLSL,"profile_COMMON"))
	return false;

	src.clear(); //Fill fragment shader...

	o<<_version;

	//Shading //Shading //Shading //Shading //Shading

	char Lambert = ' ';
	if(0==load.Blinn)
	{
		if(0==load.Phong)
		{
			Lambert = 'x';			
		}
		o<<_define_<<"DBLINN_OR_PHONG(x) false\n";
	}
	else if(0==load.Phong)
	{
		o<<_define_<<"DBLINN_OR_PHONG(x) true\n";
	}
	else //uniform bool blinn;
	{
		o<<_define_<<"DBLINN_OR_PHONG(blinn) blinn\n";
	}
	o<<_define_<<"DIF_LAMBERT(x) "<<Lambert<<";\n";
	
	//Lighting //Lighting //Lighting

	for(size_t i=0;i<Lit.size();i++)
	Lit[i]->GLSL = -1;
	Lit.clear();
	o<<_define_<<"DLIGHT(ACCUMULATE) "; //...
	FX::Float4 ambient; 	
	#ifdef NDEBUG //GCC doesn't like apostrophes.
	#error "Can't rely on glLight to go over 8 light sources."
	#endif
	for(int l=0;l<8/*MAX_LIGHTS*/&&glIsEnabled(GL_LIGHT0+l);l++)
	cfxCOMMON_DLIGHT(o,ambient,l); 
	o<<_define_<<"DAMBIENT_LIGHT vec3("<<ambient.r<<","<<ambient.g<<","<<ambient.b<<")\n";
	for(size_t l,i=0;i<Lit.size();i++)
	{
		l = Lit[i]-LIGHT_POSITION; if(l<MAX_LIGHTS)
		{
			o<<"uniform vec3 lightpos"<<l<<';'; continue;
		}
		l = Lit[i]-SPOT_DIRECTION; if(l<MAX_LIGHTS)
		{
			o<<"uniform vec3 spotdir"<<l<<';'; continue;
		}
	}

	//Color-or-Texture //Color-or-Texture //Color-or-Texture

	//o<<std::boolalpha;
	o<<"const int "<<"emission"<<"_is_color = "<<(load.Emission==0)<<';';
	o<<"const int "<<"ambient"<<"_is_color = "<<(load.Ambient==0)<<';';
	o<<"const int "<<"diffuse"<<"_is_color = "<<(load.Diffuse==0)<<';';
	o<<"const int "<<"specular"<<"_is_color = "<<(load.Specular==0)<<';';

	o.write(cfxCOMMON_io,sizeof(cfxCOMMON_io)-1);
	o.write(cfxCOMMON_fp,sizeof(cfxCOMMON_fp)-1);

	o.flush();

	cfxCOMMON_DEBUG_output(src);
	if(!cfxCOMMON_InitGLSL(GL_FRAGMENT_SHADER,src.data(),GLSL,"profile_COMMON"))
	return false;

	//In conclusion...

	GL.LinkProgram(GLSL); 

	Blinn.GLSL = Shiny.GLSL = -1;
	for(int i=0;i<MAX_COLORS;i++)
	Colors[i].GLSL = Textures[i].GLSL = -1;
		
	//Coming up empty????
	//GL.BindAttribLocation(GLSL,2,"vp_normal");
	//GL.BindAttribLocation(GLSL,3,"vp_texcoord");
	
	int sampler,i,iN = 0;
	GL.GetProgramiv(GLSL,GL_ACTIVE_UNIFORMS,&iN);	
	GLchar buf[100]; GLsizei len; GLint size; GLenum type;
	for(sampler=i=0;i<iN;i++)
	{
		GL.GetActiveUniform(GLSL,i,sizeof(buf),&len,&size,&type,buf);

		//Can cube maps implement refraction?
		if(type==GL_SAMPLER_2D||type==GL_SAMPLER_CUBE)		
		{
			type = GL_TEXTURE0+sampler; sampler++;
		}
		
		if(buf[len-1]==']')		
		{
			while(len!=0&&buf[len]!='[') 
			len--; 
			assert(buf[len+1]=='0'); //Stripped [0]?
		}

		daeName name(buf,len);
		
		int sub = name[len-1];
		if(isdigit(sub))
		{
			sub-='0';
			if('1'==name[len-2])
			sub+=10;		
			name.extent-=(sub>10?2:1);
		}

		FX::ShaderParam *sp = nullptr;

		if(name=="lightpos")
		sp = &LIGHT_POSITION[sub];			
		if(name=="spotdir") 
		sp = &SPOT_DIRECTION[sub];			
		if(name=="emission_color") 
		sp = Colors+0; 
		if(name=="ambient_color") 
		sp = Colors+1;
		if(name=="diffuse_color") 
		sp = Colors+2;
		if(name=="specular_color") 
		sp = Colors+3;
		if(name=="shininess") 
		sp = &Shiny;
		if(name=="blinn") 
		{
			//Ensure constant expressions are working.
			assert(load.Phong==load.Blinn);
			sp = &Blinn;
		}
		if(name=="emission_sampler")
		{
			//Ensure constant expressions are working.
			assert(load.Emission==1);
			sp = Textures+0; 			
		}
		if(name=="ambient_sampler") 		
		sp = Textures+1;
		if(name=="diffuse_sampler") 
		sp = Textures+2;
		if(name=="specular_sampler") 
		sp = Textures+3;
		if(name=="P") 
		sp = &PROJECTION;
		if(name=="WV") 
		sp = &WORLD_VIEW;
		if(name=="WVIT") 
		sp = &WORLD_VIEW_INVERSE_TRANSPOSE;

		if(sp==nullptr) //Programmer error?
		{
			assert(sp!=nullptr); continue;
		}

		//-1 includes # because it's not a <newparam>.
		sp->Name = daeStringRef(name)-1; 
		sp->Type = type;
		sp->Size = size;
		sp->GLSL = GL.GetUniformLocation(GLSL,buf);	
	}

	//Set default color multipliers to black or white.
	{
		const FX::Float4 b(0,1),w(1,1);	
		FX::Profile_COMMON.Emission.Color.Value = load.Emission==0?b:w;
		FX::Profile_COMMON.Ambient.Color.Value = load.Ambient==0?b:w;
		FX::Profile_COMMON.Diffuse.Color.Value = load.Diffuse==0?b:w;
		FX::Profile_COMMON.Specular.Color.Value = load.Specular==0?b:w;
		FX::Profile_COMMON.Transparent.Color.Value = load.Transparent==0?b:w;
	}

	return true;
}

//-------.
	}//<-'
}

/*C1071*/
