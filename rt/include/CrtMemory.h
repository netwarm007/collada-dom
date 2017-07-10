/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__MEMORY_H__
#define __COLLADA_RT__MEMORY_H__

#include "CrtTypes.h"

class btDiscreteDynamicsWorld;

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'

//2017: These were here already. As long as new/delete is used
//they can be left alone. The FX component just uses new/delete.

//SCHEDULED FOR REMOVAL
#define COLLADA_RT_new(c) new c //PS3 might want a nullptr check?
//SCHEDULED FOR REMOVAL?
//This is useful for std::for_each in lieu of default_delete.
//#define COLLADA_RT_delete(p) 
template<class T> void COLLADA_RT_delete(T* &p)
{
	delete p; p = nullptr;
}
//SCHEDULED FOR REMOVAL
#define COLLADA_RT_array_new(c,n) new c[n] //PS3 might want a nullptr check?
//SCHEDULED FOR REMOVAL
#define COLLADA_RT_array_delete(p) (delete[]p,p=nullptr)

inline void Assert(daeString string, bool f=false)
{
	if(!f) daeEH::Error<<"Assertion Failed: "<<string; assert(f);
}

/**2017
 * Adding this so the NO_BULLET pointers don't leak memory.
 * 
 * Quoting the library's principal author:
 * http://www.bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=9&t=4618
 * (They appear to be the same person who had written CrtPhysics.cpp.)
 *
 * "Memory management rules are simple: the one who allocates memory is 
 *  responsible for deletion."
 * "For collision shapes, that can be shared among several objects, it 
 *  is best to keep a separate array and delete them at the end."
 * "Also, the order of deletion is reversed of creation. 
 * Thanks,
 * Erwin"
 */
struct Memory
{
	struct ptr
	{
		void *p,*deleter;
		template<class T> 
		//BT_DECLARE_ALIGNED_ALLOCATOR???
		//http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?t=10316
		static void _delete_f(T *p)
		{
			//THIS IS MESSED UP???
			//The Bullet Physics library doesn't do any
			//memory management, yet it overloads new/delete
			//for some classes but seemingly not all???
			//Calling delete invokes its overload.
			//delete p; 
			_destruct(p); delete (void*)p;
		}		
		/**
		 * This is for the bulletphysics.org library's classes.
		 * It should be configurable if used otherwise.
		 * @note 16-byte alignment isn't required, but there's
		 * no obvious way to disable it in btScalar.h.
		 */
		template<class T> 
		static void _delete_g(T *p)
		{
			//Same as above with 8 realignment required.
			_destruct(p); delete((char*)p-8); 
		}		
		template<class T> ptr(T *q):p(q)
		{
			assert(0==((size_t)q&7));
			bool f = 0==((size_t)q&8);
			if(!f) (char*&)p+=8; //Strict aliasing issues.
			deleter = (void*)(f?_delete_f<T>:_delete_g<T>); 
		}		
		//~ptr //ptr needs to be STL container friendly.
		void delete_p(){ ((void(*)(void*))deleter)(p); }		

		//THIS IS JUST TO WORK AROUND BUGGY DESTRUCTORS.
		template<class T> static void _destruct(const T *p){ p->~T(); }
		//This problem persists even after building new libraries.
		static void _destruct(const btDiscreteDynamicsWorld*)
		{				  
			//This destructor is crashing. The library will have to be
			//built to get to the bottom of it.
			#ifdef NDEBUG
			daeEH::Warning<<"Memory leak on ~btDiscreteDynamicsWorld()";
			#endif
		}
	};
	std::vector<ptr> Nuke;
	#define _(x,y,...) \
	template<class S __VA_ARGS__> S *New x\
	{\
		S *p = (S*)operator new(sizeof(S)+8);\
		Nuke.push_back(p); return new(Nuke.back().p) S y;\
	}
	//TODO? C++98 is not really a requirement for RT & FX.
	//HACK: GCC wants &&. Maybe in C++98 mode & would work.
	_((),(),)
	_((T&&t),(t),, class T)
	_((T&&t,U&&u),(t,u),, class T, class U)
	_((T&t,U&u,V&v),(t,u,v),, class T, class U, class V)
	_((T&t,U&u,V&v,W&w),(t,u,v,w),, class T, class U, class V, class W)
	_((T&t,U&u,V&v,W&w,X&x),(t,u,v,w,x),, class T, class U, class V, class W, class X)
	#undef _
	
	//"Also, the order of deletion is reversed of creation."
	~Memory()
	{
		for(size_t i=Nuke.size();i-->0;) Nuke[i].delete_p();
	}
	void Delete_back(void *p)
	{
		assert(p==Nuke.back().p); Nuke.back().delete_p(); Nuke.pop_back(); 
	}
};

//-------.
	}//<-'
}

#endif //__COLLADA_RT__MEMORY_H__
/*C1071*/
