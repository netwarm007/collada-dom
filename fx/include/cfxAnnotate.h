/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_FX__ANNOTATE_H__
#define __COLLADA_FX__ANNOTATE_H__
		  
#include "cfxData.h"

COLLADA_(namespace)
{
	namespace FX
	{//-.
//<-----' 

/**wARNING
 * @warning The current implementation merges the
 * @c FX::Param to the @c FX::Data because it's a
 * chicken/egg situation. It doesn't hurt to fuse
 * the @c new and @c delete operation either. The
 * same goes for @c FX::Annotate.
 */
class Annotate
{
COLLADA_(public)

	FX::Data *Data;

	xs::ID Name;

	CGannotation Cg;

COLLADA_(public)

	typedef xs::ID arg2;
	Annotate(FX::DataMaker<FX::Annotate>*)
	:Data(Data),Name(Name),Cg(Cg)
	{ /*NOP*/ }

	void Apply(const FX::Effect *effect);
	void Apply(const FX::Param *param);
	void Apply(const FX::Pass *pass);
	void Apply(const FX::Technique *technique);
	void Apply(const FX::Shader *shader);
};

class Annotatable
{
COLLADA_(public)

	std::vector<FX::Annotate*> Annotations;

COLLADA_(public)

	template<class T>
	void Apply(T *p)
	{
		for(size_t i=0;i<Annotations.size();i++)	
		Annotations[i]->Apply(p);	
	}

	~Annotatable()
	{
		for(size_t i=0;i<Annotations.size();i++)
		delete Annotations[i];	
	}
};

//-------.
	}//<-'
}

#endif //__COLLADA_FX__ANNOTATE_H__
/*C1071*/