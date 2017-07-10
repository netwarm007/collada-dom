/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__DATA_H__
#define __COLLADA_RT__DATA_H__

#include "CrtMemory.h"
#include "CrtMatrix.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'

/**
 * This object holds and <up_axis> and <unit> pair.
 * It's compressed, since any node with an <asset>
 * will need a copy, and because it will be 0 most
 * of the time, and this makes comparisons trivial.
 */
struct Asset_Index
{
	unsigned char _i;
	operator size_t()const{ return _i; }
	Asset_Index();
};

/**
 * This is class will be removed when @c daeDatabase
 * replaces @c RT::DBase. The DOM component makes it
 * redundant.
 */
class Base
{
COLLADA_(public)
	
	//SCHEDULED FOR REMOVAL
	daeName Id,Sid; xs::string DocURI;	

	/**
	 * This is currently whatever @c RT::Asset is
	 * when @c RT::Base::Base() is called.
	 */
	const RT::Asset_Index Asset;
	
COLLADA_(public)

	Base():Id(0),Sid(0),DocURI(){}
	virtual ~Base(){}
};
inline xs::string DocURI(const DAEP::Element *e)
{
	return dae(e)->getDoc()->getDocURI()->data();
}

//Since Collada08 was introduced this became a helper template.
template<class array, class source>
/**INTERNAL
 * This is a new class to try to formalize usage of <accessor>
 * elements.
 * @see @c RT::Accessor05.
 * @see @c RT::Accessor08.
 */
class Accessor : public array
{
COLLADA_(public)

	void bind(){ array::operator=(nullptr); }

	const_daeDocumentRef document;	

	typename source::technique_common::accessor accessor;

	Accessor(const daeDocument *doc):document(doc)
	{}
	Accessor(const DAEP::Element *e):document(dae(e)->getDocument())
	{}
	template<class input>
	/**
	 * This avoids confusion between shared/unshared inputs.
	 * @return Returns <input semantic> if the array exists.
	 */
	RT::Name bind(const input &in)
	{
		accessor = document->idLookup<source>
		(in->source)->technique_common->accessor;
		array::operator=(xs::anyURI(accessor->source->*"",document).get<array>());
		return in->semantic;
	}

	/**
	 * This returns the X Y or Z sense of the 1-D param or
	 * for 2-D it returns X if YZ or Y if XZ or Z if XY in
	 * order to communicate which two X Y or Z are present.
	 */
	RT::Up sense()
	{
		if(accessor==nullptr) return 0;
		size_t d = accessor->param.size();		
		int x = 0, y = 0, z = 0, out = 0; //X
		for(size_t i=0;i<d;i++)
		{
			daeName name = accessor->param[i]->name;
			if(x==0&&"X"==name){ x++; out = 0; }
			if(y==0&&"Y"==name){ y++; out = 1; }
			if(z==0&&"Z"==name){ z++; out = 2; }
		}
		if(d==2) out = z==0?2:y==0?1:0; return out; //XY	
	}

	#define COLLADA_RT_ACCESSOR_i_s_iN(x) \
	size_t i = accessor->offset;\
	size_t s = accessor->stride;\
	size_t iN = i+accessor->count*s; assert(s>=x); (void)iN;\
	Adaptor<T> array_value(src_array);
	template<class T> struct Adaptor
	{	
		typedef const daeArray<typename T::__COLLADA__Atom> Array;
		const Array *operator->(){ return &ref; }
		const Array &ref; Adaptor(const T &t):ref(t){}
	};
	void _out_of_range()
	{
		daeEH::Error<<"<accessor> index is out-of-range "<<accessor->source;
	}

	template<class S> void get(S &dst)
	{
		if(*this!=nullptr) 
		get(dst,array::operator->()->value);
	}
	template<class S, class T>
	void get(std::vector<S> &dst, const T &src_array)
	{
		COLLADA_RT_ACCESSOR_i_s_iN(0)
		dst.reserve(dst.size()+accessor->count);
		for(S tmp;i<iN;i+=s) 		
		if(1==array_value->get1at(i,tmp)) 
		dst.push_back(tmp); else return _out_of_range();
	}

	template<class S> void get16(S &dst)
	{
		typename source::float_array fa =
		dae(*this)->template a<typename source::float_array>();
		if(fa!=nullptr) get16(dst,fa->value);
	}
	template<class S, class T>
	void get16(std::vector<S> &dst, const T &src_array)
	{
		COLLADA_RT_ACCESSOR_i_s_iN(16)
		size_t j = dst.size();
		dst.resize(j+accessor->count);		
		for(;i<iN;i+=s,j++)
		{
			RT::Matrix &m = dst[j];
			if(4!=array_value->get4at(i+0,m[M00],m[M10],m[M20],m[M30])
			 ||4!=array_value->get4at(i+4,m[M01],m[M11],m[M21],m[M31])
			 ||4!=array_value->get4at(i+8,m[M02],m[M12],m[M22],m[M32])
			 ||4!=array_value->get4at(i+12,m[M03],m[M13],m[M23],m[M33]))
			{
				dst.resize(j); return _out_of_range();				
			}
		}
	}

	template<class S> int get1at(size_t at, S &dst)
	{
		assert(*this!=nullptr);
		return get1at(at,dst,array::operator->()->value);
	}
	template<class S, class T>
	int get1at(size_t at, S &dst, const T &src_array)
	{
		COLLADA_RT_ACCESSOR_i_s_iN(0)
		if(1==array_value->get1at(i+at*s,dst)) 
		return 1; _out_of_range(); return 0;
	}	
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__DATA_H__
/*C1071*/
