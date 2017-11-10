/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_FX__DATA_H__
#define __COLLADA_FX__DATA_H__

#include "cfxTypes.h"

COLLADA_(namespace)
{
	//SCHEDULED FOR REMOVAL
	namespace RT
	{
		typedef daeDouble Float;
		typedef DAEP::Class<RT::Float[4*4]> Matrix;
	};

	namespace FX
	{//-.
//<-----'

template<class T>
/**INTERNAL 
 * This design could use more thought.
 */
struct DataMaker
{	
	FX::Paramable *p;

	xs::ID sid; typename T::arg2 aux; 

	T *&o; const daeContents &c;
		
	template<int yy> void MakeData();

	void MakeData2(T*,xs::ID);

	//ShaderParam has its data set, whereas
	//the others are new allocated/returned.
	bool Emplace(...){ return false; }
	bool Emplace(FX::ShaderParam*){ return true; }
};	  
extern void MakeData05(FX::DataMaker<FX::Annotate>&);
extern void MakeData05(FX::DataMaker<FX::NewParam>&);
extern void MakeData05(FX::DataMaker<FX::SetParam>&);
extern void MakeData05(FX::DataMaker<FX::ShaderParam>&);
extern void MakeData08(FX::DataMaker<FX::Annotate>&);
extern void MakeData08(FX::DataMaker<FX::NewParam>&);
extern void MakeData08(FX::DataMaker<FX::SetParam>&);
extern void MakeData08(FX::DataMaker<FX::ShaderParam>&);

template<class T>
struct DataTraits
{
	typedef T Unit;
	static const CGtype Base = CG_TYPELESS_STRUCT;
	static const size_t Size = 0;
};
#define _(x,y,z,w) \
template<> struct DataTraits<z>\
{\
	typedef y Unit;\
	static const CGtype Base = w;\
	static const size_t Size = x;\
};
_(1,bool,bool,CG_BOOL)
_(1,bool,FX::Bool1,CG_BOOL1)
_(1,bool,FX::Bool2,CG_BOOL2)
_(3,bool,FX::Bool3,CG_BOOL3)
_(4,bool,FX::Bool4,CG_BOOL4)
_(1,int,int,CG_INT)
_(1,int,FX::Int1,CG_INT1)
_(2,int,FX::Int2,CG_INT2)
_(3,int,FX::Int3,CG_INT3)
_(4,int,FX::Int4,CG_INT4)
_(1,float,float,CG_FLOAT)
_(1,float,FX::Float1,CG_FLOAT1)
_(2,float,FX::Float2,CG_FLOAT2)
_(3,float,FX::Float3,CG_FLOAT3)
_(4,float,FX::Float4,CG_FLOAT4)
_(2*2,float,FX::Float2x2,CG_FLOAT2x2)
_(3*3,float,FX::Float3x3,CG_FLOAT3x3)
_(4*4,float,FX::Float4x4,CG_FLOAT4x4)
_(0,FX::Sampler,FX::Sampler,CG_SAMPLER2D)
_(0,daeStringCP,xs::string,CG_STRING)
//Traits is recursive.
_(1,char,char,CG_CHAR)
//client data types// 
_(1,double,double,CG_DOUBLE)
//SCHEDULED FOR REMOVAL
#if 1==COLLADA_DOM_PRECISION
_(4*4,float,RT::Matrix,CG_FLOAT4x4)
#else
_(4*4,double,RT::Matrix,CG_DOUBLE4x4)
#endif
#undef _

//This is too compilicated.
template<class T, CGtype E=FX::DataTraits<T>::Base>
class DataType;
		
class Data
{
COLLADA_(public) //INTERFACE

	virtual CGtype GetType() = 0;
	virtual CGtype GetUnit() = 0;
	virtual size_t GetSize() = 0;
	virtual size_t GetSizeOf() = 0;
	virtual size_t GetSizeOfUnit() = 0;

COLLADA_(public) //INTERNALS

	/**OVERLOAD
	 * Load shader register.
	 */
	virtual void Load(FX::ShaderParam&) = 0;	
	/**OVERLOAD
	 * Load regular memory for numerical types.
	 * This implements @c To when types differ.
	 */
	virtual bool Load(size_t,CGtype,void*) = 0;

COLLADA_(public) //UTILITIES

	template<class T> inline T To()
	{
		T t[1] = {}; //0 or default initialize it.
		FX::DataType<T>::Load(this,*t); return *t;
	}
	//template<> daeName To<daeName>(); //GCC/C++

	template<class T> inline bool Is()
	{
		return nullptr!=As<T>();
	}

	//MSVC2010 needs all of this, but it builds too
	//slow. So probably this should just be removed.
	#ifdef _WIN32
	template<class T> struct _MSVC2010 //:(
	{
		typedef FX::DataType<T> type;
	};
	template<class T> inline typename _MSVC2010<T>::type*
	#else 
	template<class T> inline FX::DataType<T>*
	#endif
	/*template<class T> inline FX::DataType<T>*/As()
	{
		//GCC/C++ want specializations outside, MSVC2010
		//wants inside.
		return _As((T*)0); 
	}
	template<class T> FX::DataType<T> *_As(T*)
	{
		return dynamic_cast<FX::DataType<T>*>(this);
	}
	FX::DataType<FX::Sampler> *_As(FX::Sampler*)
	{
		switch(GetType())
		{
		case CG_SAMPLER1D: case CG_SAMPLERCUBE:
		case CG_SAMPLER2D: case CG_SAMPLERRECT:
		case CG_SAMPLER3D: case CG_UNKNOWN_TYPE:
		return (FX::DataType<FX::Sampler>*)this;
		default:; //-Wswitch
		}
		return nullptr;
	}
};
template<> inline daeName FX::Data::To<daeName>()
{
	//THIS ASSUMES ALL STRINGS ARE STRING-REFS.
	daeName ref; ref.string = To<xs::string>();
	if(ref.string!=nullptr)
	ref.extent = ((daeStringRef*)&ref.string)->size();
	return ref;
}

template<class T, CGtype E> 
/**
 * This is not a friendly data type if you are an
 * end-user. The dependence on the Cg API will be
 * removed or replaced with home grown enumerants.
 */
class DataType : public FX::Data
{
COLLADA_(public)
	/**
	 * Previoiusly "Data."
	 */
	T Value;

	DataType(){}
	DataType(const T &v):Value(v){}
	
	typedef FX::DataTraits<T> Traits;

	typedef typename Traits::Unit Unit;

	static const size_t Size = Traits::Size;

	static const CGtype Cg = E;

	static const CGtype Cg_Unit = FX::DataTraits<Unit>::Base;

COLLADA_(public) //FX::Data methods

	virtual CGtype GetType(){ return E; }	
	virtual CGtype GetUnit(){ return Cg_Unit; }
	virtual size_t GetSize(){ return Size; }
	virtual size_t GetSizeOf(){ return sizeof(T); }
	virtual size_t GetSizeOfUnit(){ return sizeof(Unit); }	

	//FX::ShaderParam::Apply calls this.
	virtual void Load(FX::ShaderParam&);

COLLADA_(public) //Make <annotate> etc. easy to work with.

	//FX::Data::To calls this.
	static bool Load(FX::Data *p, T &t)
	{
		if(0==Traits::Size) 
		{
			switch(E)
			{
			case CG_SAMPLER1D:
			case CG_SAMPLER2D:
			case CG_SAMPLER3D:
			case CG_SAMPLERCUBE:
			case CG_SAMPLERRECT:
			case CG_UNKNOWN_TYPE:

				if(p->Is<FX::Sampler>())
				break; return false;

			case CG_STRING:

				if(E!=p->GetType())
				*(xs::string*)&t = "Nonstring Data"; 
				else break; return false;

			default:; //-Wswitch
			}
			t = ((DataType*)p)->Value; return true;
		}
		if(E==p->GetType())
		{
			t = ((DataType*)p)->Value; return true;
		}
		else return p->Load(Size,Cg_Unit,&t);
	}
	virtual bool Load(size_t iN, CGtype u, void *t)
	{	
		//daeFig is because GCC/C++ won't facilitate explicit-specialization.
		return _Load2(daeFig<Size>(),iN,u,(char*)t);
	}
	template<int Size> bool _Load2(daeFig<Size>, size_t iN, CGtype u, char *t)
	{	
		iN = std::min<size_t>(iN,Size); daeCTC<Size!=0>();
		for(size_t i=0;i<iN;i++) 
		{
			Unit &p = ((Unit*)&Value)[i]; switch(u)
			{
			#define _(x,y) COLLADA_SUPPRESS_C(4800)\
			case CG_##x: *(y*)t = (y)p; t+=sizeof(y); break;
			_(BOOL,bool)_(INT,int)_(FLOAT,float)_(DOUBLE,double)
			#undef _
			default:; //-Wswitch
			}
		}
		return true;
	}
	bool _Load2(daeFig<0>, size_t iN, CGtype u, char *t)
	{
		if(E!=CG_STRING) return false;		
		xs::string hack = *(xs::string*)&Value;
		daeIStringBuf buf(hack,hack+strlen(hack));
		std::istream src(&buf);
		for(size_t i=0;i<iN;i++)
		{
			switch(u)
			{
			#define _(x,y) \
			case CG_##x: src>>*(y*)t; t+=sizeof(y); break;
			_(BOOL,bool)_(INT,int)_(FLOAT,float)_(DOUBLE,double)
			#undef _
			default:; //-Wswitch
			}
			if(src.fail()) return i!=0;
		}
		return true;
	}
};
typedef DataType<bool> DataBool;
typedef DataType<FX::Bool1> DataBool1;
typedef DataType<FX::Bool2> DataBool2;
typedef DataType<FX::Bool3> DataBool3;
typedef DataType<FX::Bool4> DataBool4;
typedef DataType<int> DataInt;
typedef DataType<FX::Int1> DataInt1;
typedef DataType<FX::Int2> DataInt2;
typedef DataType<FX::Int3> DataInt3;
typedef DataType<FX::Int4> DataInt4;
typedef DataType<float> DataFloat;
typedef DataType<FX::Float1> DataFloat1;
typedef DataType<FX::Float2> DataFloat2;
typedef DataType<FX::Float3> DataFloat3;
typedef DataType<FX::Float4> DataFloat4;
typedef DataType<FX::Float2x2> DataFloat2x2;
typedef DataType<FX::Float3x3> DataFloat3x3;
typedef DataType<FX::Float4x4> DataFloat4x4;
typedef DataType<FX::Sampler,CG_SAMPLER1D> DataSampler1D;
typedef DataType<FX::Sampler> DataSampler2D;
typedef DataType<FX::Sampler,CG_SAMPLER3D> DataSampler3D;
typedef DataType<FX::Sampler,CG_SAMPLERCUBE> DataSamplerCUBE;
typedef DataType<FX::Sampler,CG_SAMPLERRECT> DataSamplerRECT;
typedef DataType<FX::Sampler,CG_UNKNOWN_TYPE> DataSamplerDEPTH;
typedef DataType<xs::string> DataString;

//-------.
	}//<-'
}

#endif //__COLLADA_FX__DATA_H__
/*C1071*/
