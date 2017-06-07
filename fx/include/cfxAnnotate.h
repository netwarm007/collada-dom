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

/**
 * "Standard Annotations and Semantics"
 * These are based on CgFX more or less.
 *
 * http://www.nvidia.com/object/using_sas.html
 */
enum SAS
{
	//These values are sorted according
	//to priority. 
	string_Space_World, //#1 priority.
	string_Space_View,
};

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
	
	FX::Data &OwnData()
	{
		return *(FX::Data*)(this+1); 
	}
	
	//daeName is to make "SAS" easier
	//to implement as an afterthought.
	//If we are serious there must be
	//an enum type member in addition.
	daeName Name;

	/**
	 * Some annotations are so common
	 * to demand for easy access. The
	 * enumerant may be just the name
	 * or it can be fully represented.
	 * Annotations are ordered in the
	 * @c int value of each @ FX::SAS.
	 *
	 * http://www.nvidia.com/object/using_sas.html
	 */
	FX::SAS SAS;

	template<class T>
	/** 
	 * @a T can be any nonclass types.
	 * It doesn't manufacture strings.
	 * @see @c Annotatable::Annotate().
	 */
	inline T Value(){ return OwnData().To<T>(); }
			 
COLLADA_(public)

	typedef xs::ID arg2;
	Annotate(){ /*NOP*/ } //Profile_COMMON
	Annotate(FX::DataMaker<FX::Annotate>*)
	:Name(Name)
	{
		_InitSAS();
	}
	void _InitSAS(); //See cfxParam.cpp.
	bool operator<(const FX::Annotate &cmp)const
	{
		return (int)SAS<(int)cmp.SAS;
	}

	/**WARNING
	 * @warning Camel-case support is limited to 
	 * camel-case strings. 
	 *
	 * Looks for hints in the form of substrings.
	 */
	inline bool SAS_substring(xs::string s)
	{
		xs::string p = Value<xs::string>();
		if(p==nullptr) return false;
		xs::string part = strstr(p,s+1);
		return (size_t)part>(size_t)p
		&&toupper(s[0])==toupper(part[-1]);
	}	
};

class Annotatable
{
COLLADA_(public)

	std::vector<FX::Annotate*> Annotations;

COLLADA_(public)
	 
	~Annotatable()
	{
		for(size_t i=0;i<Annotations.size();i++)
		delete Annotations[i];	
	}

	/**
	 * This tells if a spatial @c Semantic
	 * expects values in world space or in
	 * view space.
	 */
	inline bool Space_IsView()
	{	
		return !Annotations.empty()
		&&Annotations[0]->SAS==FX::string_Space_View;
	}
		
	/**
	 * Looks for hints in the form of substrings.
	 */
	inline bool SAS_substring(xs::string s)
	{
		for(size_t i=0;i<Annotations.size();i++)
		if(Annotations[i]->SAS_substring(s))
		return true; return false;
	}

	template<class T>
	/**CASE-SENSITIVE
	 * This makes extractions shorter.
	 * Not wanting anything too fancy.
	 */
	inline bool Annotate(daeName name, T &val)
	{
		for(size_t i=0;i<Annotations.size();i++)
		if(name==Annotations[i]->Name)
		{
			val = Annotations[i]->OwnData().To<T>(); 
			return true;
		}
		return false;
	}	
};

//-------.
	}//<-'
}

#endif //__COLLADA_FX__ANNOTATE_H__
/*C1071*/