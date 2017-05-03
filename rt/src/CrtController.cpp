/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <RT.pch.h> //PCH

#include "CrtMatrix.h"
#include "CrtNode.h"
#include "CrtRender.h"
#include "CrtGeometry.h"
#include "CrtAnimation.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
	
//NOT THREAD-SAFE
#ifdef NDEBUG
#error This can benefit from SSE optimization.
#error (It's happening 30/60 times per second.)
#endif
static std::vector<RT::Matrix> CrtController_mats(1); 	
/**NOT THREAD-SAFE
 * This implements much of RT::Skin::Update_VBuffer2().
 */
struct CrtController_skin
{
	RT::Skin &skin;
		
	//TODO: In unusual circumstances it might be necessary
	//to compute "inverse-transposed" matrices for normals.
	std::vector<RT::Matrix> &mats,matsIT;
	
	CrtController_skin(RT::Skin &skin, RT::Stack_Data **joints)
	:skin(skin),mats(CrtController_mats),matsIT(mats),joints(joints)
	{
		 //CrtController_mats 		
		assert(!mats.empty()); //C++98/03
		mats.resize(std::max(mats.size(),skin.Joints.size()));
	}

	template<int Stride> void VBuffer2()
	{
		size_t iN = skin.Weights.size();		

		float *o = RT::Main.Stack.VBuffer2.new_Memory;
		if(skin.Source==nullptr)
		{	
			RT::Geometry &g = *skin.Geometry;
			const RT::Float *n = nullptr;
			if(Stride>1) n = &g.Normals->value[g.Normals.Offset];
			const RT::Float *p = &g.Positions->value[g.Positions.Offset];
			VBuffer2<Stride>(iN,p,g.Positions.Stride,n,g.Normals.Stride);
		}
		else VBuffer2<Stride>(iN,o,3*Stride,o+3,3*Stride); //Recursive?
	}	
	RT::Stack_Data **joints;
	FX::Float3 BSMVert, transVert;
	FX::Float3 BSMNorm, transNorm;
	template<int Stride, class T>
	void VBuffer2(size_t iN, const T *p, size_t ps, const T *n, size_t ns)
	{
		FX::Float3 *in = (FX::Float3*)RT::Main.Stack.VBuffer2.new_Memory;
		FX::Float3 *out = in;
		RT::Skin_Weight *Weights = &skin.Weights[0];			
		//ALGORITHM
		//The loader has pre-normalized the weights.
		//When they exceed 1; the next vertex is up.	
		float weight = Weights[0].Weight.x, weightSum; 		
		size_t j,i=0; //goto i_is_0;		
		goto i_is_0; for(;i<iN;i++)
		{
			weight = Weights[i].Weight.x;
			weightSum+=weight; if(1<weightSum)
			{
				out+=Stride;

				i_is_0: weightSum = weight;

				out[0] = FX::Float3(0);
				if(Stride>1)        
				out[1] = FX::Float3(0);

				//Reminder: p may be a double pointer.
				#ifdef NDEBUG
				#error Skip transform step if identity?
				#endif				
				FX::Float3 v((float)p[0],(float)p[1],(float)p[2]); 
				p+=ps;				
				RT::MatrixTransform(skin.BindShapeMatrix,v,BSMVert);
				if(Stride>1) //Can't hurt.
				{
					v.x = (float)n[0]; v.y = (float)n[1]; v.z = (float)n[2]; 
					n+=ns;
					RT::MatrixTransform(skin.BindShapeMatrix,v,BSMNorm);			
				}
			}
						
			j = Weights[i].Joint; assert(j>=0); //-1 becomes unsigned.
			RT::Stack_Data *joint = j<=skin.Joints.size()?joints[j]:nullptr;
			#ifdef NDEBUG
			#error Skip this if known to be impossible?
			#endif
			if(joint!=nullptr) 
			{
				RT::MatrixTransform(mats[j],BSMVert,transVert);
				if(Stride>1) //Can't hurt.
				RT::MatrixRotate(matsIT[j],BSMNorm,transNorm);
			}
			else //2017: This is new. IT MAY OR MAY NOT BE CORRECT.
			{
				//-1 bones are said to use the <bind_shape_matrix>.
				transVert = BSMVert; 
				if(Stride>1) //Can't hurt.
				transNorm = BSMNorm;
			}

			out[0]+=transVert*weight; 
			if(Stride>1) //Can't hurt.
			out[1]+=transNorm*weight;
		}
		assert((out+Stride-in)==skin.Geometry->Vertices*Stride);
	}
};
void RT::Skin::Update_VBuffer2(RT::Stack_Data **joints)
{
	if(Source!=nullptr) Source->Update_VBuffer2(joints+Joints.size());
	if(Weights.empty()) return;
	
	//ALGORITHM
	//This helper class makes templates more manageable.
	CrtController_skin skin(*this,joints);	
	{	
		size_t i,iN = std::min(Joints.size(),Joints_INV_BIND_MATRIX.size());
		for(i=0;i<iN;i++)
		RT::MatrixMult(Joints_INV_BIND_MATRIX[i],joints[i]->Matrix,skin.mats[i]);
		while(i<Joints.size()) //Just in case.
		RT::MatrixCopy(joints[i]->Matrix,skin.mats[i++]);
	}
	if(Geometry->Normals==nullptr) skin.VBuffer2<1>();
	if(Geometry->Normals!=nullptr) skin.VBuffer2<2>();	
}

//NOT THREAD-SAFE
static std::vector<RT::Float> CrtController_weights(1);
/**NOT THREAD-SAFE
 * This implements much of RT::Morph::Update_VBuffer2().
 *
 * THIS IS MUCH EASIER THAN @c CrtController_skin SINCE
 * THE VALUES ARE SIMPLY ACCUMULATED, THE INPUTS AREN'T
 * EVER FROM THE PREVIOUSLY COMPUTED RESULTS.
 */
struct CrtController_morph
{
	RT::Morph &morph;

	//vN,iN are not really required, but this an algorithm.
	const size_t vN,iN;
	RT::Float w; size_t i;	
	const RT::Float nonzero;
	CrtController_morph(RT::Morph &morph)
	:morph(morph)
	,nonzero(0.00001f)
	,vN(morph.Geometry->Vertices)
	,iN(morph.MorphTargets.size())
	{	
		//COLLADA uses the concept of a "base mesh" but it
		//is really no different from another target. Most
		//systems won't make this distinction, since there
		//is a need to blend key frames when all are equal.

		//calculate one_minus_sum_of_weights		
		if(!morph.Using_RELATIVE_method)
		{
			w = 0;
			for(size_t i=0;i<iN;i++)
			w+=CrtController_weights[i];		
			w = 1-w;
		}
		else w = 1;
		
		//i is the initial morph-target.
		i = fabs(w)>=nonzero?0:1; 
		if(i!=0) //The weights are normalized?
		{
			//In an animation targets may have 0 weights.
			//(Likely only 2 target weights are nonzero.)
			while(i<=iN&&CrtController_weights[i-1]<nonzero) 
			i++;
		}
		if(i>=iN) i = 0; //All weights are 0?
	}

	template<int Stride> void VBuffer2()
	{
		//If this is not entered the <morph> is recursive.
		if(i!=0||morph.Source==nullptr)
		{
			RT::Geometry *g0 = morph.Geometry; 
			if(i!=0) g0 = morph.MorphTargets[i-1].Vertices;															
			VBuffer2_Op<'=',Stride>(*g0);
		}
				
		//i can be 0 here. <morph source> is not included.
		for(;i<iN;i++) 
		{
			//The Conformance Test Suite has examples with
			//morph-on-morph and morph-on-skin and so on. 
			//It doesn't say if the targets use the source,
			//-or if they are unaffected by the source. If
			//not then this is correct. And in that case it
			//seems like it should be possible for a target
			//to be another controller. They are IDREFs, so
			//it's not clear. Examples only target geometry.
			//
			//(Handling <skin> targets would be challenging.
			//Handling <morph> targets would just mean more
			//virtual targets, by way of decomposing a graph
			//into matching leaf <geometry> and weight pairs.)

			w = CrtController_weights[i];
			if(fabs(w)>=nonzero)
			VBuffer2_Op<'+',Stride>(*morph.MorphTargets[i].Vertices);
		}
	}
	template<int Op, int Stride> void VBuffer2_Op(RT::Geometry &g)
	{
		float *out = RT::Main.Stack.VBuffer2.new_Memory;

		const RT::Float *n;
		if(Stride>1) n = &g.Normals->value[g.Normals.Offset];
		const RT::Float *p = &g.Positions->value[g.Positions.Offset];
		size_t ps = g.Positions.Stride; 
		size_t ns = g.Normals.Stride;		
		for(size_t v=0;v<vN;v++)
		{
			//INCOMPLETE
			//Presently the skin/morph CPU vertex-buffer is expected
			//to hold only POSITION+NORMAL values. There's no reason
			//<morph> data could not or would not interpolate others.

			for(size_t i=0;i<3;i++) 
			{
				float f = float(p[i]*w); Op=='='?out[i]=f:out[i]+=f;
			}
			out+=3; p+=ps;
			if(Stride==1) continue;
			for(size_t i=0;i<3;i++)
			{
				float f = float(n[i]*w); Op=='='?out[i]=f:out[i]+=f;
			}
			out+=3; n+=ns;
		}
	}
};
void RT::Morph::Update_VBuffer2(RT::Stack_Data **joints)
{	
	//SCHEDULED FOR REMOVAL
	//These are copied in case they need to be
	//animated. The animation code expects the
	//animated data to be contiguous in memory.
	//Animating shouldn't clobber the defaults.
	CrtController_weights.resize(MorphTargets.size());
	for(size_t i=0;i<CrtController_weights.size();i++)
	CrtController_weights[i] = MorphTargets[i].Weight;
	for(size_t i=0;i<Animators.size();i++)
	Animators[i].Animate(&CrtController_weights[0]);

	//ALGORITHM
	//This helper class makes templates more manageable.
	CrtController_morph morph(*this);
	{			
		//RECURSIVE-ON-CONDITION
		//Recursively update the recursive <controller> 
		//if the first weight is nonzero or degenerated.
		if(Source!=nullptr&&morph.i==0)
		Source->Update_VBuffer2(joints);
	}
	if(Geometry->Normals==nullptr) morph.VBuffer2<1>();
	if(Geometry->Normals!=nullptr) morph.VBuffer2<2>();	
}


//-------.
	}//<-'
}

/*C1071*/