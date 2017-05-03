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
	namespace FX
	{//-.
//<-----'
				  
class Data
{
COLLADA_(public)

	virtual ~Data(){}
	virtual CGtype GetType() = 0;
	virtual void Apply(FX::Param *param) = 0;
	virtual void Apply(FX::Annotate *annotate) = 0;
};

template<class T>
/**INTERNAL 
 * This design could use more thought.
 */
struct DataMaker
{	
	FX::NewParamable *p;

	xs::ID sid; typename T::arg2 aux; 

	T *&o; const daeContents &c; void MakeData();
};	  
template<class S, class T>
inline void MakeData(S* &o, const T &e, FX::NewParamable *p, xs::ID a1, typename S::arg2 a2=nullptr)
{
	FX::DataMaker<S> dm = {p,a1,a2,o,e->content}; FX::MakeData2(dm);
}
extern void MakeData2(DataMaker<FX::Annotate>&);
extern void MakeData2(DataMaker<FX::NewParam>&);
extern void MakeData2(DataMaker<FX::SetParam>&);
extern void MakeData2(DataMaker<FX::BindParam>&);

#ifdef NDEBUG
#error These may not be needed/exposed.
#endif
template<class T, CGtype E> 
class DataType : public FX::Data
{
COLLADA_(public)
	/**
	 * Previoiusly "Data."
	 */
	T Value;

	//SCHEDULED FOR REMOVAL?
	static const CGtype Cg = E;

COLLADA_(public) //SCHEDULED FOR REMOVAL?

	virtual CGtype GetType(){ return Cg; }

	virtual void Apply(FX::Param *param);
	virtual void Apply(FX::Annotate *annotate);	
};
typedef DataType<bool,CG_BOOL> DataBool;
typedef DataType<FX::Bool1,CG_BOOL1> DataBool1;
typedef DataType<FX::Bool2,CG_BOOL2> DataBool2;
typedef DataType<FX::Bool3,CG_BOOL3> DataBool3;
typedef DataType<FX::Bool4,CG_BOOL4> DataBool4;
typedef DataType<int,CG_INT> DataInt;
typedef DataType<FX::Int1,CG_INT1> DataInt1;
typedef DataType<FX::Int2,CG_INT2> DataInt2;
typedef DataType<FX::Int3,CG_INT3> DataInt3;
typedef DataType<FX::Int4,CG_INT4> DataInt4;
typedef DataType<float,CG_FLOAT> DataFloat;
typedef DataType<FX::Float1,CG_FLOAT1> DataFloat1;
typedef DataType<FX::Float2,CG_FLOAT2> DataFloat2;
typedef DataType<FX::Float3,CG_FLOAT3> DataFloat3;
typedef DataType<FX::Float4,CG_FLOAT4> DataFloat4;
typedef DataType<FX::Float2x2,CG_FLOAT2x2> DataFloat2x2;
typedef DataType<FX::Float3x3,CG_FLOAT3x3> DataFloat3x3;
typedef DataType<FX::Float4x4,CG_FLOAT4x4> DataFloat4x4;
typedef DataType<FX::Sampler,CG_SAMPLER1D> DataSampler1D;
typedef DataType<FX::Sampler,CG_SAMPLER2D> DataSampler2D;
typedef DataType<FX::Sampler,CG_SAMPLER3D> DataSampler3D;
typedef DataType<FX::Sampler,CG_SAMPLERCUBE> DataSamplerCUBE;
typedef DataType<FX::Sampler,CG_SAMPLERRECT> DataSamplerRECT;
typedef DataType<FX::Sampler,CG_UNKNOWN_TYPE> DataSamplerDEPTH;
typedef DataType<xs::string,CG_STRING> DataString;

//-------.
	}//<-'
}

#endif //__COLLADA_FX__DATA_H__
/*C1071*/