/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM_INL__
#define __COLLADA_DOM_INL__
				
//This includes the entirety of the library.
#include "dae.h"
#include "dae/domAny.h"
#include "dae/daeMetaSchema.h"
#include "dae/daeErrorHandler.h"
#include "dae/daeIOPluginCommon.h"
#include "dae/daeStandardURIResolver.h"
					  
//Miscellaneous and circularly defined APIs.
COLLADA_(namespace)
{//-.
//<-'
	
//WARNING: THIS WON'T WORK IF THERE ARE MULTIPLE META.	
template<class S, class T>
/**LEGACY
 * This should not be used, and will only work correctly
 * if @a e belongs to the same @c XS::Schema as @a S.
 * you can define an empty release macro for @c daeSafeCast().
 * Nevertheless it is used, and the new @c daeElement::a()
 * is implemented by way of calling back to @c daeSafeCast<T>().
 * @see also @c daeElement::an().
 */
inline typename daeConstOf
<T,typename S::__COLLADA__T>::type *daeSafeCast(T *e)
{
	const daeElement *upcast = *e;
	if(e!=nullptr) if(daeUnsafe<S>(e))
	{
	#ifndef COLLADA_quiet_daeSafeCast
	assert(!"daeSafeCast() failed."); //2.5: Alert the user?
	#endif
	}
	else return (S*)e; return nullptr;
}
/**TEMPLATE-SPECIALIZATION Implements @c daeSafeCast(). */
template<class S> inline bool daeUnsafe(const daeElement *e)
{
	return _daeUnsafe2<daeConstOf<int,typename S::__COLLADA__T>::type>(e);
}
/**TEMPLATE-SPECIALIZATION Implements @c daeSafeCast(). */
template<template<class> class S> inline bool daeUnsafe(const daeElement *e)
{
	return _daeUnsafe2<daeConstOf<int,typename S::__COLLADA__T>::type>(e);
}
/**TEMPLATE-SPECIALIZATION Implements @c daeUnsafe(). */
template<class S> inline bool _daeUnsafe2(const daeElement *e)
{	
	#ifdef COLLADA_dynamic_daeSafeCast
	return dynamic_cast<const DAEP::Elemental<S>*>(e)==nullptr;
	#else
	return e->getMeta()!=daeGetMeta<S>();
	#endif
}
/**TEMPLATE-SPECIALIZATION Implements @c daeUnsafe(). */
template<> inline bool _daeUnsafe2<domAny>(const daeElement *e)
{
	return !e->_isAny();
}
/**TEMPLATE-SPECIALIZATION Implements @c daeUnsafe(). */
template<> inline bool _daeUnsafe2<daeObject>(const daeElement*){ return false; }
/**TEMPLATE-SPECIALIZATION Implements @c daeUnsafe(). */
template<> inline bool _daeUnsafe2<daeElement>(const daeElement*){ return false; }
/**TEMPLATE-SPECIALIZATION Implements @c daeUnsafe(). */
template<> inline bool _daeUnsafe2<DAEP::Element>(const daeElement*){ return false; }

/**
 * Extract the metadata by constructing an uninitialized "elemental"
 * in order to use its virtual-method-pointer. In theory optimizers
 * can elide construction of the object.
 *
 * It's debateable whether having this procedure as a virtual method
 * is for the best. It makes more sense with objects than elements. 
 * Elements are objects. When this was worked on initially, it wasn't
 * obvious that static inline methods could serve the same purpose. 
 * It would've been simple to do a test, but "I" had so much doubt that
 * I didn't bother. (I can't recall what my biases were...)
 *
 * It's trivial to have DAEP Elemental declare a static inline method, 
 * and let the virtual method wrap around that. It would be the best of
 * both worlds, in case there's some use to having the virtual methods,
 * -or just because elements should be valid objects.
 */
template<class T> inline const daeModel &daeGetModel()
{
	daeElement *nonconst_test = dae((typename T::__COLLADA__T*)nullptr);
	return DAEP::Elemental<typename T::__COLLADA__T>(DAEP::VPTR).__DAEP__Object__v1__model(); 
}
/**
 * Statically extract the @c daeMetaElement pointer. 
 * @tparam T must be based on @c daeElemental or @c DAEP::Elemental, or 
 * this template will not compile. This excludes @c domAny.
 */
template<class T> inline daeMeta &daeGetMeta()
{
	//Don't use daeOpaque here as long as daeModel converts to daeMeta*.
	return daeOpaque(&daeGetModel<typename T::__COLLADA__T>())[sizeof(typename T::__COLLADA__T)];
}
/**TEMPLATE-SPECIALIZATION
 * Since @c domAny is shared, its layout isn't known to clients.
 */
template<> inline daeMeta &daeGetMeta<domAny>()
{
	//Reminder: This lets "math:math" default to domAny.
	return daeGetModel<domAny>();
}
		
/**CIRCULAR-DEPENDENCY
 * @warning A @c daeDOM is never stored in a database.
 * @return Returns the @c daeDatabase storing the object, or
 * -returns @c nullptr if the object is not database stored.
 * @see ColladaDOM.inl header's definition.
 */
inline daeDatabase *daeObject::_getDBase()const
{
	return _isData()?&getDOM()->getDatabase():nullptr; 
}
/**LEGACY, CIRCULAR-DEPENDENCY
* Accessor to get the database associated with this document.
* @return Returns the database associated with this document.
*/
inline daeDatabase &daeDoc::getDatabase()const{ return getDOM()->getDatabase(); }
/**CIRCULAR-DEPENDENCY
 * This is newly added in case it's decided to store a copy of @c daeDOM::_database in
 * every element, alongside a copy of the @c daeMetaElement pointer. They are constant
 * for the entirety of the element's life.
 */
inline daeDatabase &daeElement::getDatabase()const{ return getDOM()->getDatabase(); }

template<> template<> 
/**KISS, CIRCULAR-DEPENDENCY 
 * Implements @c daeArray<daeContent>::clear().
 * Perhaps "__COLLADA__clear" is in order? Contents-arrays are special.
 * @see ColladaDOM.inl header's definition.
 */
inline void daeArray<daeContent>::_clear2<daeContent>()
{
	//If getObject() is nullptr it's likely to be @c XS::Choice::_solve().
	//In that case, clear() is not supported; call _clear2<void>() directly.
	assert(_au->_offset!=0); ((daeElement*)getObject())->__clear(*this); 
}
template<> template<> 
/** KISS, CIRCULAR-DEPENDENCY
 * Implements @c daeArray<daeContent>::clear(). 
 * @c __COLLADA__move() was going to be used, but when 
 * @c daeCursor was made to be an iterator, it became necessary to adjust it.
 * @see ColladaDOM_3.inl header's implementation.
 */
inline void daeArray<daeContent>::_grow2<daeContent>(size_t minCapacity)
{
	//If getObject() is nullptr it's likely to be @c XS::Choice::_solve().
	//In that case, clear() is not supported; call _clear2<void>() directly.
	assert(_au->_offset!=0); 
	//>= is used to include the 0-terminator. __grow() adds 1 to minCapcity.
	if(minCapacity>=getCapacity()) ((daeElement*)getObject())->__grow(*this,minCapacity); 
}

/**LEGACY, CIRCULAR DEPENDENCY
 * Goes beyond @c get(), returning either an xs:list dataset, 
 * -or, an individual unit of data. (This implements SIDREF.)
 * @see ColladaDOM.inl header's definition.
 */
inline daeOK daeRef::get(daeRefRequest &req)const
{	
	const_daeDOMRef dom(req.object!=nullptr?req.object->getDOM():getDOM());
	return nullptr==dom?DAE_ERR_INVALID_CALL:dom->getRefResolvers().resolve(*this,req);
}

/////////////////////////////////////////////////////////////////
//This section is for ColladaDOM 3 style enum structs. Defining//
//these operators in the template, with @c friend, dramatically//
//bloats MSVC2015 precompiled headers. Perhaps it would be best//
//to move the non = assignment operators into the global scope?//
/////////////////////////////////////////////////////////////////

//Defining this may reduce the size of precompiled header files.
//It seems to help Visual Studio dramatically just by virtue of
//having them as global operators, versus defined in a template.
#ifndef COLLADA_DOM_GLOBAL_ASSIGNMENT_OPERATORS_NOT_INCLUDED
#define __ DAEP::Value<ID,T,CC,PtoM>
#define _(bop) \
template<int ID, class T, class CC, typename CC::_ PtoM, class S>\
inline __ &operator bop(__ &lv, const S &rv)\
{\
	DAEP::Notice<__,typename __::underlying_type> lv2(&lv,lv); lv2 bop rv; return lv; \
}
/** Implements @c DAEP::Concern based change-notice. */
_(-=)_(+=)_(/=)_(%=)_(*=)_(<<=)_(>>=)_(&=)_(|=)_(^=)
#undef _
#define _(uop) \
template<int ID, class T, class CC, typename CC::_ PtoM>\
inline __ &operator uop(__ &lv)\
{\
	DAEP::Notice<__,typename __::underlying_type> lv2(&lv,lv); uop lv2; return lv; \
}\
template<int ID, class T, class CC, typename CC::_ PtoM>\
inline typename __::underlying_type operator uop(__ &lv,int)\
{\
	DAEP::Notice<__,typename __::underlying_type> lv2(&lv,lv); return lv2 uop;\
}
/**WARNING
 * Implements @c DAEP::Concern based change-notice. 
 * @warning Postfix ++ and -- return a copy of the
 * original value, and apply the prefix equivalent.
 */
_(++)_(--)
#undef _	
#undef __
//Reminder: DAEP Notice may be rolled into DAEP Value if per-item
//notices are dropped, in favor of applying DAEP NoConcern to all.
#define _(bop) \
template<class S, class T, class U>\
inline const T &operator bop(DAEP::Notice<S,T> /*&*/lv, const U &rv)\
{\
	struct _{ static void f(T &lv, const U &rv){ lv bop rv; } };\
	DAEP::Notice<S,T>::template _operation<U> op(_::f,lv,rv);\
	lv.template _do<typename DAEP::Notice<S,T>::note>(op,nullptr); return lv;\
}
/** Implements @c DAEP::Concern based change-notice. */
_(-=)_(+=)_(/=)_(%=)_(*=)_(<<=)_(>>=)_(&=)_(|=)_(^=)
#undef _
#define _(uop) \
template<class S, class T>\
inline DAEP::Notice<S,T> &operator uop(DAEP::Notice<S,T> /*&*/lv)\
{\
	struct _{ static void f(T &lv, const int &rv){ lv uop; } };\
	DAEP::Notice<S,T>::template _operation<int> op(_::f,lv,int());\
	lv.template _do<typename DAEP::Notice<S,T>::note>(op,nullptr); return lv;\
}\
template<class S, class T>\
inline T operator uop(DAEP::Notice<S,T> /*&*/lv,int){ T o = lv; uop lv; return o; }
/**WARNING
 * Implements @c DAEP::Concern based change-notice. 
 * @warning Postfix ++ and -- return a copy of the
 * original value, and apply the prefix equivalent.
 */
_(++)_(--)
#undef _
#endif //COLLADA_DOM_GLOBAL_ASSIGNMENT_OPERATORS_NOT_INCLUDED

template<int ID, class T, class CC, typename CC::_ PtoM, class S>
/** Nominal == support for ColladaDOM 3 style enum structs. */
bool operator==(const DAEP::Value<ID,T,CC,PtoM> &a, const S &b)
{
	return *a.operator->()==b;
}
template<int ID, class T, class CC, typename CC::_ PtoM, class S>
/** Nominal != support for ColladaDOM 3 style enum structs. */
bool operator!=(const DAEP::Value<ID,T,CC,PtoM> &a, const S &b)
{
	return *a.operator->()!=b;
}
template<int ID, class T, class CC, typename CC::_ PtoM, class S>
/** Nominal == support for ColladaDOM 3 style enum structs. */
bool operator==(const S &a, const DAEP::Value<ID,T,CC,PtoM> &b)
{
	return a==*b.operator->();
}
template<int ID, class T, class CC, typename CC::_ PtoM, class S>
/** Nominal != support for ColladaDOM 3 style enum structs. */
bool operator!=(const S &a, const DAEP::Value<ID,T,CC,PtoM> &b)
{
	return a!=*b.operator->();
}
template<int ID,class T,class CC,typename CC::_ PtoM, int ID2,class T2,class CC2,typename CC::_ PtoM2>
/** C++ is kind of messed up in that it requires 6 overloads to not be ambiguous. */
bool operator==(const DAEP::Value<ID,T,CC,PtoM> &a, const DAEP::Value<ID2,T2,CC2,PtoM2> &b)
{
	return *a.operator->()==*b.operator->();
}
template<int ID,class T,class CC,typename CC::_ PtoM, int ID2,class T2,class CC2,typename CC::_ PtoM2>
/** C++ is kind of messed up in that it requires 6 overloads to not be ambiguous. */
bool operator!=(const DAEP::Value<ID,T,CC,PtoM> &a, const DAEP::Value<ID2,T2,CC2,PtoM2> &b)
{
	return *a.operator->()!=*b.operator->();
}
//SCHEDULED FOR REMOVAL
//Reminder: DAEP Notice may be rolled into DAEP Value if per-item
//notices are dropped, in favor of applying DAEP NoConcern to all.
template<class S, class T, class U> 
/** Nominal == support for ColladaDOM 3 style enum structs. */
inline bool operator==(DAEP::Notice<S,T> /*&*/a, const U &b)
{
	return *a.operator->()==b;
}
template<class S, class T, class U>
/** Nominal != support for ColladaDOM 3 style enum structs. */
inline bool operator!=(DAEP::Notice<S,T> /*&*/a, const U &b)
{
	return *a.operator->()!=b;
}
template<class S, class T, class U>
/** Nominal == support for ColladaDOM 3 style enum structs. */
inline bool operator==(const U &a, DAEP::Notice<S,T> /*&*/b)
{
	return a==*b.operator->();
}
template<class S, class T, class U>
/** Nominal != support for ColladaDOM 3 style enum structs. */
inline bool operator!=(const U &a, DAEP::Notice<S,T> /*&*/b)
{
	return a!=*b.operator->();
}
template<class S, class T, class U, class V>
/** C++ is kind of messed up in that it requires 6 overloads to not be ambiguous. */
inline bool operator==(DAEP::Notice<S,T> &a, DAEP::Notice<U,V> /*&*/b)
{
	return a.operator->()==*b.operator->();
}
template<class S, class T, class U, class V>
/** C++ is kind of messed up in that it requires 6 overloads to not be ambiguous. */
inline bool operator!=(DAEP::Notice<S,T> &a, DAEP::Notice<U,V> /*&*/b)
{
	return a.operator->()==*b.operator->();
}

template<class S, class T>
/** @c daeDocument uses this to convert strings into @c daeStringRef if need be. */
inline const daeStringRef daeBoundaryStringRef(const S &c, const T &str)
{
	return daeStringRef(*c,str);
}
template<class S>
/** @c daeDocument uses this to convert strings into @c daeStringRef if need be. */
inline const daeStringRef &daeBoundaryStringRef(const S&, const daeStringRef &str)
{
	return str;
}
template<class S, int ID, class T, class CC, typename CC::_ PtoM>
/** @c daeDocument uses this to convert strings into @c daeStringRef if need be. */
inline const daeStringRef &daeBoundaryStringRef(const S&, const DAEP::Value<ID,T,CC,PtoM> &str)
{
	return str;
}

//---.
}//<-'

#endif //__COLLADA_DOM_INL__
/*C1071*/