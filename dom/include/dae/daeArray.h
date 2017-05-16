/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_ARRAY_H__
#define __COLLADA_DOM__DAE_ARRAY_H__

#include "daeTypes.h"

COLLADA_(namespace)
{//-.
//<-'

/**RTTI-SUPPORT
 * Pre-2.5 this class was "daeChar*" and possibly "daeMemoryRef" before
 * that. It evolved through phases. At first the idea was to mark it as
 * a @c void* like type, so it was called "daeOpaque." It needed to act
 * like a @c char* so it could be used to compute offsets without casts.
 * But it wasn't a "char*" so the idea was to "typedef char* daeOpaque;"
 * That fused the type and "*" together, so it couldn't have const-like
 * semantics any longer. So it became a very simple wrapper class. That
 * lasted for a long time, until it became evident that a C++ reference
 * cast does not behave strictly like a C pointer cast! What happens if
 * you do (T&)x is if x::x(T) exists, it tries to construct an x from T!
 * IOW, it invokes a constructor. That's a minefield, but if there is a
 * template based constructor, then all Ts invoke a constructor. That'd
 * especially became a problem because all of the casting code had been
 * rewritten to use (X&)* instead of *(X*) because it seems more direct.
 * (And because it keeps the symbols clustered together.) 
 * At that point it was clear that a simple wrapper would not do. To do
 * this kind of "opaque" magic with C++ is actually a minefield. And it
 * is unavoidable with this kind of library. So the new thinking became
 * how to make this as safe and simple as can be? The (T&) problem made
 * clear that it needed a conversion to reference operator, that used C
 * pointer casting internally. That is actually pretty cool, and a good
 * way to write deceptively easy to follow code, because the casting is
 * automatic. But it requires a type to cast to. For template arguments
 * there is no type. In that case @c operator& converts to a pointer to
 * do manual C-style pointer casting. @c operator[] is for computing an
 * offset. It avoids full-blown arithmetic operators, that would be out
 * of hand and hard to follow.
 * In general, it is like a C++ reference, that's seated with a pointer.
 * Note, the auto-casting can cast to a pointer. The [] behavior is not
 * based on the pointer's built-in [] operator. It's just its own thing.
 * As is @c operator==. It's comparing the pointers, but doesn't take a
 * pointer. It takes a reference. This way it can compare pointer types.
 *
 * Nothing should be added; only disabled. Fewer interactions is better.
 * Getting the address of the @c daeOpaque is probably impossible. That
 * is good. It's like references, and eliminates confusion and mistakes.
 */
class daeOpaque 
{
	mutable char *_ptr; 
	//These aren't to be used, so keeping it private is easier.
	void operator*()const{}
	template<class T> operator T()const{ return *(T*)nullptr; }		

COLLADA_(public) 

	daeOpaque(){ /*NOP*/ }
	daeOpaque(const void *vp):_ptr((char*)vp){}
	daeOpaque &operator=(void *vp)
	{ _ptr = (char*)vp; return *this; }
	const daeOpaque &operator=(const void *vp)const
	{ _ptr = (char*)vp; return *this; }		
	template<class T> operator T&(){ return *(T*)_ptr; }
	template<class T> operator const T&()const{ return *(const T*)_ptr; }		
	char *operator&(){ return _ptr; }		
	const char *operator&()const{ return _ptr; }
	daeOpaque operator[](daeOffset i){ return daeOpaque(_ptr+i); }
	const daeOpaque operator[](daeOffset i)const{ return daeOpaque(_ptr+i); }						
	bool operator==(daeOpaque b)const{ return _ptr==b._ptr; }
	template<class T> friend //!
	//Does T include const? Anyway, MSVC2015 can't compare two integral 
	//constants without trying to apply these operators/luckily failing.
	bool operator==(daeOpaque a, /*const*/ T &b){ return a._ptr==(char*)&b; }
	template<class T> friend //!
	//Does T include const? Anyway, MSVC2015 can't compare two integral 
	//constants without trying to apply these operators/luckily failing.
	bool operator==(/*const*/ T &a, daeOpaque b){ return b._ptr==(char*)&a; }		
};

/**SKETCHY
 * This attaches some information to a @c daeAllocThunk.
 * It's not generally helpful, but it's hard to dismiss.
 * @c daeRefRequest has it. It's a field of @c daeValue.
 *
 * An idea is to replace the @c daeAllocThunk vptr with
 * @c daePrototype for a portable ABI & 64bit alignment.
 */
struct daePrototype 
{
COLLADA_(public)
	/**
	 * This is typically the simpleType's name.
	 */
	daeClientString alias;
	/**
	 * This provides RTTI and opaque operators.
	 */
	daeTypewriter *writer;
	/**
	 * This is typically the generated class's
	 * prototype's member field. It holds what
	 * should be a suitable default value.
	 */
	daeOpaque value; 

	/**SCHEDULED FOR REMOVAL?
	 * @remark Since the generator uses @c size_on_stack
	 * for fixed-length <xs:list> based arrays these are
	 * playing only a neglible to non-existent part.
	 */
	unsigned int maxLength, minLength;	

COLLADA_(public) //OPERATORS (legacy compatibility layer)

	inline operator daeTypewriter*()const{ return writer; }
	inline operator daeTypewriter&()const{ return *writer; }
	inline daeTypewriter *operator->()const{ return writer; }		
};

/**
 * @daeAllocThunk, alone, is a 0-sized-placeholder for arrays.
 * However, it is also the non-0-sized base-class of @c daeAlloc.
 *
 * @note It does not have getX, or setX members, as conceptually a
 * thunk is always 0, and non-operable, and so that is meaningless.
 * This is despite being a base-class for non-thunk allocation-units.
 *
 * @todo At some much later date, decide if it's worth putting a
 * fully custom-made function pointer table inside of @c _prototype.
 * The virtual-methods are not high frequency, and they cause this
 * thunk to not be strictly 16-byte aligned, which would be a nice
 * feature. The custom solution might offer other benefits if given
 * lots of thought.
 */
class daeAllocThunk
{	
COLLADA_(public)
	/**
	 * Non-Virtual Non-Destructor 
	 */
	~daeAllocThunk(){}

COLLADA_(protected) //VIRTUAL METHODS
	/**
	 * @note This is implemented by @c daeAlloc<>. This
	 * version is intended to catch programmer-errors.
	 *
	 * This is  defined in @c daeAllocThunk merely, so it will
	 * have an overwritable "vptr." Hence its nondescript name:
	 */	
	virtual daeOpaque __Thunk__v1__method(size_t N)const
	{
		//This indicates a programmer neglected to locateThunk(*this).
		assert(0); return nullptr; 
	}
	/**INTERNAL Tests presence of a @c daePlatonic op. */
	inline bool _feature_test(int op)const
	{
		return __Thunk__v1__method2(nullptr,op)!=DAE_ERR_NOT_IMPLEMENTED;
	}

	friend class daeFeature;
	template<class,class> friend class daeArrayAware;
	/**
	 * This supports @c daeFeature. It performs @a op upon @a on.
	 * @param op is a @c daePlatonic @c enum.
	 * @param on is a @c daeArray<T> or a "T." It's not an object.
	 */
	virtual daeError __Thunk__v1__method2(void *on, int op,...)const
	{
		assert(0); return DAE_ERR_NOT_IMPLEMENTED;
	}

COLLADA_(public) //DATA-MEMBERS
	/**
	 * Internal counter for arrays to use to count up to @c _capacity.
	 *
	 * IMPORTANT!
	 * @remarks @c _counter is not necessarily less than @c _capacity.
	 * This is due to the order-of-events inside of reallocation code.
	 *
	 * @see @c daeAlloc::setInternalCounter().
	 */
	size_t _counter; 

	/**CONST DATA-MEMBER
	 * Fixed capacity; by definition an allocation unit has a fixed 
	 * capacity. A thunk has 0 capacity.
	 *
	 * @remarks Although called @c daeAllocThunk. It's also a base class.
	 * (Because it's a base class, it does not have advertised methods.)
	 */
	const size_t _capacity;	

	/**
	 * Positive byte (machine character) offset to the owning @c daeObject.
	 * 
	 * If between 0 and 3 the offset is understood to be 0, and so unowned.
	 *
	 * Unowned allocation units use the global default allocation pathways.
	 * However these are local to the calling translation unit, and are in
	 * addition to this, virtual method calls, which can be overriden. When
	 * the unit is "owned" by an object, it SHALL be stored in the database
	 * alongside that of the objects that comprise that DOM-cum-daeDatabase.
	 */
	daeOffset _offset;
	
	/**SKETCHY
	 * This may be a @c daePrototype if it's in the object's meta-data.
	 *
	 * This was to be a prototype, but it's not really useful as such, 
	 * -as why should it be that only arrays have RTTI inside of them?
	 * And beside, it was not very useful to have for implementing the
	 * @c daeArray class.
	 *
	 * The only part that may be useful in theory is the array bounds.
	 * In theory these can optimize allocation, if checking them isn't
	 * an overall drain. 
	 *
	 * TODO (THIS IS COPIED FROM THE daeAllocThunk DOXYGENTATION.)
	 * @todo At some much later date, decide if it's worth putting a
	 * fully custom-made function pointer table inside of @c _prototype.
	 * The virtual-methods are not high frequency, and they cause this
	 * thunk to not be strictly 16-byte aligned, which would be a nice
	 * feature. The custom solution might offer other benefits if given
	 * lots of thought.
	 */
	const daePrototype *_prototype;
	/**OPERATOR
	 * This is to complement @c daeFeature::getAllocThunk().
	 */
	inline const daePrototype *operator->()const
	{
		assert(_prototype!=nullptr);
		return static_cast<const daePrototype*>(_prototype); 
	}

	/**
	 * Default Constructor
	 *
	 * The memory layout for daeAlloc and thunks should not change.
	 * Any future information will be facilitated by @c _prototype.
	 */
	daeAllocThunk(size_t c=0):_counter(),_capacity(c),_offset(),_prototype(){}

COLLADA_(public) //Non-advertised inline methods (There was more.)

  /////////////////////////////////////////////////////////////
  //NOW THAT THE GENERATOR IS USING EMBEDDED ARRAYS FOR FIXED//
  //LENGTH LIST ATTRIBUTES, THIS IS NO LONGER OF ANY REAL USE//
  /////////////////////////////////////////////////////////////

	/**SKETCHY
	 * Just optimizing the allocation capacity for now. 
	 * What this does is take something like a matrix, and
	 * See that it isn't reallocated more than once/necessary.
	 *
	 * NOW THAT THE GENERATOR IS USING EMBEDDED ARRAYS FOR FIXED
	 * LENGTH LIST ATTRIBUTES, THIS IS NO LONGER OF ANY REAL USE.
	 */
	inline size_t _getminmax(size_t inlength, size_t floor)
	{
		if(_prototype!=nullptr)
		{
			size_t len = _prototype->minLength;			
			if(len>inlength) return len; 
			len = _prototype->maxLength; assert(floor<=len||len==0);
			if(inlength>len&&len>=floor) return len;
		}
		return inlength;
	}
};

/**ABSTRACT TEMPLATE-SPECIALIZATION
 * This class has the semantics of @c void*, only it's self-aware.
 *
 * @tparam N must be 4*4, or the same as @c daeAlloc<>.
 *
 * @remarks Post-2.5 @c daeAlloc<> replaces the pre-2.5 "daeArray."
 */
template<int N> class daeAlloc<void,N> : protected daeAllocThunk
{			
	//moveThunk
	friend class daeAllocThunk;
	//Exposing protected daeAllocThunk just
	//so debuggers don't jump into accessors.
	template<class,int> friend class daeArray;

COLLADA_(public) //Thunk operations
	/**OPERATOR
	 * Converts @c daeAlloc<T>::localThunk() into a
	 * pointer to ease @c daeFeature._localthunk set up.
	 */
	inline operator daeAllocThunk*(){ return this; }

COLLADA_(protected) //void-semantics
	/**
	 * Default Constructor
	 */	
	daeAlloc(size_t c=0):daeAllocThunk(c)
	{
		daeCTC<N==4*4>(); //Must equal daeAlloc<>.
	} 
	/**
	 * Non-Destructor (AUs are not C++ objects.)
	 */	
	~daeAlloc(){ assert(_capacity<=1); }

COLLADA_(private)	
	/**
	 * Disabled Copy Constructor & Assignment Operator
	 */	
	daeAlloc(const daeAlloc&); void operator=(const daeAlloc&);  	

COLLADA_(protected) //VIRTUAL METHOD TABLE
	/**
	 * This "method" is called via @c deleteThis() and @c newThis().
	 * It's not implemented in @c daeAllocThunk so that uninitialized
	 * thunks will trigger an assertion.
	 * Thunks are initialized by @c locateThunk(). (Generators do this.)
	 */	
	virtual daeOpaque __Thunk__v1__method(size_t N)const
	{
		if(N!=0) return operator new(N); delete (void*)this; return nullptr;
	}	

	/**LEGACY
	 * Gets a pointer to the raw memory of a particular element.
	 * @return Returns a pointer to the memory for the raw data.
	 *
	 * @note This could be fudged without a virtual method, but-
	 * for the virtual-table causing the thunk members to not be
	 * 16-byte aligned; so the alignment is anyone's guess.
	 *
	 * THE ONLY SOLUTION would be to store a custom-made function
	 * pointer table inside of @c daeAllocThunk::_prototype. This
	 * seems extreme--although at some point it can be considered.
	 */
	virtual daeOpaque __Alloc__v1__getRaw(size_t index=0) = 0;
	
COLLADA_(public) //non-database AU memory management APIs.
	/**
	 * @c daeObject::re/deAlloc calls to free freestanding memory.
	 * @see daeAlloc<T>::newThis().
	 */	
	inline void deleteThis()const{ __Thunk__v1__method(0); }	
	/**
	 * @c daeObject::reAlloc calls to allocate freestanding memory.
	 * @see daeAlloc<>::deleteThis().
	 */
	inline daeAlloc &newThis(size_t N)const
	{
		return __Thunk__v1__method(&getRaw(N)-&daeOpaque(this)); 
	}

COLLADA_(public) //ACCESSORS ONLY -- No Mutators for <void> 	
	/**
	 * Gets the internal counter.
	 * @remarks Use with care, as _counter may be greater than _capacity.
	 */
	inline size_t getInternalCounter()const{ return _counter; }	
	/**
	 * Gets the number of items stored in this @c daeAlloc.
	 * @return Returns the number of items stored in this @c daeAlloc.
	 */
	inline size_t getCount()const{ return _counter<=_capacity?_counter:_capacity; }	

	/**
	 * Gets the current capacity of the array, the biggest it can get without incurring a realloc.
	 * @return Returns the capacity of the array.
	 */
	inline size_t getCapacity()const{ return _capacity; }	
	
	/**
	 * Gets the positive offset from the @c daeObject 
	 * to this allocation unit's embedded pointer.
	 * @return Returns 0 if no @c daeObject owns this unit.
	 */
	inline daeOffset getOffset()const{ return _offset; }
	
	/**LEGACY
	 * Gets a pointer to the raw memory of a particular element.
	 * @return Returns a pointer to the memory for the raw data.
	 *
	 * @warning DON'T USE THIS IN A LOOP. HAVE index BE 0 UNLESS
	 * YOU'RE GETTING JUST ONE, OR BEGINNING A LOOP AT AN OFFSET.
	 */
	inline daeOpaque getRaw(size_t index=0)
	{
		return __Alloc__v1__getRaw(index); 
	}
	/**LEGACY, CONST-FORM
	 * Gets a pointer to the raw memory of a particular element.
	 * @return Returns a pointer to the memory for the raw data.
	 *
	 * @warning DON'T USE THIS IN A LOOP. HAVE index BE 0 UNLESS
	 * YOU'RE GETTING JUST ONE, OR BEGINNING A LOOP AT AN OFFSET.
	 */
	inline const daeOpaque getRaw(size_t index=0)const
	{
		return const_cast<daeAlloc*>(this)->getRaw(index);
	} 

	/**LEGACY
	 * Gets the size of an element in this array.
	 * @return Returns the size of an element in this array.
	 */
	inline size_t getElementSize()const
	{
		return __Alloc__v1__getRaw(1)-__Alloc__v1__getRaw(0); 
	}

COLLADA_(public) //STATIC METHODS	

	template<class T> //VS2013 wants the pointer fully qualified.
	/**	 
	 * @note CANNOT BE A METHOD BECAUSE @c this IS UNADDRESSABLE.
	 * @note const versions are not provided. Adding const cannot seem to work.
	 * @return Returns true if it's appropriate to delete this allocation-unit.
	 * Reasons to not delete are: A) it's a thunk, and B) it's object-embedded.	 
	 */
	inline static bool isFreeAU(daeAlloc<T>* &au)
	{
		//1 should be harmless. It's for the contents-array hidden partition.
		//Embedded AUs can be 1. Leave it even if a partition isn't required.
		return au->getCapacity()>/*0*/1&&!isEmbeddedAU(au);
	}	

	template<class T> //Here template is required because of C++.
	/** 
	 * @see @c daeAlloc<void>isFreeAU() notes.
	 * @return Returns true if @au is part of a non-specialized @c daeArray,
	 * thats @c size_on_stack>0, and that is the initial/non-dynamic AU.
	 */
	inline static bool isEmbeddedAU(daeAlloc<T>* &au)
	{
		//daeOffsetOf should not be a macro!
		typedef daeArray<char,!0> commaless;
		daeOffset delta = daeOffset(au)-daeOffset(&au);
		return delta==daeOffsetOf(commaless,_stack_au);
	}

COLLADA_(public) //UTILITIES
	/**
	 * Copy virtual-table and reallocation information over to @a reAlloc.
	 *
	 * @note Logically, not making this a virtual-method seems provably sound.
	 *
	 * @remarks @c locateThunk() should be preferred to @c moveThunk(). This does
	 * move-semantics, post-reallocation, although there is nothing to do but copy.
	 */
	inline void moveThunk(daeAlloc<> *reAlloc, size_t reCap)const
	{
		//__Alloc__v1__cloneThunk(clone); //seems like this has to always work.
		memcpy(reAlloc,this,sizeof(daeAllocThunk)); 
		const_cast<size_t&>(reAlloc->_capacity) = reCap;
	}
	/** Implements @c moveThunk() while retaining the thunks existing capacity. */
	inline void moveThunk(daeAlloc<> *reAlloc)const
	{
		moveThunk(reAlloc,reAlloc->_capacity);
	}
};   

template<class T, int size_of_array>
/**
 * Allocation Unit, or, AU
 * @tparam size_of_array must be 0 when constructing an object of this type.
 * It must be 1 or more to access the array. Construction is done to set the
 * virtual-table pointer up; and, to initialize daeAllocThunk's data members.
 */
class daeAlloc : public daeAlloc<> 
{	
COLLADA_(public)
	/**
	 * Default Constructor
	 */	
	daeAlloc(size_t c=0):daeAlloc<>(c){ assert(size_of_array==0); } 	

	COLLADA_SUPPRESS_C(4624)
	/**
	 * Non-Destructor (AUs are not C++ objects.)
	 */	
	~daeAlloc(){ daeCTC<size_of_array==0>(); }

COLLADA_(public) //DATA MEMBER
	/**
	 * C++ forbids 0-sized arrays in structures (and on the stack.)
	 */	
	template<int size> struct _empty_arrays_are_forbidden
	{
		typedef T type[size];
	};
	template<> struct _empty_arrays_are_forbidden<0>
	{
		typedef enum{}type;
	};
	typename _empty_arrays_are_forbidden<size_of_array>::type _varray;		
	
	/**LOCAL-SINGLETON FACTORY
	 * @return Returns a per translation-unit shared-thunk.
	 * @remark This could be a virtual method, to save space. It doesn't
	 * matter in terms of correctness. Static method might help inlining.
	 */
	static inline const daeAlloc &localThunk()
	{
		//T ensures size_of_array is invariant for _localThunk2(), which
		//has a local-static variable (the local thunk.)
		return (daeAlloc&)(daeAlloc<T>::_localThunk2()); 
	}
	/**SKETCHY
	 * @todo It would be good to setup a @c daePrototype w/ @c daeTypewriter. 
	 * There isn't a path for every type to a typewriter. @c enum types will
	 * be especially challenging if not impossible. The @c typeid name could 
	 * be one way to support types that are pre-registered, either by a user
	 * or by an @c XS::Schema.
	 */
	static inline const daeAlloc &_localThunk2()
	{
		//0 prevents construction of _varray when localThunk() is never used.
		static daeAlloc<T,0> t;
		COLLADA_ASSUME(t._capacity==0&&t._counter==0); return (daeAlloc&)t; 
	}	

COLLADA_(public) //OPERATORS
	/**
	 * This is for @c daeFeature::setAllocThunk().
	 */
	operator daeAllocThunk*()const
	{
		daeCTC<size_of_array==0>(); return (daeAllocThunk*)this;
	}

COLLADA_(public) //MUTATORS	
	/**
	 * Sets the number of items stored in this @c daeAlloc.
	 */
	inline void setInternalCounter(size_t c){ _counter = c; assert(c<=_capacity); }	
	
COLLADA_(private) //daeAllocThunk methods	

	template<class> friend class daePlatonic;
	template<class,class> friend class daeArrayAware;
	/**INTERNAL
	 * This supports @c daeFeature. It executes @a op upon @a on.
	 * If optimization is enabled this should compile down to no
	 * more than @c return(DAE_ERR_NOT_IMPLEMENTED).
	 * The metadata may test/cache the error code to avoid calls.
	 */
	virtual daeError __Thunk__v1__method2(void *on, int op,...)const
	{
		daePlatonic<T> p(on,this); switch(op)
		{		
		case p.atomize: return p.maybe<p.atomize>(); //Does teardown.
		case p.locate: //Does copy construction local to the array's module.
		{	va_list va; va_start(va,op); daeError e = p.maybe<p.locate>(va);
			va_end(va);	return e;
		}}return DAE_ERR_NOT_IMPLEMENTED;
	}

COLLADA_(protected) //daeAlloc<> methods									
	/**OVERRIDE
	 * Gets a pointer to the raw memory of a particular element.
	 * @return Returns a pointer to the memory for the raw data.
	 */
	virtual daeOpaque __Alloc__v1__getRaw(size_t index=0)
	{
		//newThis() calls getRaw() to size the new array.
		//assert(index<_counter);
		return ((daeAlloc<T>*)this)->_varray+index; 
	}	
			 
COLLADA_(public) //UTILITIES
	/**
	 * Initialize this thunk. 
	 * Sometimes this is a function pointer. It's no big deal
	 * that it does this courtesy return whenever cold-called.
	 */
	inline static daeAlloc &locateThunk(daeAllocThunk &thunk)
	{ 
		assert(thunk._capacity==0);
		daeOffset o = thunk._offset;
		const daePrototype *p = thunk._prototype;
		new((void*)&thunk) daeAlloc<T,0>; 
		thunk._offset = o; thunk._prototype = p; return (daeAlloc&)thunk;
	}

	/** @see @c _empty_arrays_are_forbidden. */
	inline T *data(){ return ((daeAlloc<T,1>*)this)->_varray; }	
	/** @see @c daeArrayAware Doxygentation. */
	inline static daeAlloc<T> &dataptr_cast(T *dataptr)
	{ 
		return (daeAlloc<T>&)daeOpaque(dataptr)[-daeOffsetOf(daeAlloc<T>,_varray)];
	}	
};

/**
 * COLLADA class that implements array move-semantics, and object-based initialization.
 *
 * For portability sake, the move procedures receive the AU's data-pointer. It's always
 * the 0th unit of data. @c daeAlloc::dataptr_cast() can obtain the AU given the pointer.
 */
template<class T, class U=T> class daeArrayAware
{
	typedef char Yes; typedef long No;		
	/** Indicates @c S::S() requires a pointer to its @c DAEP::Object. */
	template<typename S> static Yes Aware(typename S::__COLLADA__Object*);
	template<typename S> static No Aware(...);
	template<class T, T> class PtoM{};
	//NOTE: pointer-to-members won't find an inherited implementation. That's actually
	//a help here, since it automatically prevents "slicing;" requiring reimplementation.
	/** Indicates their is a virtual-method-table with a cross-module "locate" API defined. */
	template<typename S> static Yes Local(PtoM<void(S::*)(const S&),&S::__COLLADA__locate>*);
	template<typename S> //__COLLADA__locate with __COLLADA__Object. See above explanation. 
	static Yes Local(PtoM<void(S::*)(typename S::__COLLADA__Object*,const S&),&S::__COLLADA__locate>*);
	template<typename S> static No Local(...);	
	template<typename T> struct YesT{ Yes yes; };	
	/** Indicates @a S is a @c struct, @c class, or @c union. Think @c std::is_object. */
	template<typename S> static YesT<S S::*> Class(S*,...);
	template<typename S> static No Class(...);
	template<typename xs> static No Class(xs*,typename xs::__enum__*); //ColladaDOM 3
	/** Indicates @a S is a wrapper of a raw-copyable type. For ignoring ref-counting. */
	template<typename S> static Yes POD(typename S::__COLLADA__POD*);
	template<typename S> static No POD(...);	

public: /** Prefer @c is_plain over @c is_class for copying data. */		
		static const bool is_aware = sizeof(Yes)==sizeof(Aware<T>(nullptr));
		static const bool is_local = sizeof(Yes)==sizeof(Local<T>(nullptr));
		static const bool is_class = sizeof(Yes)==sizeof(Class<T>(nullptr,nullptr));
		static const bool is_plain = sizeof(Yes)==sizeof(POD<T>(nullptr))||!is_class;
private:

	template<int> static void tri_move(const DAEP::Object*,T*,T*,size_t);
	template<> static void tri_move<0>(const DAEP::Object*, T *lv, T *rv, size_t iN)
	{
		memcpy(lv,rv,iN*sizeof(T));
	}	
	template<> static void tri_move<1>(const DAEP::Object *obj, T *lv, T *rv, size_t iN)
	{
		//UGLY: IF __move USES __locate *IT* MUST LOOK SOMETHING LIKE tri_move<2>().
		void (*sigtest)(typename T::__COLLADA__Object*,T*,T*,size_t) = T::__COLLADA__move; 
		T::__COLLADA__move((typename T::__COLLADA__Object*)obj,lv,rv,iN);
	}	
	template<> static void tri_move<2>(const DAEP::Object*, T *lv, T *rv, size_t iN)
	{
		//NEW: &daeAlloc<T>::dataptr_cast(lv) is forwarding to tri_place<-2>().
		for(size_t i=0;i<iN;i++){ place(&daeAlloc<T>::dataptr_cast(lv),nullptr,lv+i,rv[i]); rv[i].~T(); }
	}	

	template<int> static void tri_place(const DAEP::Object*, T *placement_new_this, const U &cp, daeAlloc<T>*)
	{
		new(placement_new_this) T(cp);
	}
	template<> static void tri_place<1>(const DAEP::Object *obj, T *placement_new_this, const U &cp, daeAlloc<T> *AU)
	{
		void (T::*sigtest)(typename T::__COLLADA__Object*,const U&) = &T::__COLLADA__place; 
		placement_new_this->__COLLADA__place((typename T::__COLLADA__Object*)obj,cp);
	}
	template<> static void tri_place<-1>(const DAEP::Object *obj, T *placement_new_this, const U &cp, daeAlloc<T> *AU)
	{
		const T &cpT = cp; //__COLLADA__locate is instantiated by daePlatonic::_locate().
		AU->__Thunk__v1__method2(placement_new_this,daePlatonic<>::locate,obj,&cpT);
	}
	template<> static void tri_place<-2>(const DAEP::Object*, T *placement_new_this, const U &cp, daeAlloc<T> *AU)
	{
		const T &cpT = cp; //__COLLADA__locate is instantiated by daePlatonic::_locate().
		AU->__Thunk__v1__method2(placement_new_this,daePlatonic<>::locate,&cpT);
	}

public: static const bool value = is_aware;

	static const int tri_value = is_plain?0:is_aware?1:2;

	/** C++03 object-aware allocation-unit swap/hook facility. */
	static void move(const daeObject *obj, T *lv, T *rv, size_t iN)
	{
		tri_move<tri_value>(obj,lv,rv,iN);
	}

	/** C++03 object-aware placement-new based copy-construction. */
	static void place(daeArray<T> *a, T *placement_new_this, const U &cp)
	{
		place(a->getAU(),a->getObject(),placement_new_this,cp);
	}
	static void place(daeAlloc<T> *AU, const DAEP::Object *obj, T *placement_new_this, const U &cp)
	{
		tri_place<is_local?-tri_value:tri_value>(obj,placement_new_this,cp,AU);
	}
};

/**PARTIAL-TEMPLATE-SPECIALIZATION
 *
 * COLLADA C++ class that implements storage for resizable array containers.
 *
 * @remarks The main reason this class now uses @c template<class S> inputs
 * is @c daeStringRef::daeStringRef(daeString) had to go, and so there must
 * be a way to pass the @c daeString down, down, down, until it is assigned.
 */
template<class T> class daeArray<T>
{		
COLLADA_(public) //DAEP::Value
	/** 
	 * Don't confused this with @c __COLLADA__atomize.
	 * Here. "Atom" is stemming from @c daeAtomicType.
	 */
	typedef T __COLLADA__Atom, Atom, value_type; 

	typedef T *iterator; typedef const T *const_iterator;

COLLADA_(protected) //DATA MEMBER

	template<class,class> friend class daeArrayAware;
	/**
	 * Allocation Unit
	 */
	union{ daeAlloc<T> *_au; const daeAlloc<T> *_thunk_au; };
	  
COLLADA_(public)

	template<class Type> //DAEP::Value<...>
	/**WARNING
	 * Prototype Constructor
	 *
	 * @todo If there are per-value compile-time-contants, the call
	 * to @c empty() etc. can be eliminated. For instance, if fixed
	 * then the copy is may not required. If there's no default, it
	 * is definitely unrequired.
	 *
	 * @warning This is a little too dicey because @c this can be a
	 * prototype, inside @c daeMetaElement::~daeMetaElement(). It's
	 * a defect maybe. Because of this, @c getCount() is used. This
	 * seems like a trap that it'd be better if it weren't possible.
	 * //
	 * @note Using @c 0==getCapacity() would likely be an issue for
	 * the contents-arrays, since their thunks have 1 capacity, for
	 * copying their hidden-partition without added is-empty? logic.
	 */
	explicit daeArray(const DAEP::Proto<Type> &pt)
	{	
		if(pt.has_default||pt.is_fixed)
		{
			//There is no copy-on-write mechanism, so copy the defaults.
			daeAlloc<T> *swap = _au;
			_au = (daeAlloc<T>*)&pt.model()[pt.feature()].getAllocThunk();
			*this = (daeArray&)swap;
		}
	}
	/**LEGACY-SUPPORT
	 * Explicit Copy Constructor
	 * This constructor assumes the AU is @c daeAlloc::localThunk().
	 * 
	 * @remarks 2.5 marks this as @c explicit to try to limit cases
	 * where arrays are created via copy accidentally when they are
	 * being passed via const-reference.
	 */
	explicit daeArray(const daeArray<T> &cpy){ *new(this)daeArray = cpy; }
	/**LEGACY
	 * Constructor that takes one element and turns into an array.
	 * This constructor assumes the AU is @c daeAlloc::localThunk().
	 */
	explicit daeArray(const T &el){ (*new(this)daeArray).push_back(el); }	
	/**
	 * Default Constructor
	 */
	daeArray(const daeAlloc<T> &AU=daeAlloc<T>::localThunk()):_thunk_au(&AU){}
	/**
	 * Destructor
	 * @remark @c clear() is not used to avoid @c _clear2<daeContent>().
	 * @see @c XS::Choice::_solve() definition.
	 */
	~daeArray(){ _clear2<void>(); getObject()->deAlloc(_au); }

	/**
	 * This is exposed for the RTTI subsystem.
	 * This is a dangerous API. Use with care.
	 */
	daeAlloc<T>* &getAU(){ return _au; }
	/**CONST-FORM
	 * This is exposed for the RTTI subsystem. 
	 * This is a dangerous API. Use with care.
	 */
	const daeAlloc<T> *getAU()const{ return _au; }

	/**WARNING
	 * @warning Freestanding arrays return @c nullptr
	 * Get the allocation unit's containing object--or @c nullptr.
	 */
	inline daeObject *getObject()
	{
		daeOffset os = _au->getOffset();
		return 0==os?nullptr:reinterpret_cast<daeObject*>((char*)this-os);
	}
	/**WARNING, CONST-FORM
	 * @warning Freestanding arrays return @c nullptr
	 * Get the allocation unit's containing object--or @c nullptr.
	 */
	inline const daeObject *getObject()const
	{
		return const_cast<daeArray*>(this)->getObject();
	}

COLLADA_(public) //Standard Library compatibility layer
	/**
	 * Pre-2.5 @c clear freed the memory.
	 * Since 2.5 the capacity does not change.
	 * Maintainers note: to completely free the memory, a pointer to the 
	 * original thunk must be added to _prototype.
	 * Freestanding arrays can re-construct themself with placement-new.
	 * HOWEVER. This should not be done with clear.
	 */
	inline void clear(){ _clear2<T>(); }
	/**
	 * Implements @c clear(). 
	 */
	template<class> inline void _clear2()
	{
		size_t iN = _au->getCount(); //hack: use safe count (min(count,capacity))
		for(size_t i=0;i<iN;i++) _au->_varray[i].~T(); _au->setInternalCounter(0);		
	}
	/**KISS, CIRCULAR-DEPENDENCY
	 * Implements @c daeArray<daeContent>::clear(). 
	 * @see ColladaDOM_3.inl header's implementation.
	 */
	template<> inline void _clear2<daeContent>();

	/** Implements '\0' terminator for @c daeArray<daeStringCP>. */
	inline void clear_and_0_terminate()
	{
		daeCTC<daeArrayAware<T>::is_plain>(); //must be raw-copyable
		grow(1); _au->setInternalCounter(0); _au->_varray[0] = '\0';
	}	

	/**
	 * @return Returns @c true if @c 0==getCount().
	 */
	inline bool empty()const{ return 0==getCount(); }

	/**
	 * Gets the data pointer without triggering @c assert when empty.
	 */
	inline T *data(){ return _au->_varray; }			
	/**CONST-FORM
	 * Gets the data pointer without triggering @c assert when empty.
	 */
	inline const T *data()const{ return _au->_varray; }			

	/**
	 * Gets the last element in the array. @c getCount() cannot be 0.
	 */
	inline T &back(){ return get(getCount()-1); }			
	/**CONST-FORM
	 * Gets the last element in the array. @c getCount() cannot be 0.
	 */
	inline const T &back()const{ return get(getCount()-1); }			
	
	/**
	 * Gets the first element in the array. @c getCount() cannot be 0.
	 */
	inline T &front(){ return get(0); }
	/**CONST-FORM
	 * Gets the first element in the array. @c getCount() cannot be 0.
	 */
	inline const T &front()const{ return get(0); }			

	/**
	 * Forms a pointer-based "STL" style iterator for <algorithm>.
	 */
	inline T *begin(){ return _au->_varray; }			
	/**CONST-FORM
	 * Forms a pointer-based "STL" style iterator for <algorithm>.
	 */
	inline const T *begin()const{ return _au->_varray; }			
	/**
	 * Forms a const-pointer-based "STL" style iterator for <algorithm>.
	 */
	inline const T *cbegin()const{ return _au->_varray; }			

	/**
	 * Forms a pointer-based "STL" style delimiter for <algorithm>.
	 */
	inline T *end(){ return _au->_varray+_au->_counter; }
	/**CONST-FORM
	 * Forms a pointer-based "STL" style delimiter for <algorithm>.
	 */
	inline const T *end()const{ return _au->_varray+_au->_counter; }
	/**
	 * Forms a const-pointer-based "STL" style delimiter for <algorithm>.
	 */
	inline const T *cend()const{ return _au->_varray+_au->_counter; }			

	/**
	 * @see @c getCount() legacy API.
	 */
	inline size_t size()const
	{
		assert(_au->_counter<=_au->_capacity); return _au->getInternalCounter(); 
	}
	
	template<class S>
	/** @c std::string compatible version of assign. */
	inline daeArray &assign(const S &other)
	{
		clear(); return append(other.data(),other.size()); 
	}
	/** @c std::string compatible version of assign. */
	inline daeArray &assign(const T *c_array, size_t iN)
	{
		if(daeArrayAware<T>::is_plain) //raw-copyable?
		{
			grow(iN); memcpy(_au->_varray,c_array,iN*sizeof(T));
		}
		else
		{
			clear(); grow(iN);
			for(size_t i=0;i<iN;i++) 
			daeArrayAware<T>::place(this,_au->_varray+i,c_array[i]);		
		}
		_au->setInternalCounter(iN); return *this;
	}
	template<class S>
	/** Implements '\0' terminator for @c daeArray<daeStringCP>. */
	inline daeArray &assign_and_0_terminate(const S &s)
	{
		clear(); return append_and_0_terminate(s);
	}
	template<class S, class T>
	/** Implements '\0' terminator for @c daeArray<daeStringCP>. */
	inline daeArray &assign_and_0_terminate(const S &s, const T &t)
	{
		clear(); return append_and_0_terminate(s,t);
	}
	template<class S>
	/** @c std::string compatible version of append. */
	inline daeArray &append(const S *c_array, size_t iN, bool _plus_1=false)
	{
		grow(size()+iN+(_plus_1?1:0));
		for(size_t i=0;i<iN;i++) push_back(c_array[i]); return *this;
	}
	template<>
	/** @c std::string compatible version of append. */
	inline daeArray &append(const T *c_array, size_t iN, bool _plus_1/*=false*/)
	{
		size_t oldSize = size();
		size_t newSize = oldSize+iN;
		grow(newSize+(_plus_1?1:0));
		if(daeArrayAware<T>::is_plain) //raw-copyable?
		{
			memcpy(_au->_varray+oldSize,c_array,iN*sizeof(T));
			_au->setInternalCounter(newSize);
		}
		else for(size_t i=0;i<iN;i++) push_back(c_array[i]); return *this;
	}
	template<class S>
	/**
	 * These are to guard against calling @c append() on an other @c daeArray.
	 * @see @c appendArray() legacy API.
	 */
	inline daeArray &append(const S &other)
	{
		return append(other.data(),other.size()); 
	}
	template<class S>
	/**GENIUS!
	 * This is for symmetry with the above API, and it can chain @c daeRefView.
	 * @see the C-array version of @c append_and_0_terminate(). 
	 */
	inline daeArray &append_and_0_terminate(const S &other)
	{
		return append_and_0_terminate(other.data(),other.size()); 
	}
	/**GENIUS!
	 * Implements '\0' terminator for @c daeArray<daeStringCP>.
	 * (The 0-terminator is not included in @c size().)
	 */
	inline daeArray &append_and_0_terminate(const T *c_array, size_t iN)
	{
		daeCTC<daeArrayAware<T>::is_plain>(); //must be raw-copyable

		append(c_array,iN,+1); _au->_varray[size()] = 0; return *this;
	}	
	template<class S>
	/**
	 * Post 2.5 @c push_back() should be used instead of the old @c append() API
	 * for single argument uses.
	 */
	inline void push_back(const S &value){ set(size(),value); }
	/**
	 * Complements @c push_back(). (Removes last item.)
	 */
	inline void pop_back(){ setCountLess(size()-1); }
	
	template<class S>
	/**
	 * Standard Library like replacement for @c setCount(). 
	 */
	inline void resize(size_t size, const S &value){ setCount(size,value); }
	/**
	 * Standard Library like replacement for @c setCount(). 
	 */
	inline void resize(size_t size){ setCount(size); }

COLLADA_(public) //LEGACY ACCESSORS & MUTATORS
	/**LEGACY
	 * Resets the number of elements in the array. If the array increases in size, the new 
	 * elements will be initialized to the specified value.
	 * @param nElements The new size of the array.
	 * @param value The value new elements will be initialized to.
	 * @note Shrinking the array does NOT free up memory.
	 */
	inline void setCount(size_t nElements)
	{
		return setCount(nElements,T());
	}
	template<class S>
	/**LEGACY
	 * Resets the number of elements in the array. If the array increases in size, the new 
	 * elements will be initialized to the specified value.
	 * @param nElements The new size of the array.
	 * @param value The value new elements will be initialized to.
	 * @note Shrinking the array does NOT free up memory.
	 */
	inline void setCount(size_t nElements, const S &value)
	{
		grow(nElements);
		{
			size_t iN = getCount();
			//Destruct the elements that are being chopped off
			for(size_t i=nElements;i<iN;i++) _au->_varray[i].~T();
			//Use value to initialize the new elements
			for(size_t i=iN;i<nElements;i++) 
			daeArrayAware<T,S>::place(this,_au->_varray+i,value);
		}
		_au->setInternalCounter(nElements);
	} 
	/** 2.5 variation on @c setCount(), where: @c i<=getCount(). */
	inline void setCountLess(size_t i)
	{
		size_t iN = getCount(); _au->setInternalCounter(i); 
		assert(i<=iN);
		//Destruct the elements that are being chopped off.
		while(i<iN) _au->_varray[i++].~T();		
		//HACK: protect clear-only contents-arrays.
		if(false) setCount(i);
	} 	
	/** 2.5 variation on @c setCount(), where: @c iN>=getCount().  */
	inline void setCountMore(size_t iN)
	{
		return setCountMore(iN,T());
	}
	template<class S>
	/** 2.5 variation on @c setCount(), where: @c iN>=getCount().  */
	inline void setCountMore(size_t iN, const S &value)
	{
		assert(iN>=getCount());
		grow(iN);
		//Use value to initialize the new elements
		for(size_t i=getCount();i<iN;i++)
		daeArrayAware<T,S>::place(this,_au->_varray+i,value);
		_au->setInternalCounter(iN);
	} 
	/**
	 * Gets the number of items stored in this @c daeArray.
	 * @see @c size().
	 * @return Returns the number of items stored in this @c daeArray.	 
	 */
	inline size_t getCount()const
	{
		assert(_au->_counter<=_au->_capacity); return _au->getInternalCounter(); 
	}	

	/**
	 * Gets the current capacity of the array, the biggest it can get without incurring a realloc.
	 * @return Returns the capacity of the array.
	 */
	inline size_t getCapacity()const{ return _au->_capacity; }			

	template<class S>
	/**
	 * Sets a specific index in the @c daeArray, growing the array if necessary.
	 * @param index Index of the object to set.
	 * @param value Value to store at index in the array.
	 */
	inline void set(size_t index, const S &value)
	{
		if(index>=getCount()) setCountMore(index+1); get(index) = value;
	}
	/**
	 * Gets the object at a specific index in the @c daeArray.
	 * @param index Index of the object to get, asserts if the index is out of bounds.
	 * @return Returns the object at index.
	 */
	inline T &get(size_t index){ assert(index<getCount()); return _au->_varray[index]; }	
	/**CONST-FORM
	 * Gets the object at a specific index in the @c daeArray.
	 * @param index Index of the object to get, asserts if the index is out of bounds.
	 * @return Returns the object at index.
	 */
	inline const T &get(size_t index)const{ assert(index<getCount()); return _au->_varray[index]; }	
	
	/**
	 * Increases the capacity of the @c daeArray.
	 * @param minCapacity The minimum array capacity (the actual resulting capacity may be higher).
	 */
	inline void grow(size_t minCapacity)
	{
		_grow2<T>(minCapacity);
	}
	template<class>	
	/** Implements @c grow(). */
	inline void _grow2(size_t minCapacity)
	{
		if(minCapacity>getCapacity()) _grow3(minCapacity);
	}
	/**
	 * @note @c_getminmax() is potentially
	 * protecting extra hidden-partition data, and 
	 * is not strictly a churn reduction optimization. 
	 */
	COLLADA_NOINLINE void _grow3(size_t minCapacity)
	{
		enum
		{
		_small = 16*sizeof(void*)/sizeof(T),
		minCap = _small<2?2:_small
		};
		size_t newCap = getCapacity();
		if(newCap<=1) newCap = minCap/2; //1;
		//Changing this formula so new arrays come out
		//at the initially specified capacity, and not
		//more. std::max(newCap*2,minCapacity); is new.		
		//This is easier than inventing an API for the
		//static metadata arrays; EVEN THOUGH daeArray
		//IS OVERKILL IN THEIR CASE, IT'S EASY FOR NOW.
		//while(newCap<minCapacity) newCap*=2; 
		newCap = std::max(newCap*2,minCapacity);
		newCap = _au->_getminmax(newCap,minCapacity); //!
		getObject()->reAlloc(_au,newCap,_grower);
	}			
	template<>	
	/** KISS, CIRCULAR-DEPENDENCY
	 * Implements @c daeArray<daeContent>::clear(). 
	 * @c __COLLADA__move() was going to be used, but
	 * when @c daeCursor was made to be an iterator, it
	 * became necessary to adjust it.
	 * @see ColladaDOM_3.inl header's implementation.
	 */
	void _grow2<daeContent>(size_t minCapacity);
	//TODO? _sizer() "illustrates" growing AND shrinking.
	static void _sizer(const daeObject *obj, daeAlloc<T> &dst, daeAlloc<T> &src)
	{	
		size_t i = dst.getCount(), iN = src.getCount();
		daeArrayAware<T>::move(obj,dst._varray,src._varray,i<iN?i:iN);
		while(i<iN) src._varray[i++].~T();
	}	
	static void _grower(const daeObject *obj, daeAlloc<T> &dst, daeAlloc<T> &src)
	{	
		assert(dst.getCapacity()>src.getCapacity());
		daeArrayAware<T>::move(obj,dst._varray,src._varray,src.getCount());
	}
	static void _copier(const daeObject *obj, daeAlloc<T> &dst, daeAlloc<T> &src)
	{	
		assert(dst.getCount()==src.getCount());
		daeArrayAware<T>::move(obj,dst._varray,src._varray,src.getCount());
	}

COLLADA_(public) //daeArray<daeArray> (etc.) move/grow implementation	

	typedef const daeObject __COLLADA__Object;
	static void __COLLADA__move(const daeObject *obj, daeArray *lv, daeArray *rv, size_t iN)
	{
		daeCTC<sizeof(daeArray)==sizeof(void*)>(); //COLLADA__VIZDEBUG?
		_derive__COLLADA__move(obj,lv,rv,iN);
	}
	template<class S>
	/**HELPER Derived classes can use this to implement @c __COLLADA__move(). */
	static void _derive__COLLADA__move(const daeObject *obj, S *lv, S *rv, size_t iN, void (*mover)(S&,const S&)=nullptr)
	{
		daeCTC<sizeof(daeArray)==sizeof(void*)>(); //COLLADA__VIZDEBUG?
		for(size_t i=0;i<iN;i++)
		{
			lv[i]._au = rv[i]._au; if(mover!=nullptr) mover(lv[i],rv[i]);
		}
		_finish__COLLADA__move(obj,lv,rv,iN);
	}
	template<class S>
	static void _finish__COLLADA__move(const daeObject *obj, S *lv, S *rv, size_t iN)
	{
		const daeAlloc<T> &plug = daeAlloc<T>::localThunk();
		size_t i; for(i=0;i<iN;i++) rv[i]._thunk_au = &plug;
		if(obj!=nullptr)
		{
			daeOffset os = daeOffset(lv)-daeOffset(obj);
			for(size_t i=0;i<iN;i++,os+=sizeof(daeArray))
			{
				daeAllocThunk &au = static_cast<daeAllocThunk&>(*lv->_au);
				if(au._capacity>0) au._offset = os;
			}
		}
		else assert(i==0||rv[0].getObject()==nullptr);
	}  

COLLADA_(public) //daeArray<daeArray> (etc.) placement-new implementation

	/** __COLLADA__locate does cross-module virtual-method-table ownership. */
	inline void __COLLADA__locate(const daeObject *obj, const daeArray &cp)
	{
		//If obj!=nullptr then localThunk() cannot be used, as its offset must be 0.
		//In which case begin with a temporary thunk, set its offset, and be certain
		//that it grows, so the temporary thunk gets replaced with an allocation-unit.
		//Note: daeArray<T,!0>::__COLLADA__locate() is only *new(this)daeArray(obj)=cp;
		const daeAlloc<T> &lt = daeAlloc<T>::localThunk();
		daeAllocThunk tmp; tmp._offset = obj==nullptr?0:daeOffset(this)-daeOffset(obj);
		new(this) daeArray(tmp._offset==0?lt:lt.locateThunk(tmp));
		if(!cp.empty()||0!=tmp._offset) grow(cp.empty()?64/sizeof(T)+1:cp.size());
		for(size_t i=0;i<cp.size();i++)
		daeArrayAware<T>::place(this,_au->_varray+i,cp[i]);	_au->setInternalCounter(cp.size());
	}

COLLADA_(public) //OPERATOR OVERLOADS
	/**
	 * Gets the object at a specific index in the @c daeArray.
	 * @param index Index of the object to get, asserts if the index is out of bounds.
	 * @return Returns the object at @c index.
	 */
	inline T &operator[](size_t index){ assert(index<getCount()); return _au->_varray[index]; }
	/**CONST-FORM
	 * Gets the object at a specific index in the @c daeArray.
	 * @param index Index of the object to get, asserts if the index is out of bounds.
	 * @return Returns the object at @c index.
	 */
	inline const T &operator[](size_t index)const{ assert(index<getCount()); return _au->_varray[index]; }

	template<class S>
	/**
	 * Overloaded assignment operator.
	 * @param other A reference to the array to copy
	 * @return A reference to this object.
	 */
	inline daeArray<T> &operator=(const S &cp)
	{
		if((void*)this!=(void*)&cp) //LEGACY
		{
			size_t iN = cp.size();

			clear(); grow(iN);			
			{
				for(size_t i=0;i<iN;i++)
				daeArrayAware<T,typename daeAtomOf<S>::type>::place(this,_au->_varray+i,cp[i]);
			}
			_au->setInternalCounter(iN);
		}
		return *this;
	}
	/**
	 * Non-Default Assignment Operator
	 * C++ requires this, to not generate a default @c operator=.
	 */
	inline daeArray<T> &operator=(const daeArray &cp)
	{
		return operator=<daeArray>(cp);
	}

	template<class S>
	/**
	 * Overloaded @c operator==.
	 * @param other A reference to the other array.
	 * @return true if the arrays are equal, false otherwise.
	 */
	inline bool operator==(const S &cmp)const
	{
		size_t iN = size();
		if(iN!=cmp.size()) return false;
		for(size_t i=0;i<iN;i++) 
		if(_au->_varray[i]!=cmp[i]) return false; return true;
	}
	template<class S>
	/**
	 * Overloaded @c operator!=.
	 * @param other A reference to the other array.
	 * @return true if the arrays are equal, false otherwise.
	 */
	inline bool operator!=(const S &cmp)const{ return !(*this==cmp); }

COLLADA_(public) //INEFFICIENT LEGACY METHODS

  /////////////////////////////////////////////////////////
  //THESE METHODS NEED TO BE TIGHTENED UP, OR DEPRECATED.//
  //(OR BOTH.) AT LEAST THEY SHOULD NOT DOUBLE-CONSTRUCT.//
  /////////////////////////////////////////////////////////

	//template<class S> //Use push_back.
	/**LEGACY
	 * Appends a new object to the end of the @c daeArray.
	 * @param value Value of the object to append.
	 * @return Returns the index of the new object.
	 */
	inline size_t append(const T &value)
	{
		push_back(value); return _au->_counter-1; 
	}

	template<class S>
	/**LEGACY
	 * Appends a unique object to the end of the @c daeArray.
	 * Functions the same as @c append(), but does nothing if the value is already in the @c daeArray.
	 * @param value Value of the object to append.
	 * @return Returns the index where this value was appended. If the value already exists in the array, 
	 * returns the index in this array where the value was found.
	 */
	inline size_t appendUnique(const S &value)
	{
		size_t ret; return find(value,ret)!=DAE_OK?append(value):ret;
	}

	template<class S>
	/**LEGACY
	 * Adds a new item to the front of the @c daeArray.
	 * @param value Item to be added.
	 */
	inline void prepend(const S &value){ insertAt(0,value); }
	
	template<class S>
	/**LEGACY
	 * Finds an item from the @c daeArray.
	 * @param value A reference to the item to find.
	 * @param index If the function returns DAE_OK, this is set to the index where the value appears in the array.
	 * @return Returns DAE_OK if no error or DAE_ERR_QUERY_NO_MATCH if the value was not found.
	 */
	inline daeOK find(const S &value, size_t &index)const
	{
		size_t iN = getCount();
		for(size_t i=0;i<iN;i++) if(get(i)==value)
		{
			index = i; return DAE_OK;
		}
		return DAE_ERR_QUERY_NO_MATCH;
	}
	template<class S>
	/**LEGACY?
	 * @note This API had returned "get(i)" which is not a T*. That suggests
	 * it was never used by any software, and the compilers involved were not
	 * checking its completeness, or the C++ standard cannot catch such errors.
	 *
	 * Just like the previous function, but has a more reasonable interface.
	 * @param value The value to find.
	 * @return Returns a pointer to the value if found, null otherwise.
	 */
	inline T *find(const S &value)
	{
		size_t i; if(find(value,i)==DAE_OK) return _au->_varray+i; return nullptr;
	}
	template<class S>
	/**CONST-FORM, LEGACY?
	 * @note See non-const form's notes, etc. Pre-2.5 there was only a @c const
	 * form, that returned a non-const T*.
	 */
	inline const T *find(const S &value)const{ return ((daeArray*)this)->find(value); }
	
	/**LEGACY
	 * Inserts the specified number of elements at a specific location in the array.
	 * @param index Index into the array where the elements will be inserted
	 * @param n The number of elements to insert
	 * @param val The value to insert
	 */
	inline void insert(size_t index, size_t n)
	{
		return insert(index,n,T()); 
	}
	template<class S>
	/**LEGACY
	 * Inserts the specified number of elements at a specific location in the array.
	 * @param index Index into the array where the elements will be inserted
	 * @param n The number of elements to insert
	 * @param val The value to insert
	 */
	inline void insert(size_t index, size_t n, const S &val)
	{
		size_t i = getCount(), j, iN = index+n;
		if(index>=i)
		{
			//Append to the end of the array			
			setCount(iN);
			//2.5 BREAKING CHANGE: 
			//Was setting indices below index to val.
			//Now these are left equal to the default value of T().
			while(index<iN) get(index++) = val;
		}
		else
		{			
			setCount(i+=n);
			for(j=--i-n;i>=iN;) get(i--) = get(j--);
			for(i=index;i<iN;i++) get(i) = val;
		}
	}	

	template<class S>
	/**LEGACY
	 * Inserts an object at a specific index in the daeArray, growing the array if neccessary
	 * @param index Index into the array for where to place the object
	 * @param value The object to append
	 */
	inline void insertAt(size_t index, const S &value){ insert(index,1,value); }

	template<class S>
	/**LEGACY
	 * Removes an item from the @c daeArray.
	 * @param value A reference to the item to delete.
	 * @return Returns DAE_OK if success, a negative value defined in daeError.h otherwise.
	 * @note The @c daeElement objects sometimes list
	 * objects in two places, the class member and the <i> @c _contents </i> array, when you remove something from the
	 * do, you must remove it from both places.
	 */
	inline daeOK remove(const S &value, size_t *idx=nullptr)
	{
		size_t index;
		if(find(value,index)==DAE_OK)
		{
			if(idx!=nullptr) *idx = index; return removeIndex(index);
		}
		return DAE_ERR_QUERY_NO_MATCH;
	}

	/**LEGACY
	 * Removes an item at a specific index in the @c daeArray. 
	 * @param index Index number of the item to delete.
	 * @return Returns DAE_OK if success, a negative value defined in daeError.h otherwise.
	 * @note The @c daeElement objects sometimes list
	 * objects in two places, the class member and the <i> @c _contents </i> array, when you remove something from the
	 * dom, you must remove it from both places.
	 */
	inline daeOK removeIndex(size_t index)
	{
		size_t iN = getCount();
		if(index>=iN--) 
		return DAE_ERR_INVALID_CALL;
		for(size_t i=index;i<iN;i++) 
		get(i) = get(i+1);
		get(iN).~T();
		_au->setInternalCounter(iN);
		return DAE_OK;
	}

	template<class S>
	/**LEGACY
	 * Sets the array to the contain the two values specified.
	 * @param one The first value.
	 * @param two The second value.
	 */
	inline void set2(const S &one, const T &two)
	{
		setCount(2); set(0,one); set(1,two); 
	}
	template<class S>
	/**LEGACY
	 * Sets the array to the contain the three values specified.
	 * @param one The first value.
	 * @param two The second value.
	 * @param three The third value.
	 */
	inline void set3(const S &one, const T &two, const T &three)
	{
		setCount(3); set(0,one); set(1,two); set(2,three); 
	}
	template<class S>
	/**LEGACY
	 * Sets the array to the contain the four values specified.
	 * @param one The first value.
	 * @param two The second value.
	 * @param three The third value.
	 * @param four The fourth value.
	 */
	inline void set4(const S &one, const T &two, const T &three, const T &four)
	{
		setCount(4); set(0,one); set(1,two); set(2,three); set(3,four);
	}

	template<class S>
	/**LEGACY
	 * Sets the values in the array at the specified location to the contain the two 
	 * values specified. This function will grow the array if needed.
	 * @param index The position in the array to start setting.
	 * @param one The first value.
	 * @param two The second value.
	 */
	inline void set2at(size_t index, const S &one, const T &two)
	{
		set(index,one); set(index+1,two); 
	}
	template<class S>
	/**LEGACY
	 * Sets the values in the array at the specified location to the contain the three 
	 * values specified. This function will grow the array if needed.
	 * @param index The position in the array to start setting.
	 * @param one The first value.
	 * @param two The second value.
	 * @param three The third value.
	 */
	inline void set3at(size_t index, const S &one, const T &two, const T &three)
	{
		set(index,one); set(index+1,two); set(index+2,three);
	}
	template<class S>
	/**LEGACY
	 * Sets the values in the array at the specified location to the contain the four 
	 * values specified. This function will grow the array if needed.
	 * @param index The position in the array to start setting.
	 * @param one The first value.
	 * @param two The second value.
	 * @param three The third value.
	 * @param four The fourth value.
	 */
	inline void set4at(size_t index, const S &one, const T &two, const T &three, const T &four)
	{
		set(index,one); set(index+1,two); set(index+2,three); set(index+3,four);
	}

	template<class S, class T>
	/**LEGACY
	 * Appends two values to the array.
	 * @param one The first value.
	 * @param two The second value.
	 */
	inline void append2(const S &one, const T &two)
	{
		push_back(one); push_back(two); 
	}
	template<class S, class T, class U>
	/**LEGACY
	 * Appends three values to the array.
	 * @param one The first value.
	 * @param two The second value.
	 * @param three The third value.
	 */
	inline void append3(const S &one, const T &two, const U &three)
	{
		push_back(one); push_back(two); push_back(three);
	}
	template<class S, class T, class U, class V>
	/**LEGACY
	 * Appends four values to the array.
	 * @param one The first value.
	 * @param two The second value.
	 * @param three The third value.
	 * @param four The fourth value.
	 */
	inline void append4(const S &one, const T &two, const U &three, const V &four)
	{
		push_back(one); push_back(two); push_back(three); push_back(four);
	}

	template<class S, class T>
	/**LEGACY
	 * Inserts two values into the array at the specified location.
	 * @param index The position in the array to start inserting.
	 * @param one The first value.
	 * @param two The second value.
	 */
	inline void insert2at(size_t index, const S &one, const T &two)
	{
		insert(index,2); set(index,one); set(index+1,two);
	}
	template<class S, class T, class U>
	/**LEGACY
	 * Inserts three values into the array at the specified location.
	 * @param index The position in the array to start inserting.
	 * @param one The first value.
	 * @param two The second value.
	 * @param three The third value.
	 */
	inline void insert3at(size_t index, const S &one, const T &two, const U &three)
	{
		insert(index,3); set(index,one); set(index+1,two); set(index+2,three);
	}
	template<class S, class T, class U, class V>
	/**LEGACY
	 * Inserts four values into the array at the specified location.
	 * @param index The position in the array to start inserting.
	 * @param one The first value.
	 * @param two The second value.
	 * @param three The third value.
	 * @param four The fourth value.
	 */
	inline void insert4at(size_t index, const S &one, const T &two, const U &three, const V &four)
	{
		insert(index,4); set(index,one); set(index+1,two); set(index+2,three); set(index+4,four);
	}

	template<class S>
	/**LEGACY
	 * Gets one values from the array at the specified location.
	 * @param index The position in the array to start getting.
	 * @param one Variable to store the first value.
	 * @return Returns The number of elements retrieved.
	 */
	inline int get1at(size_t index, S &one)const
	{
		if(index>=getCount()) return 0; 
		COLLADA_SUPPRESS_C(4244) one = get(index); return 1;
	}
	template<class S, class T>
	/**LEGACY
	 * Gets two values from the array at the specified location.
	 * @param index The position in the array to start getting.
	 * @param one Variable to store the first value.
	 * @param two Variable to store the second value.
	 * @return Returns The number of elements retrieved.
	 */
	inline int get2at(size_t index, S &one, T &two)const
	{
		if(1!=get1at(index++,one)) return 0;
		if(1!=get1at(index++,two)) return 1; return 2;
	}
	template<class S, class T, class U>
	/**LEGACY
	 * Gets three values from the array at the specified location.
	 * @param index The position in the array to start getting.
	 * @param one Variable to store the first value.
	 * @param two Variable to store the second value.
	 * @param three Variable to store the third value.
	 * @return Returns The number of elements retrieved.
	 */
	inline int get3at(size_t index, S &one, T &two, U &three)const
	{
		if(1!=get1at(index++,one)) return 0;
		if(1!=get1at(index++,two)) return 1;
		if(1!=get1at(index++,three)) return 2; return 3;
	}
	template<class S, class T, class U, class V>
	/**LEGACY
	 * Gets four values from the array at the specified location.
	 * @param index The position in the array to start getting.
	 * @param one Variable to store the first value.
	 * @param two Variable to store the second value.
	 * @param three Variable to store the third value.
	 * @param four Variable to store the fourth value.
	 * @return Returns The number of elements retrieved.
	 */
	inline int get4at(size_t index, S &one, T &two, U &three, V &four)const
	{
		if(1!=get1at(index++,one)) return 0;
		if(1!=get1at(index++,two)) return 1;
		if(1!=get1at(index++,three)) return 2; 
		if(1!=get1at(index++,four)) return 3; return 4;
	}

	#ifdef NDEBUG
	#error use setCount in both	versions of appendArray--then check the rest.
	#endif
	/**LEGACY
	 * Appends a number of elements to this array from a C native array.
	 * @param num The number of elements to append.
	 * @param array The C native array that contains the values to append.
	 */
	inline void appendArray(size_t iN, const T *c_array)
	{
		if(c_array!=nullptr) //LEAGACY
		{	
			append(c_array,iN);
		}
	}
	/**LEGACY
	 * Appends a number of elements to this array from another daeArray.
	 * @param array The daeArray that contains the values to append.
	 */
	inline void appendArray(const daeArray<T> &other)
	{
		append(other.data(),other.getCount());
	}
};	

template<class T, int size_on_stack>
/**
 * COLLADA C++ array that initially lives on the stack, until it outgrows it. 
 *
 * @see the partial-template-specialization form, where @a size_on_stack is 0.
 *
 * @remarks 2D arrays can also live on the stack; they start out like C-arrays.
 * 2D arrays are not recommended, and should not be passed between modules, just
 * as any other type T that that allocates its own memory, should no be so passed.
 */
class daeArray : public daeArray<T,0>
{	  
	#ifdef COLLADA__VIZDEBUG
	daeAlloc<T,size_on_stack> &__vizAlloc; T (&__vizVArray)[size_on_stack];
	#endif

COLLADA_(public) 

	using daeArray<T,0>::_au;
	//using daeArray<T,0>::operator=;
	/**
	 * Non-Default Assignment Operator
	 * C++ requires this, to not generate a default @c operator=.
	 */
	inline daeArray &operator=(const daeArray &cp)
	{
		daeArray<T,0>::operator=<daeArray<T,0>>(cp); return *this;
	}

	template<class Type> //DAEP::Value<...>
	/**WARNING
	 * Prototype Constructor
	 *
	 * @todo If there are per-value compile-time-contants, the call
	 * to @c empty() etc. can be eliminated. For instance, if fixed
	 * then the copy is may not required. If there's no default, it
	 * is definitely unrequired.
	 *
	 * @warning This is a little too dicey because @c this can be a
	 * prototype, inside @c daeMetaElement::~daeMetaElement(). It's
	 * a defect maybe. Because of this, @c getCount() is used. This
	 * seems like a trap that it'd be better if it weren't possible.
	 */
	explicit daeArray(const DAEP::Proto<Type> &pt):
	#ifdef COLLADA__VIZDEBUG
	__vizAlloc(_stack_au),__vizVArray(__vizAlloc._varray),
	#endif
	daeArray<T,0>(*_au) //NOP
	{
		daeAlloc<T> *swap = _au; _au = (daeAlloc<T>*)&_stack_au;
		//There is no copy-on-write mechanism, so copy the defaults.
		if(pt.has_default>0||pt.is_fixed>0) 
		{
			//NOTE. COLLADA 1.4.1 DOES HAVE SOME OF THESE. 1.5.0 DOES NOT.
			//The generator is adding the number of spaces in the strings
			//to their value. It should be equal to the capacity since if
			//the generator uses this kind of array, the min-max is equal.
			//(This checks for creative users or when something's wrong.)
			daeCTC<(pt.has_default<=size_on_stack&&pt.is_fixed<=size_on_stack)>();
			assert(swap->getCapacity()==getCapacity());
			if(/*swap->getCapacity()>getCapacity()||*/!daeArrayAware<T>::is_plain)
			{
				_au->setInternalCounter(0);	*this = (daeArray&)swap;
			}
		}
	}
	/**
	 * Default Constructor
	 *
	 * @param real_size is available if variable initialization is required.
	 */
	daeArray(const daeObject *c=nullptr, size_t real_size=size_on_stack):
	#ifdef COLLADA__VIZDEBUG
	__vizAlloc(_stack_au),__vizVArray(__vizAlloc._varray),
	#endif//MSVC2013 is insisting on <T,0>. !!0 is to clarify which of them.
	daeArray<T,0>((daeAlloc<T>&)*new(&_stack_au)daeAlloc<T,!!0>(real_size))
	{ 
		_au->_offset = c==nullptr?0:(daeOffset)this-(daeOffset)c; 
	}

COLLADA_(private) //DATA-MEMBER

	union //stack/embedded AU data
	{
		COLLADA_DOM_INT64 _aligner; 	
		char _[sizeof(daeAlloc<T,size_on_stack>)];
		operator daeAlloc<T,size_on_stack>&(){ return (daeAlloc<T,size_on_stack>&)*this; }

	}_stack_au; friend class daeAlloc<>; //isEmbeddedAU()

COLLADA_(public) //daeArray<daeArray> (etc.) grow implementation

	typedef const daeObject __COLLADA__Object; 
	static void __COLLADA__move(const daeObject *obj, daeArray *lv, daeArray *rv, size_t iN)
	{
		_derive__COLLADA__move(obj,lv,rv,iN);		
	}	
	template<class S>
	/**HELPER Derived classes can use this to implement @c __COLLADA__move(). */
	static void _derive__COLLADA__move(const daeObject *obj, S *lv, S *rv, size_t iN, void (*mover)(S&,const S&)=nullptr)
	{
		for(size_t i=0;i<iN;i++)
		{
			new(lv+i) S; //daeArray;
			if(mover) mover(lv[i],rv[i]);
			if(daeAlloc<>::isEmbeddedAU(rv[i]._au))
			{
				rv[i]._au->moveThunk(lv[i]._au); _copier(obj,*lv[i]._au,*rv[i]._au);
			}
			else lv[i]._au = rv[i]._au;
		}//Set lv offsets & assign rv thunks.
		_finish__COLLADA__move(obj,lv,rv,iN);		
	}

COLLADA_(public) //daeArray<daeArray> (etc.) placement-new implementation

	/** __COLLADA__locate does cross-module virtual-method-table ownership. */
	inline void __COLLADA__locate(const daeObject *obj, const daeArray &cp)
	{
		*new(this)daeArray(obj) = cp;
	}
};

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_ARRAY_H__
/*C1071*/