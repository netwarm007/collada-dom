/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_SMART_REF_H__
#define __COLLADA_DOM__DAE_SMART_REF_H__

#include "daeRefCountedObj.h"

COLLADA_(namespace)
{//-.
//<-'

//2.5 SIGNIFICANTLY RELAXES daeSmartRef TO MAKE IT BROADLY MORE
//AVAILABLE FOR INTERNAL USE, BY BREAKING CYCLICAL DEPENDENCIES.
//AS SUCH IT ACCEPTS ANY KIND OF T FOR PURPOSES OF CONSTRUCTION.
/**
 * The @c daeSmartRef template class automates reference counting for
 * objects derived from @c daeObject (previously daeRefCountedObj.)
 *
 * REQUIREMENTS
 * ============
 * This class should act like a pointer. It shouldn't overload operators
 * in such a way that makes it behave differently from a pointer. 
 * (Not even to get rid of nagging compiler warnings.)
 */
template<class T> class daeSmartRef
{				  
COLLADA_(public) //QUIRKS	
	/**WARNING
	 * Compacting ~ operator
	 * @c operator~ is used to compact @c daeDOM::_closedDocs.
	 * @return Returns @c true if the reference was released as 
	 * a result.
	 * @warning It's dangerous to use this strategy, because if two
	 * holders do so--and conditions do not change--they are assured
	 * to deadlock.
	 * THE WISDOM (OR LACK THEREOF) OF USING ~ FOR THIS DEBATABLE.
	 */
	bool operator~(){ return _ptr->_compact()&&(_ptr=nullptr,true); }

	/**WORKAROUND
	 * @c __COLLADA__T is a "loopback" mechanism.
	 * It cannot be @c __COLLADA__Element because @a T is an incomplete type.
	 * So, @c daeSmartRef<T>::__COLLADA__T::__COLLADA__Element is
	 * the same as @c T::__COLLADA__T::__COLLADA__Element; and so templates can
	 * this way work with @c T or @c daeSmartRef<T> transparently.
	 *
	 * @c __COLLADA__POD indicates @c this wraps plain-old-data.
	 */
	typedef T __COLLADA__T;

COLLADA_(public) //daeArray traits

	typedef const daeObject __COLLADA__Object;
	template<class S>
	inline void __COLLADA__place(const daeObject *obj, const S &cp)
	{
		new(this) daeSmartRef(cp); 
	}
	static void __COLLADA__move(const daeObject *obj, daeSmartRef *lv, daeSmartRef *rv, size_t iN)
	{
		memcpy(lv,rv,iN*sizeof(daeSmartRef)); //raw-copyable style move.
	}	

COLLADA_(public) //Funky constructors

  ////////////////////////////////////////////////////////////////////
  //MSVC2013 requires this. Not sure it's legit. Assignment is fine.//
  //http://stackoverflow.com/questions/20008689/why-is-a-conversion-//
  //operator-not-called-when-using-initialization-syntax-and-why-doe//
  ////////////////////////////////////////////////////////////////////

	template<class U, int N> //BOTH class T & U MUST BE DEFINED.
	/**
	 * There seems to be a difference between () and = construction.
	 */
	daeSmartRef(dae_Array<U,N> &cp)
	{
		U *u = cp; new(this) daeSmartRef(u); //Doing in body, just to be safe.
	}
	template<class U, int N> //BOTH class T & U MUST BE DEFINED.
	/**CONST-FORM
	 * There seems to be a difference between () and = construction.
	 */
	daeSmartRef(const dae_Array<U,N> &cp)
	{
		const U *u = cp; new(this) daeSmartRef(u); //Doing in body, just to be safe.
	}

	template<class U> //BOTH class T & U MUST BE DEFINED.
	/**
	 * Not entirely sure why this is, but @c COLLADA_DOM_3__struct__daeSmartRef 
	 * based refs want to use the @c daeSmartRef based constructor without this.
	 */
	explicit daeSmartRef(const daeDocRoot<U> &cp)
	{
		T *t; t = cp; new(this) daeSmartRef(t); //Doing in body, just to be safe.
	}

COLLADA_(public) //Factory constructors

	//class T MUST BE DEFINED.
	/**
	 * @c explicit 2.5 T Factory Constructor
	 *
	 * @todo @a DOM will eventually be a DAEP::DOM type.
	 */
	explicit daeSmartRef(/*const*/ daeDOM &DOM) 
	:_ptr((daeObject*)DOM._add<daeConstOf<int,T>::type>())
	{
		const DAEP::Object *upcast = (T*)nullptr; _ptr->_ref();
	}
	//This affords code the courtesy of not dereferencing.
	template<class T> struct _NonDOM{ typedef daeDOM type; };
	template<> struct _NonDOM<daeDOM>{ typedef class undefined type; };
	template<> struct _NonDOM<const daeDOM>{ typedef class undefined type; };
	//class T MUST BE DEFINED.
	/**WARNING
	 * @c explicit 2.5 T Factory Constructor
	 *
	 * @todo @a DOM will eventually be a DAEP::DOM type.
	 * @warning @a DOM cannot be @c nullptr. (It's not the shared-pointer's subject.)
	 */
	explicit daeSmartRef(/*const*/ typename _NonDOM<T>::type *DOM) 
	:_ptr((daeObject*)DOM->_add<daeConstOf<int,T>::type>())
	{
		const DAEP::Object *upcast = (T*)nullptr; _ptr->_ref();
	}	

COLLADA_(public) //Non-factory constructors
	/**
	 * Default Constructor
	 */
	inline daeSmartRef() : _ptr(nullptr){}	
	/**
	 * Destructor
	 */
	inline ~daeSmartRef(){ _ptr->_release(); }
	/**
	 * Constructor
	 * @param ptr a pointer to an object of the same template type.
	 */
	daeSmartRef(T *ptr) : _ptr((daeObject*)ptr)
	{
		_ptr->_ref();
	}	
	template<class U> //BOTH class T & U MUST BE DEFINED.
	/**
	 * Constructor
	 * @param ptr a pointer to an object of a different template type.
	 */
	daeSmartRef(U *ptr):_ptr((daeObject*)ptr)
	{
		_upcast(ptr,(T*)nullptr); _ptr->_ref();
	}			
	/** Implements DAEP::Element branch from DAEP::Object logic. */
	template<class U> void _upcast(U *ptr, const daeObject*)
	{
		daeConstOf<T,DAEP::Object>::type *upcast = ptr;
	}
	/** Implements DAEP::Element branch from DAEP::Object logic. */
	template<class U> void _upcast(U *ptr, const daeElement*)
	{
		daeConstOf<T,daeElement>::type *upcast = dae(ptr);
	}
	template<class U> void _upcast(U *ptr,...)
	{
		T *upcast = ptr; 
	}
	template<class U> //BOTH class T & U MUST BE DEFINED.
	/**
	 * Copy Constructor that will convert from one template to the other.
	 * @param smartRef a daeSmartRef to the object to copy from.
	 */	
	daeSmartRef(const daeSmartRef<U> &cp) : _ptr((daeObject*&)cp)		
	{
		_upcast((U*)nullptr,(T*)nullptr); _ptr->_ref();
	}
	/**
	 * Copy Constructor
	 * @param smartRef a daeSmartRef of the same template type to copy from
	 */
	daeSmartRef(const daeSmartRef &smartRef) : _ptr((daeObject*&)smartRef)
	{	
		_ptr->_ref();
	}
	
COLLADA_(public) //OPERATORS

	//R is expanding this to cover daeDocRoot.
	template<template<class> class R, class U> //BOTH class T & U MUST BE DEFINED.
	/**
	 * Overloaded assignment operator which will convert between template types.
	 * @param smartRef a daeSmartRef to the object to copy from.
	 * @return Returns a reference to this object.
	 * 
	 * @see @c daeElement class template specializations at the end of this file.
	 */	
	inline daeSmartRef &operator=(const R<U> &cp)
	{
		//Reminder: cp can be daeDocRoot, which converts to more than one pointer.
		T *ptr = cp;
		dae(ptr)->_ref(); _ptr->_release(); 
		_ptr = (daeObject*)ptr; return *this;
	}
	/**C++ (REQUIRED)
	 * Overloaded assignment operator.
	 * @param other a daeSmartRef to the object to copy from.  Must be of the same template type.
	 * @return Returns a reference to this object.
	 */
	inline daeSmartRef &operator=(const daeSmartRef &cp)
	{
		daeObject *ptr = cp._ptr;
		dae(ptr)->_ref();
		_ptr->_release();
		_ptr = ptr; return *this;
	}

	/**
	 * Overloaded assignment operator.
	 * @param ptr a pointer to the object to copy from.  Must be of the same template type.
	 * @return Returns a reference to this object.
	 */
	inline daeSmartRef &operator=(T *ptr)
	{
		dae(ptr)->_ref();
		_ptr->_release();
		_ptr = (daeObject*)ptr;
		return *this;
	}
	 	
	template<class U, int N> //BOTH class T & U MUST BE DEFINED.
	/**
	 * There seems to be a difference between () and = construction.
	 */
	daeSmartRef &operator=(dae_Array<U,N> &cp)
	{
		return operator=((U*)cp);
	}
	template<class U, int N> //BOTH class T & U MUST BE DEFINED.
	/**CONST-FORM
	 * There seems to be a difference between () and = construction.
	 */
	daeSmartRef &operator=(const dae_Array<U,N> &cp)
	{
		return operator=((const U*)cp);
	}

	//class T MUST BE DEFINED.
	/**
	 * Overloaded member selection operator.
	 * @return a pointer of the template class to the object.
	 */
	inline T *operator->()const
	{
		//Disabling this assert so the new deferred dereferencing
		//feature set can be used with ->.
		//dae_Array::operator->() and (Type*) conversion operator,
		//DAEP::InnerValue::operator->*()
		//assert(nullptr!=_ptr); 
		return static_cast<T*>((DAEP::Object*)_ptr);
	}

	/**
	 * Overloaded cast operator.
	 * @return Returns an @a T pointer.
	 */
	inline operator T*()const{ return static_cast<T*>((DAEP::Object*)_ptr); }

	/**
	 * Overloaded dereference operator.
	 * @return Returns an @a T reference.
	 */
	inline T &operator*()const{ return (T&)*static_cast<T*>((DAEP::Object*)_ptr); }
	
	/**
	 * @warning Might generate double-const warnings.
	 * Post-2.5 implicit conversion to const reference.
	 */
	inline operator daeSmartRef<const T>&(){ return *(daeSmartRef<const T>*)this; }
	/**CONST-FORM
	 * @warning Might generate double-const warnings.
	 * Post-2.5 implicit conversion to const reference.
	 */
	inline operator const daeSmartRef<const T>&()const{ return *(daeSmartRef<const T>*)this; }

COLLADA_(public) //DEPRECATIONS

	#ifndef COLLADA_NODEPRECATED
	/**
	 * Function that returns a pointer to object being reference counted.
	 * @return the object being reference counted.
	 */
	inline T *cast()const{ return (T*)_ptr; }
	//SCHEDULED FOR REMOVAL
	//(seems very awkward to do)
	template<class U> //BOTH class T & U MUST BE DEFINED.
	/**LEGACY, UNUSED (INTERNALLY)
	 * Static cast function.
	 * @param smartRef a smartRef to cast from
	 * @return a pointer to an object of this template class.
	 */	
	inline static T *staticCast(const daeSmartRef<U> &smartRef)
	{
		return static_cast<T*>(smartRef._ptr);
	}
	#endif //COLLADA_NODEPRECATED

COLLADA_(protected) //DATA-MEMBER

	template<class T> friend class daeSmartRef;
	/* The pointer to the element which is being reference counted. */
	daeObject *_ptr;
};
template<>
template<template<class> class R, class U>
/**CLASS TEMPLATE-SPECIALIZATION
 * This converts from @c DAEP::Element to @c daeElement for @c xs::any.
 */	
inline daeElementRef &daeElementRef::operator=(const R<U> &cp)
{
	daeElement *ptr = dae(cp);
	dae(ptr)->_ref(); _ptr->_release(); 
	_ptr = (daeObject*)ptr; return *this;
}
template<>
template<template<class> class R, class U>
/**CLASS TEMPLATE-SPECIALIZATION
 * This converts from @c DAEP::Element to @c daeElement for @c xs::any.
 */	
inline const_daeElementRef &const_daeElementRef::operator=(const R<U> &cp)
{
	const daeElement *ptr = dae(cp);
	dae(ptr)->_ref(); _ptr->_release(); 
	_ptr = (daeObject*)ptr; return *this;
}

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_SMART_REF_H__
/*C1071*/