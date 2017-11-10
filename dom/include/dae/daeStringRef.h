/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_STRING_REF_H__
#define __COLLADA_DOM__DAE_STRING_REF_H__

#include "daeArray.h"
#include "daeDatabase.h"
  
COLLADA_(namespace)
{//-.
//<-'

template<class This>
/**SUPPORT-TEMPLATE
 * This support class implements functionality that is common to
 * @c daeHashString, @c daeRefView, and @c daeStringRef. That is
 * @c operator==() and @c operator!=(). Getting those right is a
 * lot of work, which is the real reason for this class existing.
 * Nothing should be added to this class lightly. There's reason
 * for the classes being separate, and anything added might only
 * make sense in one case or the other; but not all 3 characters.
 */
struct daeString_support
{
COLLADA_(public) //daeArray traits

	typedef void __COLLADA__atomize;

COLLADA_(public)
	/**
	 * COLLADA 1.4.1 and 1.5.0 has many crazy ID
	 * like # fragment addressing modes. This is
	 * used to detect a / implicating its SIDREF.
	 */
	inline bool getID_slashed()const
	{
		This &_this = *(This*)this;
		for(size_t i=0;i<_this.size();i++)		
		if('/'==_this[i]) return true; return false;
	}

	//These simplify std::vector::insert, etc.
	/**
	 * Implements @c begin() in terms of @c data().
	 */
	daeString begin()const{ return ((This*)this)->data(); }
	/**
	 * Implements @c end() in terms of @c data()+size().
	 */
	daeString end()const{ return begin()+((This*)this)->size(); }

COLLADA_(public) //COMPARISON OPERATORS

	template<class T> struct _eq
	{	
		template<class S>
		/**
		 * @return Returns @c true if two string/size
		 * pairs are equal or @c nullptr is equal, but
		 * does not compile if comparing two C-strings.
		 */
		static bool op(const This &_this, const S &cmp)
		{
			return _op2((T*)nullptr,_this,cmp);
		}
		
		template<class S>
		/**
		 * This is just for comparing to @c nullptr. 
		 * @tparam T Not all systems have @c nullptr.
		 * It can't be used with @c daeString(nullptr).
		 */
		static bool _op2(daeString*,const This &_this, const S &nul_or_0)
		{
			class undefined *upcast = (S)0; assert(0==nul_or_0); (void)upcast;
			return nullptr==_this.data();
		}		
		/** 
		 * This is the same as @c daeStringEqualFunctor() below. 
		 * @tparam T lets @c daeHashString be an icomplete type.
		 */
		static bool _op2(daeHashString*, const This &_this, const T &cmp)
		{
			return _this.size()==cmp.size()&&0==memcmp(_this.data(),cmp.data(),cmp.size());
		}		
	};
	template<class T>
	/**
	 * Overloading can't pick up on the array forms.
	 */
	inline bool operator==(const T &cmp)const
	{
		return _eq<typename daeBoundaryString2<T>::type>::op((const This&)*this,cmp); 
	}	
	template<class T>
	/**
	 * Implements @c operator!=() relative to @c operator==(). 
	 */
	inline bool operator!=(const T &cmp)const
	{
		return !_eq<typename daeBoundaryString2<T>::type>::op((const This&)*this,cmp); 
	}	
	template<int N> 
	/**
	 * This does string-literal on the left side equality. The right side doesn't
	 * appear to try to construct a @c daeHashString. (Good.)
	 */
	friend bool operator==(const daeStringCP (&cmp)[N], const This &_this)
	{
		return _eq<daeHashString>::op(_this,cmp); 
	}
	template<int N> 
	/**
	 * This does string-literal on the left side equality. The right side doesn't
	 * appear to try to construct a @c daeHashString. (Good.)
	 */
	friend bool operator!=(const daeStringCP (&cmp)[N], const This &_this)
	{
		return !_eq<daeHashString>::op(_this,cmp); 
	}	
};

extern daeString daeStringRef_empty;
/**
 * Defines the @c daeStringRef class.
 *
 * Historically this class aspired to be a hashed string-table setup.
 * In reality it was a pump-and-dump (nuke) bulk allocation strategy.
 *
 * THE FUTURE IS UNCERTAIN
 *
 * It's difficult to say if this is worthwhile or not. Databases can
 * decide on what works and what doesn't, and clients/users by proxy.
 */
class daeStringRef : public daeString_support<daeStringRef>
{
COLLADA_(public)

	template<class Type> //DAEP::Value<...>
	/**
	 * Prototype Constructor
	 *
	 * This constructor transfers the schema's default strings
	 * to the database string-ref pool. The empty-string cases
	 * go along an @c inline pathway. Default-strings go along
	 * a @c COLLADA_DOM_LINKAGE path.
	 */
	explicit daeStringRef(const DAEP::Proto<Type> &pt)
	{	
		//HACK: underlying_type should be daeStringRef.
		const COLLADA_INCOMPLETE(Type)
		daeDOM *DOM = dae(pt.object()).getDOM();		
		if(pt.has_default||pt.is_fixed) 
		{
			_ref(*DOM); assert(!empty());
		}
		else _string = DOM->getEmptyURI().data(); 
	}

COLLADA_(public) //daeArray traits

	typedef const daeObject __COLLADA__Object;
	template<class T>
	inline void __COLLADA__place(const daeObject *obj, const T &cp)
	{
		_ref(obj,cp); 
	}
	template<class T>
	static void __COLLADA__move(const daeObject *obj, T *lv, T *rv, size_t iN)
	{
		daeCTC<sizeof(T)==sizeof(void*)>(); //daeTokenRef
		memcpy(lv,rv,iN*sizeof(void*)); //raw-copyable style move.
	}	

COLLADA_(public)
	/**
	 * Default Constructor
	 * @warning Historically it set @c _string=nullptr.
	 */
	daeStringRef()
	{ 		
		//_ref() assigns the system-pool's empty string. It would
		//be nice if the pointer could be assigned directly inline.
		//Are data exports portable? DOM_process_share could hold it. 
		#ifdef BUILDING_COLLADA_DOM
		_string = daeStringRef_empty;
		#else
		_ref();
		#endif
	}		
	template<class LAZY> //Object or daeBoundaryString2 type.
	/**WARNING, EXPLICIT
	 * @warning THIS IS NOW TWO-DISTINCT CONSTRUCTORS IN ONE.
	 * (It had to be merged for C++ templates reasons.)
	 *
	 * @tparam LAZY can be either, a DAEP Object based pointer
	 * or object-reference, or a number of string-type objects.
	 * Objects arguments yield an empty string in the object's
	 * pool. String arguments are drawn from the system's pool.
	 * 
	 * DEFAULT-CONSTRUCTOR CONTINUED
	 * =============================
	 * 1) @c DAEP::Object based single-argument. This form is
	 * chiefly for @c daeArray::getObject(). It can be thought
	 * of as an extension of the default-constructor, receiving
	 * a DOM to locate its empty-string in.
	 *
	 * STRING-BASED SYSTEM-POOL CONSTRUCTOR
	 * ====================================
	 * 2) System-Pool Constructor
	 *
	 * This is not a replacement for the old @c daeString based
	 * constructor. Pre-2.5 @c daeStringRef didn't have ownership
	 * semantics. The system-pool is a special pool that's used for
	 * metadata defaults, xs:any style client-strings, and long names.
	 *
	 * This method of construction is provided so clients can construct
	 * comparators. It's used by @c daeAlloc<daeStringRef>::localThunk()
	 * and equivalents for @c daeStringRef based classes.
	 * @see @c daeTokenRef.
	 */
	explicit daeStringRef(const LAZY &cp)
	{
		//Incorporating daeHashString complicates this constructor.
		_string_or_object(cp,nullptr);
	}
	template<int N>
	/**LEGACY Minimal non-const buffer support. */
	explicit daeStringRef(daeString (&cp)[N])
	{
		_ref((daeString)cp,N); 
	}
	template<class T> 
	/**HACK Implements single-argument constructor. Receives a DAEP Object. */
	void _string_or_object(T *obj, typename T::__DAEP__Object__signed*)
	{
		if(obj!=nullptr) _string_or_object(*obj,nullptr);
		else new(this) daeStringRef(); //Set to "" via the system-pool.
	}
	template<class T> 
	/**HACK Implements single-argument constructor. Receives a DAEP Object. */
	void _string_or_object(T &obj, typename T::__DAEP__Object__signed*)
	{
		const COLLADA_INCOMPLETE(T)
		daeDOM *DOM = dae(obj).getDOM();
		if(DOM==nullptr) 
		{
			new(this) daeStringRef(); //Set to "" via the system-pool.
		}
		else _string = DOM->getEmptyURI().data();
	}
	template<class T> 
	/**HACK Implements single-argument constructor. Receives non DAEP Object. */
	void _string_or_object(T &cp,...)
	{
		_ref(typename daeBoundaryString2<T>::type(cp));
	}
	/**WARNING
	 * Copy Constructor
	 * It's not clear when this would be useful to use internally.
	 * @warning @a cp must belong to the same object, or the constructed ref
	 * must be outside of objects, or of a limited scope, tied to the scope 
	 * of the object being copied from.
	 * @note Some initialization routines may have this as a subroutine.
	 */
	daeStringRef(const daeStringRef &cp):_string(cp)
	{
		//_rollover lets pools prioritize strings and handle the case of
		//a ref-counter having overflowed the 16-bit counter. This is not
		//a constructor that would see heavy use by itself. Normally a set
		//API calls it, along with ~daeStringRef(), which has to _release().
		enum{ arbitrary_power_of_2=256 };
		if(_ptr->ref()%arbitrary_power_of_2==0) 
		_rollover(const_cast<daeStringRef&>(cp),arbitrary_power_of_2);
	}

	#ifdef NDEBUG
	#error Can this and the above/below possibly be refactored to allow
	#error for non-const buffer assignment?
	#endif
	//REMINDER
	//daeElement is no longer based on DAEP Element.
	template<class LAZY>
	/**OPTIMIZING, CIRCULAR-DEPENDENCY
	 * Constructor
	 * An element should always have a DOM, and they
	 * will be getting a direct pointer to their DOM.
	 */
	daeStringRef(const daeElement &c, const LAZY &cp)
	{
		const COLLADA_INCOMPLETE(LAZY) daeElement &i = c;

		_ref(i.getDOM(),cp); //Emulate the DOM constructor.
	}			
	template<class LAZY>
	/**OPTIMIZING, CIRCULAR-DEPENDENCY
	 * Constructor
	 * An element should always have a DOM, and they
	 * will be getting a direct pointer to their DOM.
	 */
	daeStringRef(const DAEP::Element &c, const LAZY &cp)
	{
		new(this) daeStringRef(dae(c),cp);
	}	
	template<class LAZY>
	/**OPTIMIZING, CIRCULAR-DEPENDENCY
	 * Constructor with @a extent argument
	 * An element should always have a DOM, and they
	 * will be getting a direct pointer to their DOM.
	 */
	daeStringRef(const daeElement &c, const LAZY &cp, size_t extent)
	{
		const COLLADA_INCOMPLETE(LAZY) daeElement &i = c;

		_ref(i.getDOM(),cp,extent); //Emulate the DOM constructor.
	}	
	template<class LAZY>
	/**OPTIMIZING, CIRCULAR-DEPENDENCY
	 * Constructor with @a extent argument
	 * An element should always have a DOM, and they
	 * will be getting a direct pointer to their DOM.
	 */
	daeStringRef(const DAEP::Element &c, const LAZY &cp, size_t extent)
	{
		new(this) daeStringRef(dae(c),cp,extent);
	}		

	/**
	 * System-Pool Constructor with @a extent argument
	 * @see @c explicit form of this constructor's Doxygentation.
	 * @param extent Technically @c cp[extent] doesn't have to be a 0-terminator.
	 * However, it's on database implementors to implement this correctly, and a
	 * performance loss can be incurred if the string must be copied in order to
	 * do so. With @c COLLADA_DOM_UNORDERED_SET there is no loss for system-refs.
	 */
	daeStringRef(daeBoundaryStringIn cp, size_t extent){ _ref(cp.c_str,extent); }
	/**
	 * Constructor
	 * The other forms more-or-less extract the DOM.
	 */
	daeStringRef(const daeDOM &DOM, const daeStringRef &cp)
	{
		_ref(DOM,cp); 
	}
	/**
	 * Constructor
	 */
	daeStringRef(const DAEP::Object &c, const daeStringRef &cp)
	{
		_ref(c,cp); 
	}
	template<class LAZY>
	/**
	 * Constructor
	 * The other forms more-or-less extract the DOM.	 
	 */
	daeStringRef(const daeDOM &DOM, const LAZY &cp)
	{
		_ref(DOM,daeBoundaryString2<LAZY>::type(cp)); 
	}
	/**
	 * Constructor with @a extent argument
	 * The other forms more-or-less extract the DOM.
	 * @param extent Technically @c cp[extent] doesn't have to be a 0-terminator.
	 * However, it's on database implementors to implement this correctly, and a
	 * performance loss can be incurred if the string must be copied in order to
	 * do so. With @c COLLADA_DOM_UNORDERED_SET there is no loss for system-refs.
	 */
	daeStringRef(const daeDOM &DOM, daeBoundaryStringIn cp, size_t extent)
	{
		_ref(DOM,cp.c_str,extent); 
	}
	template<class LAZY>
	/**
	 * Constructor	 
	 */
	daeStringRef(const DAEP::Object &c, const LAZY &cp)
	{
		_ref(c,typename daeBoundaryString2<LAZY>::type(cp));
	}		
	/**
	 * Constructor with @a extent argument
	 * @param extent Technically @c cp[extent] doesn't have to be a 0-terminator.
	 * However, it's on database implementors to implement this correctly, and a
	 * performance loss can be incurred if the string must be copied in order to
	 * do so. With @c COLLADA_DOM_UNORDERED_SET there is no loss for system-refs.
	 */
	daeStringRef(const DAEP::Object &c, daeBoundaryStringIn cp, size_t extent)
	{
		_ref(c,cp.c_str,extent); 
	}		
	/**
	 * Destructor
	 * Ref counting is required to deal with large text objects.
	 * An allocator can assign 2 refs to small strings to ensure
	 * that they won't call the exporeted API.
	 */
	~daeStringRef(){ if(0==_ptr->release()) _release(); }
	
	/**
	 * @c daeString_support requires this.
	 */
	inline daeString data()const{ return _string; }
	/**
	 * @c daeString_support requires this.
	 *
	 * Gets the size of this string, excluding the 0 termintor.
	 *
	 * %2 is for "id" attributes. It might make sense to do it
	 * only for a "daeStringRefID" class. All strings have a #.
	 */
	inline daeUInt size()const
	{
		//Compilers should combine %2 with _Ptr::operator->().
		return _ptr->fragmentN-size_t(_string)%2; 
		daeCTC<sizeof(size_t)==sizeof(void*)>(); //unsigned intptr_t??
		daeCTC<sizeof(size())==daeSizeOf(daeDBaseString,fragmentN)>();
	}		
	/**
	 * The "" string is expected to be pool-agnostic. This is
	 * to accelerate the construction of DAEP Element objects.
	 */
	inline bool empty()const{ return _string[0]=='\0'; };

	/**LEGACY
	 * Sets a string from an external @c daeStringRef.
	 * @param cp The daeString to copy.
	 * @see @c setRef() to change pools.
	 * @see @c reset() to assert that the pools are the same.
	 */
	inline void set(const daeStringRef &cp)
	{
		if(_ptr->pool==cp._ptr->pool)
		{
			this->~daeStringRef(); new(this) daeStringRef(cp);
		}
		else _ref_and_release(cp);
	}
	template<class LAZY>
	/**LEGACY
	 * Sets a string from an external @c daeString.
	 * @param cp The daeString to copy.
	 * @see @c setString().
	 */
	inline void set(const LAZY &cp)
	{
		_ref_and_release(typename daeBoundaryString2<LAZY>::type(cp));
	}
	template<int N>
	/**LEGACY Minimal non-const buffer support. */
	inline void set(daeString (&cp)[N])
	{
		_ref_and_release((daeString)cp,N);
	}
	/**
	 * Sets a string from an external @c daeString.
	 * @param cp The daeString to copy.
	 * @see @c setString().
	 */
	inline void set(daeBoundaryStringIn cp, size_t extent)
	{
		_ref_and_release(cp,extent);
	}

	/**
	 * Explicitly sets a string-ref via a ref without a same pool check.
	 * @note It's safe to change pools, although it's hard to think of a
	 * reason to do so.
	 * @param cp The daeString to copy.
	 */
	inline void setRef(const daeStringRef &cp)
	{
		this->~daeStringRef(); new(this) daeStringRef(cp);
	}

	/**
	 * Explicitly sets a string-ref via a ref belonging to the same pool.
	 * Does @c assert(_ptr->pool==cp.ptr->_pool);
	 * @param cp The daeString to copy.
	 */
	inline void reset(const daeStringRef &cp)
	{
		assert(_ptr->pool==cp._ptr->pool);
		this->~daeStringRef(); new(this) daeStringRef(cp);
	}

	template<class LAZY>
	/**
	 * Explicitly sets a string from an external @c daeStringRef.
	 * @param cp The daeString to copy.
	 */
	inline void setString(const LAZY &cp)
	{
		_ref_and_release(daeBoundaryString2<LAZY>::type(cp)); 
	}
	/**
	 * Explicitly sets a string from an external @c daeStringRef.
	 * @param cp The daeString to copy.
	 */
	inline void setString(daeBoundaryStringIn cp, size_t extent)
	{
		_ref_and_release(cp,extent); 
	}
								
COLLADA_(public) //OPERATORS	

	typedef const daeStringCP __COLLADA__Atom;

	template<class T>
	/**OPERATOR
	 * Sets a string from an external @c daeString.
	 * @param cp The daeString to copy.
	 * @return A reference to this object.
	 */
	inline daeStringRef &operator=(const T &cp)
	{
		set(cp); return *this; 
	}
	template<int N>
	/**LEGACY Minimal non-const buffer support. */
	inline daeStringRef &operator=(daeString (&cp)[N])
	{
		set(cp); return *this;
	}
	/**C++ BUSINESS
	 * This must be present or C++ will generate @c operator=.
	 */
	inline daeStringRef &operator=(const daeStringRef &cp)
	{
		set(cp); return *this; 
	}

	/**OPERATOR
	 * Converts to  @c daeString. 
	 */
	inline operator daeString()const{ return _string; }
		
	template<class I>	
	/**
	 * C-string style array accessor. 
	 * @return Returns by address to satisfy @c DAEP::Value::operator[]().
	 */
	inline const daeStringCP &operator[](const I &i)const
	{
		return _string[i]; 
	}

	using daeString_support::operator==;
	/**
	 * Pools must guarantee strict-equality. 
	 */
	inline bool operator==(const daeStringRef &cmp)const
	{
		return _string==cmp._string;
	}
	using daeString_support::operator!=;
	/**
	 * Pools must guarantee strict-equality. 
	 */
	inline bool operator!=(const daeStringRef &cmp)const
	{
		return _string!=cmp._string;
	}

	template<class T>
	/**WARNING
	 * @warning Defect? This is not a lexical comparison. 
	 * @c daeStringRef shouldn't support <, <=, >, nor >=.
	 * To do so, access the string directly via other means.
	 *
	 * @param cmp must be convertible to a @c daeStringRef&.
	 * This had been specialized under Visual Studio, but it's
	 * too messy elsewhere and this can handle wrappers as well.
	 */
	inline bool operator<(const T &cmp)const
	{
		//Casting to prevent copy construction.
		daeStringRef &ref = const_cast<T&>(cmp);
		return _string<ref._string;
	}

COLLADA_(public) //daeDBaseString API
	/**WARNING
	 * @warning DON'T FORM A REFERENCE TO THE RETURNED REFERENCE
	 * IF IT'S TO BE USED AFTER A SEMICOLON (;) STATEMENT. SORRY!
	 *
	 * The library should find a way to guarantee this always
	 * works for strings originating from @c XS::Schema::getIDs()
	 * registered attribute names.
	 * For the time being it should work with all @c daeStringRef.
	 */
	const daeStringRef &getID_fragment
	(class undefined*_=0,const daeString &def=0)const
	{
		(daeString&)def = (daeString)((daeOffset)_string&~daeOffset(1));
		assert('#'==def[0]); return (daeStringRef&)def;
	}
	/**WARNING
	 * @warning DON'T FORM A REFERENCE TO THE RETURNED REFERENCE
	 * IF IT'S TO BE USED AFTER A SEMICOLON (;) STATEMENT. SORRY!
	 *
	 * This does the inverse of @c getID_fragment().
	 */
	const daeStringRef &getID_id
	(class undefined*_=0,const daeString &def=0)const
	{
		(daeString&)def = (daeString)((daeOffset)_string|daeOffset(1));
		assert('#'==def[-1]); return (daeStringRef&)def;
	}

	/**WARNING
	 * Gets the underlying reference count. 
	 *
	 * @warning This is not for monkeying with refs.
	 * COLLADA encourages high-frequency SIDs which
	 * are very difficult to dereference because if
	 * a bottom-up search is used it must eliminate
	 * many many SIDs. But a top-down search has to
	 * consider the entire document downstream from
	 * the starting point. A middle-ground is to do
	 * a one level deep search if the SID is a high
	 * frequency, and fall back to a slow bottom-up.	 
	 * THIS STRATEGY IS IMPERFECT, BUT THOUGHT GOOD
	 * ENOUGH FOR THE BUILT-IN, PROVIDED FACILITIES.
	 */
	inline size_t getSID_frequency()const{ return (size_t)_ptr->refs; }

COLLADA_(private) //DATA-MEMBER

	struct _Ptr //Emulate daeSmartRef::_ptr.
	{		
		daeString string;
		/**OPERATOR (previously of daeStringRef.)
		 * Gets the underlying @c daeDBaseString. 
		 * It doesn't really have public methods.
		 * @see @c daeStringRef::size() about %2.
		 */
		inline daeDBaseString *operator->()const
		{
			//This might better be computed with "diff&~size_t(1);" This trusts the compiler output.
			return (daeDBaseString*)(string-size_t(string)%2-daeOffsetOf(daeDBaseString,fragment));
			daeCTC<1==sizeof(daeStringCP)>();
		}
	};	
	union //daeStringRef itself can't be a union.
	{
		/**
		 *Gets operator-> out of daestringRef.
		 */
		_Ptr _ptr;
		
		/**
		 * This is really a ref-counted @c daeDBaseString.
		 */
		daeString _string;
	};

COLLADA_(private) //daeStringRef.cpp business

	//Every permutation is implemented.
	//It's impenetrable, but there's nothing to gain by restating it.
	COLLADA_DOM_LINKAGE void _release();
	COLLADA_DOM_LINKAGE void _rollover(daeStringRef&,int);	
	COLLADA_DOM_LINKAGE void _ref_and_release(daeString); //assignment
	COLLADA_DOM_LINKAGE void _ref_and_release(daeString,size_t); //assignment
	COLLADA_DOM_LINKAGE void _ref_and_release(const daeStringRef&); //assignment
	COLLADA_DOM_LINKAGE void _ref_and_release(const daeHashString&); //assignment
	COLLADA_DOM_LINKAGE void _ref(const daeObject*,daeString); //array-emplacement
	COLLADA_DOM_LINKAGE void _ref(const daeObject*,daeString,size_t); //array-emplacement
	COLLADA_DOM_LINKAGE void _ref(const daeObject*,const daeStringRef&); //array-emplacement
	COLLADA_DOM_LINKAGE void _ref(const daeObject*,const daeHashString&); //array-emplacement
	COLLADA_DOM_LINKAGE void _ref(const daeDOM&); //prototype-constructor
	COLLADA_DOM_LINKAGE void _ref(const daeDOM&,daeString); //constructor/prototype-constructor
	COLLADA_DOM_LINKAGE void _ref(const daeDOM&,daeString,size_t); //constructor/prototype-constructor
	COLLADA_DOM_LINKAGE void _ref(const daeDOM&,const daeStringRef&); //constructor
	COLLADA_DOM_LINKAGE void _ref(const daeDOM&,const daeHashString&); //constructor
	COLLADA_DOM_LINKAGE void _ref(const DAEP::Object&,daeString); //constructor
	COLLADA_DOM_LINKAGE void _ref(const DAEP::Object&,daeString,size_t); //constructor
	COLLADA_DOM_LINKAGE void _ref(const DAEP::Object&,const daeStringRef&); //constructor	
	COLLADA_DOM_LINKAGE void _ref(const DAEP::Object&,const daeHashString&); //constructor	
	COLLADA_DOM_LINKAGE void _ref(); //system-pool's empty-string
	COLLADA_DOM_LINKAGE void _ref(const daeString); //system-pool/subroutine
	COLLADA_DOM_LINKAGE void _ref(const daeString,size_t); //system-pool/subroutine
	COLLADA_DOM_LINKAGE void _ref(const daeStringRef&); //same, inaccessible
	COLLADA_DOM_LINKAGE void _ref(const daeHashString&); //same, inaccessible
};/** This is a shorthand for Microsoft's natvis product. */
static const daeOffset daeStringRef_debase = daeOffsetOf(daeDBaseString,fragment);

/**INTERNAL
 * This class registers @c daeStringPool.
 */
class daeStringPool_pool
{	
	template<class>
	friend class daeStringPool;
	/**
	 * Constructor
	 * This just assigns @c pool. 
	 */
	COLLADA_DOM_LINKAGE daeStringPool_pool();
	/**WARNING
	 * Destructor
	 * This just removes @c pool.
	 */
	COLLADA_DOM_LINKAGE ~daeStringPool_pool();

COLLADA_(public)
	/**
	 * This corresponds to @c daeDBaseString::pool.
	 * The only real reason it's not stored there as
	 * a pointer, is to try to not make small-strings
	 * significantly larger, and to encapsulate it all.
	 */
	const size_t pool;
};
template<class T=daeDBaseString>
/**ABSTRACT INTERFACE
 * String pool base class and pool association logic.
 *
 * @c daeDatabase implementors must implement a pool.
 * The library is not prescriptive about how they work.
 * Indeed, experimentation is encouraged.
 * @see The daeStringRef.cpp system-pool is a reference.
 *
 * Databases can contain multiple DOMs. The DOMs hold
 * a pointer to an empty string-ref, that is supplied by
 * the database. This ref connects the DOM's string-refs
 * to the pool that the empty string-ref belongs to.
 * This doesn't mean that there can only be one pool per
 * DOM, but it does mean that for all practical purposes
 * string-refs that begin their lives as empty-strings
 * will always start out in the same pool.
 *
 * Databases can share pools, and can even use the system
 * pool, although this is not recommended. The default
 * built-in database uses the system-pool. It's in better
 * position to do so, since it's linked to the library.
 * Still, that database is only for limited applications.
 * @see @c daeLegacyDBase. Notice how it doesn't register
 * the system-pool. It just forwards calls to the system
 * pool constructors of @c daeStringRef. 
 *
 * Pools must be registered: or assigned to a 16-bit ID.
 * The ID corresponds to @c daeDBaseString::pool. 0's is
 * the system-pool. It's currently not possible to have
 * a block of pool IDs. The exported constructor assigns
 * an ID to the pools as they are constructed.
 *
 * As stated elsewhere, prototypes use the system-pool,
 * -and a @c daeArray that doesn't have an object will
 * end up using it also; as do @c daeStringRef that do
 * not have an object argument while being constructed.
 */
class daeStringPool : public daeStringPool_pool
{	
COLLADA_(public)
	/**HELPER
	 * This is a small helper class to ease writing
	 * a pool interface, that databases must implement.
	 */
	union String
	{
		/**
		 * This is the same one @c daeStringRef holds.
		 */
		daeString string;

		struct _Size
		{
			operator size_t()
			{
				return ((daeStringRef*)this)->size(); 
			}
		/**
		 * Gets the size of the @c daeStringRef string.
		 */
		}size;

		String(daeString str):string(str){}		

		/**
		 * Accesses the front of the @c daeStringRef string.
		 */
		inline T *operator->(){ return *this; }
		/**
		 * @c delete can be used on this.
		 */
		inline operator T*()
		{
			return (T*)(string-size_t(string)%2-daeOffsetOf(T,fragment));
		}
	};

COLLADA_(public) //ABSTRACT INTERFACE

	/**ABSTRACT-INTERFACE
	 * Provides the pool's interface for releasing string-refs.
	 */
	virtual void release(String string) = 0;

	/**ABSTRACT-INTERFACE
	 * Provides the pool's interface for assigning to string-refs.
	 * @c cp[extent] is NOT required to be a 0-terminator--though
	 * @c ref(cp,extent)[extent] IS required to be a 0 terminator.
	 */
	virtual daeString ref(daeString cp, size_t extent) = 0;

	/**ABSTRACT-INTERFACE, WARNING
	 * @warning Implementing this API alone will not correctly deal
	 * will rollovers! This API addresses cases where databases are
	 * bypassed. Whenever a database increases the ref-counters, it
	 * must also perform the same logic that it applies in this API.
	 * @see the @c daeStringRef copy-constructor.
	 * 
	 * This is messy. Every @a factor references this interface is
	 * called. Here the pool can take any action it sees fit.
	 * The returned string is assigned to a calling @c daeStringRef.
	 * This provides an opportunity to switch it to another pointer,
	 * -keeping in mind that its reference will need to be released.
	 * The returned string can belong to a different pool. Small
	 * strings that enforce strict-equality should not be changed,
	 * -it would do no good, even if all instances could be updated.
	 *
	 * The ref-counter only goes up to 65535. If there are more than
	 * this many instances, then a pool must take some action.
	 * @note The limit was going to be 32767, to permit error checks,
	 * -but it's 65535, so the final rollover sets to 0, so that the
	 * database can easily detect this most dire of scenarios. 
	 * (Also, signed figures aren't neatly divisible by power of 2.)
	 *
	 * @note "returned string" above refers to @a string. The source
	 * string-ref can also be changed via @a source. Changing source 
	 * is more likely to stop propogation, which is important mainly
	 * to avoid "fighting" where @c rollover() is called as a result
	 * of a ref-counter going back-and-forth on a multiple of factor.
	 * In general it cannot hurt to have access to the source string.
	 */
	virtual void rollover(String &string, String &source, int factor) = 0;
};

/**INTERNAL
 * @c daeStringAlligator implements a one-way memory pool.
 * It's definitely not limited to strings, but that's the
 * only thing it's used for, and why it exists originally.
 *
 * @tparam N is roughly the number of @a T to reserve. In
 * reality N should be double-or-so, because usually some
 * accessory structures are involved, in fact rarely will
 * T be allocated by itself, if ever.
 */
template<class T, int N=0> struct daeStringAlligator
{
	union _Maw
	{
		char *mem; _Maw *old; 		

		operator char*(){ return mem; } 

		_Maw &operator*(){ return *old; } 

	}_maw;

COLLADA_(public) 

	//This is the current bucket.
	//THERE SHOULD ALSO BE A DELETIONS LIST,
	//-BECAUSE unordered_set/map LIKELY USE vector(s).
	T *_nexT; char *_end;

COLLADA_(public) 
	/**
	 * Adds a new block of memory to the ever growing pool.
	 */
	inline void _reserve(size_t geometric_growth)
	{
		_Maw tail = _maw; 
		assert(geometric_growth>sizeof(_Maw));
		_maw.mem = (char*)operator new(geometric_growth);				
		*_maw = tail;
		_nexT = (T*)(_maw+sizeof(_Maw)); 
		_end = _maw+geometric_growth;
	}
	/**
	 * Calls @c _reserve() with generic geometric function.
	 * 5/4 increases the size by 25% each time. Maybe it's
	 * too aggressive, but it's a simple fraction.
	 */
	inline void _reserve(){ _reserve(_reserveN()); }   
	inline int _reserveN(){ return (_end-_maw)*5/4; }

	/**UNUSED/ILLUSTRATIVE
	 * This was carried over from @c daeSmallStringTable::reset().
	 */
	inline void _reset(size_t r=sizeof(T)*N)
	{
		this->~daeStringAlligator(); new(this) daeStringAlligator(r);
	}

	/**
	 * Constructor
	 *
	 * This is redesigned so that no memory is allocated until it
	 * is requested. It would do @c _reserve(r) otherwise.
	 * @note It's not clear if @c std::unordered_map/set allocate buckets
	 * as soon as they are constructed. Maybe the bucket argument is stored
	 * instead, so it can be consulted on first insertion.
	 *
	 * 32 is added to @a r by default, since there are always accessory
	 * structures involved, including the strings, their extents, hashes, etc.
	 */
	daeStringAlligator(size_t r=(sizeof(T)+32)*N):_nexT((T*)r),_end((char*)r)
	{
		_maw.mem = nullptr; 
	}
	/** Destructor */
	~daeStringAlligator(){ for(_Maw m=_maw,o;m!=nullptr;delete m,m=o) o = *m; }
};
/**INTERNAL
 * @c daeStringAllocator implements a Standard Library allocator. 
 * It's based on @c daeStringAlligator in order to keep the STL's
 * requirements separate.
 * Actually, it's very complicated, and difficult to meet all of
 * the various requirements.
 */
template<class T, int N> struct daeStringAllocator
{	
COLLADA_(public)

	typedef daeStringAlligator<T,N> Alligator;
	/**
	 * This has to be wrapped in a class or MSVC2013
	 * won't work with it. It has to be reference-like
	 * as it's liberally passed around; a stateful setup
	 * is out of the question.
	 */
	Alligator *A;

	/**
	 * @old is a C++03 requirement, that C++11 removes.
	 */
	inline T *allocate(size_t n, const void *old=nullptr) 
	{
		daeCTC<sizeof(T)%sizeof(void*)==0>(); (void)old;
		while(A->_nexT+n>(T*)A->_end) A->_reserve(); 
		T *nextT = A->_nexT; A->_nexT+=n; return nextT;
	}
	/**TODO, DO-NOTHING
	 * Doesn't appear to call the destructor.
	 * @todo std::unordered_set/map probably use std::vector for their 
	 * buckets. If so they will dump their old copies of their vectors.
	 * Maybe that memory (only) could be recycled somehow; but how?
	 */
	inline void deallocate(T*, size_t/*n*/){ /*NOP*/ } 	
	
	daeStringAllocator():A(){}
	daeStringAllocator(Alligator &cp):A(&cp){}

	template<class U, int M>
	daeStringAllocator(daeStringAllocator<U,M> cp):A((Alligator*)cp.A){}
	template<class U, int M>
	inline bool operator==(daeStringAllocator<U,M> b)const{ return (void*)A==(void*)b.A; }
	template<class U, int M>
	inline bool operator!=(daeStringAllocator<U,M> b)const{ return (void*)A!=(void*)b.A; }

	//N must carry over for MSVC2010 uses rebind to arrive at its allocator_type???
	template<class U> struct rebind{ typedef daeStringAllocator<U,/*0*/N> other; };

COLLADA_(public) //C++11 requirements

	typedef T value_type;
							   
COLLADA_(public) //C++03 requirements

	typedef value_type &reference, *pointer;
	typedef const value_type &const_reference, *const_pointer;
	typedef size_t size_type; typedef ptrdiff_t difference_type;
	void construct(pointer p, value_type const &val){ new(p) value_type(val); }
	#ifdef __GNUC__
	//DEPRECATED IN C++17
	//I can't find a ... less way to do this :(
	template<class U, class... Args>
	//template< class U, class... Args > void construct(U* p, Args&&... args );
	void construct(U *p, Args&&...args){ new((void*)p) U(std::forward<Args>(args)...); }
	#endif
	void destroy(pointer p){ p->~value_type(); }
	size_type max_size()const{ return std::numeric_limits<size_type>::max()/sizeof(value_type); }
	pointer address(reference x)const{ return &x;}
	const_pointer address(const_reference x)const{ return &x; }
};

/**INTERNAL */
inline size_t daeStringHash(daeString str, size_t len)
{	
	//Variation on VS std::hash<> functional:
	//The string implementation doesn't seem to
	//care about char size, so words are used here.
	//ODDLY this implementation cannot be found today.
	//I don't know if it's free to use. It turns up with
	//VS2010, but a full 2013 search for hash shows nothing.
	//FYI: by "today" I mean to say, not-literally-yesterday?!
	size_t hash = 2166136261U;
	size_t last_word = 0, whole_words = len/sizeof(size_t);
	size_t rem = len-whole_words*sizeof(size_t); 
	while(rem>0) ((char*)&last_word)[rem--] = str[--len];	
	/*Assuming small strings. Can't say how small is small though.
	if(whole_words>10) 
	{
		size_t stride = 1+whole_words/10;
		for(size_t i=0;i<whole_words;i+=stride) hash = 16777619U*hash^((size_t*)str)[i];
	}
	else*/ for(size_t i=0;i<whole_words;i++) hash = 16777619U*hash^((size_t*)str)[i];
	return 16777619U*hash^last_word;
}
//This class was an internal detail, but it crept up all around the library.
/**-INTERNAL-
 * This class is used to avoid calling @c strlen when the extent is on hand.
 * Unfortunately, the @c size_t is stored in the key, where it's not needed.
 * http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3573.html seeks
 * to extend C++14's "is_transparent" via a void @c std::hash<>. Until then
 * there's no satisfactory workaround.
 * ON THE OTHER HAND
 * @c daeStringEqualFunctor IS ABLE TO USE @c memcmp() V. @c strcmp() OWING
 * TO THE EXTRA @c extent MEMBER. PERHAPS IT'S BETTER TO LEAVE IT LIKE THIS.
 * OH & 1 MORE THING
 * It also means it's technically possible to insert string-views: or where
 * there's not a 0-terminator. This is trecherous however, and not possible
 * unless @c COLLADA_DOM_UNORDERED_SET and @c COLLADA_DOM_UNORDERED_MAP are
 * respectively defined. (@c daeSmallStringTable solves this by preparing a
 * temporary copy, with a 0-terminator added onto the end.)
 */
class daeHashString : public daeString_support<daeHashString>
{	
COLLADA_(public)
	/**
	 * This could just be @c std::pair<daeString,size>, except
	 * that there must be a strong guarantee that @c string is
	 * the first member, as @c daeStringMap may or may not use
	 * this class. (If just @c std::set it uses @c daeString.)
	 *
	 * @c view is added so @c daeRefView can inherit from this.
	 */
	daeString string; size_t extent;

	/**OPERATOR
	 * Converts to @c string.
	 */
	inline operator daeString()const{ return string; } 
	
	/**WARNING, MICRO-OPTIMIZATION
	 * This constructs an empty string by using @c extent as a
	 * string that is 0 sized and 0 terminated.
	 * @warning It's safe to copy such a string, but the source
	 * string must remain in memory, as-is, as if it's a string
	 * literal. The library uses this to default client-strings.
	 * ("" could be used, but it might be an island in memory.)
	 */
	inline void _clear_client_string()
	{	
		extent = 0; string = (daeString)&extent;
	}

	template<class T>
	/**
	 * This is added to parser COLLADA's <keyword> field that's
	 * not an xs:list.
	 */
	inline daeHashString pop_first_word(const T &sentinels)
	{
		//Reminder: isspace does not return 1.
		for(;0!=extent&&0!=sentinels(*string);string++,extent--);
		daeHashString out = *this;
		for(;0!=extent&&0==sentinels(*string);string++,extent--);
		out.extent = string-out.string; 
		for(;0!=extent&&0!=sentinels(*string);string++,extent--);
		return out;
	}
	/**
	 * This is added to parser COLLADA's <keyword> field that's
	 * not an xs:list.
	 */
	inline daeHashString pop_first_word()
	{
		return pop_first_word(COLLADA_isspace);
	}	

COLLADA_(public) //Assignment operator

	template<class T>
	/**
	 * So is this (for @c daeValue and @c XS::Element.)
	 */
	inline daeHashString &operator=(const T &cp)
	{
		new(this) daeHashString(cp); return *this;
	}
	/**
	 * C++ requires a non-template assignment operator.
	 */
	inline daeHashString &operator=(const daeHashString &cp)
	{
		new(this) daeHashString(cp); return *this;
	}

COLLADA_(public) //Ambiguous constructors
	
	template<class T>
	/**
	 * Overloading can't pick up on the array forms.
	 */
	daeHashString(T &cp, typename DAEP::NoValue<T>::type SFINAE=0)
	{
		_init<T>::cp(*this,cp); (void)SFINAE;
	}
	//MSVC bug??
	template<class T>
	/**
	 * Overloading can't pick up on the array forms.
	 * @note Two of these are required to take part 
	 * in overload resolution. 
	 * @see @c daeElement::getChild() is an example.
	 */
	daeHashString(const T &cp, typename DAEP::NoValue<T>::type SFINAE=0)
	{
		_init<T>::cp(*this,cp); (void)SFINAE; 
	}
					
COLLADA_(public) //Non-copy constructors
	/**
	 * Default Non-Constructor
	 * This is for @c daeValue and @c XS::Element.
	 * NEW: Initializing @c extent=0 so to be safe seems harmless.
	 */
	daeHashString(){ /*NOP*/ extent = 0; }
	/**
	 * Constructor
	 */
	daeHashString(daeString a, size_t b){ string = a; extent = b; }	
	/**C++
	 * Non-Default Copy Constructor
	 */
	daeHashString(const daeHashString &cp){ string = cp.string; extent = cp.extent; }		

COLLADA_(public) //Standard Library compatibility layer	
	/**
	 * @c daeString_support requires this.
	 */
	inline size_t size()const{ return extent; }	
	/**
	 * This is practically unavoidable.
	 */
	inline bool empty()const{ return 0==extent; }
	/**
	 * @c daeString_support requires this.
	 */
	inline daeString data()const{ return string; }	

COLLADA_(private)
	/**
	 * This is used by @c daeHashString::daeHashString() because methods
	 * convert arrays (string-literals) into pointers and so cannot work.
	 */
	template<class T> struct _init
	{
		//REMINDER: THIS PULLS IN CLASSES DERIVED FROM
		//daeHashString ITSELF.
		template<class S> //T
		/**
		 * This is designed to fault on types that don't
		 * have express constructors...
		 * But might as well support classes like @c daeRefView
		 * that can't be included at the same time.
		 */
		static void cp(daeHashString &_this, const S &cp)
		{
			_maybe_nullptr((typename daeBoundaryString2<T>::type*)0,_this,cp);
		}
		template<class S>
		static void _maybe_nullptr(daeString*, daeHashString &_this, const S &cp)
		{
			class undefined *upcast = (T)0; (void)upcast;
			assert(cp==0); _this.string = nullptr; _this.extent = 0;
		}
		template<class S>
		static void _maybe_nullptr(daeHashString*, daeHashString &_this, const S &cp)
		{
			daeCTC<sizeof(daeStringCP)==sizeof(*cp.data())>();
			_this.string = (daeString)cp.data(); _this.extent = cp.size();
		}	
		
		/**
		 * C++98 @c std::map and @c std::set constructor
		 */
		static void cp(daeHashString &_this, const daeString &c_str)
		{
			_this.string = c_str; _this.extent = c_str==nullptr?0:strlen(c_str);
		}
		
		/**NON-CONST-VARIANT
		 * C++98 @c std::map and @c std::set constructor
		 */
		static void cp(daeHashString &_this, daeStringCP*const &c_str)
		{
			_this.string = c_str; _this.extent = c_str==nullptr?0:strlen(c_str);
		}
		
		template<class B, class C>
		/**
		 * @c std::string constructor.
		 */
		static void cp(daeHashString &_this, const std::basic_string<daeStringCP,B,C> &str)
		{
			_this.string = str.c_str(); _this.extent = str.size();
		}

		template<int> friend class daeURI_size;
		template<int N>
		/**
		 * This is preventing conversion from a @c daeURI. This is to force a
		 * user to choose @c daeURI_base::getURI_baseless() or the entire URI.
		 * Generally the relative form is the preferred representation of URI.
		 */
		static void cp(daeHashString &_this, const daeURI_size<N>&)
		{
			daeCTC<0>("getURI_baseless() or getURI()?");
		}
	}; 
	/*** CONT. BELOW. GCC/C++ WANT SPECIALIZATIONS IN THEIR NAMESPACE ****/
};
template<class T, int N> 
/**TEMPLATE-SPECIALIZATION
 * This is used by @c daeHashString::daeHashString() because methods
 * convert arrays (string-literals) into pointers and so cannot work.
 */
struct daeHashString::_init<T[N]>
{
	static void cp(daeHashString &_this, daeStringCP (&lit)[N])
	{
		_this.string = lit; //No const? Assume a buffer...
		_this.extent = strlen(lit); assert(_this.extent<N);
	}
	static void cp(daeHashString &_this, const daeStringCP (&lit)[N])
	{	
		_this.string = lit; _this.extent = N-1;
		//This catches users (and librarians) passing buffers.
		//Ideally compilers can eliminate it for string-literals.
		//Unfortunately there's no other way to tell literals apart.
		assert(lit[N-1]=='\0'&&strlen(lit)==N-1); daeCTC<(N>0)>();
	}			
};	
template<class T, int N> 
struct daeHashString::_init<T (&)[N]> : daeHashString::_init<T[N]>
{};
template<class T, int N> 
struct daeHashString::_init<const T[N]> : daeHashString::_init<T[N]>
{};
template<class T, int N> 
struct daeHashString::_init<const T (&)[N]> : daeHashString::_init<T[N]>
{};

/**INTERNAL */
struct daeStringEqualFunctor
{
	inline bool operator()(const daeHashString &a, const daeHashString &b)const
	{
		return a.extent==b.extent&&0==memcmp(a.string,b.string,a.extent);
	} 
};
/**INTERNAL */
struct daeStringLessFunctor
{
	inline bool operator()(daeString a, daeString b)const{ return strcmp(a,b)<0; } 
};
/**INTERNAL */
struct daeStringHashFunctor
{
	inline size_t operator()(const daeHashString &a)const{ return daeStringHash(a,a.extent); } 
};

#ifdef COLLADA_DOM_UNORDERED_SET
struct daeStringSet:daeStringAlligator<daeHashString,32>, //A
std::unordered_set<daeHashString,daeStringHashFunctor,
daeStringEqualFunctor,daeStringAllocator<daeHashString,32>>
{
	COLLADA_SUPPRESS_C(4355)
	daeStringSet():/*A,*/unordered_set(8/*MSVC2013*/,hasher(),key_equal(),*this){}
};
#elif defined(COLLADA_DOM_SET)
struct daeStringSet:daeStringAlligator<daeString,32>, //A
std::set<daeString,daeStringLessFunctor,daeStringAllocator<daeString,32>>
{
	COLLADA_SUPPRESS_C(4355)
	daeStringSet():/*A,*/set(key_compare(),*this){}
};
#endif //COLLADA_DOM_SET

#ifdef COLLADA_DOM_UNORDERED_MAP
template<class T, int N=64, class map=
std::unordered_map<daeHashString,T,daeStringHashFunctor,
daeStringEqualFunctor,daeStringAllocator<std::pair<const daeHashString,T>,N>>> 
struct daeStringMap:map::allocator_type::Alligator,map 
{
	COLLADA_SUPPRESS_C(4355)
	daeStringMap():/*A,*/map(8/*MSVC2013*/,typename map::hasher(),typename map::key_equal(),*this){}
};
template<class T, int N=64, class map=
std::unordered_multimap<daeHashString,T,daeStringHashFunctor,
daeStringEqualFunctor,daeStringAllocator<std::pair<const daeHashString,T>,N>>> 
struct daeStringMultiMap:map::allocator_type::Alligator,map
{
	COLLADA_SUPPRESS_C(4355)
	daeStringMultiMap():/*A,*/map(8/*MSVC2013*/,typename map::hasher(),typename map::key_equal(),*this){}
};
template<class T, int N=64, class map=
std::unordered_map<daeString/*Ref*/,T,std::hash<daeString>,
std::equal_to<daeString>,daeStringAllocator<std::pair<const daeString,T>,N>>>
struct daeStringRefMap:map::allocator_type::Alligator,map
{
	COLLADA_SUPPRESS_C(4355)
	daeStringRefMap():/*A,*/map(8/*MSVC2013*/,typename map::hasher(),typename map::key_equal(),*this){}
};
template<class T, int N=64, class map=
std::unordered_multimap<daeString/*Ref*/,T,std::hash<daeString>,
std::equal_to<daeString>,daeStringAllocator<std::pair<const daeString,T>,N>>>
struct daeStringRefMultiMap:map::allocator_type::Alligator,map
{
	COLLADA_SUPPRESS_C(4355)
	daeStringRefMultiMap():/*A,*/map(8/*MSVC2013*/,typename map::hasher(),typename map::key_equal(),*this){}
};
#elif defined(COLLADA_DOM_MAP)

template<class T, int N=64, class map=
std::map<daeString,T,daeStringLessFunctor,
daeStringAllocator<std::pair<const daeString,T>,N>>>
struct daeStringMap:map::allocator_type::Alligator,map
{
	COLLADA_SUPPRESS_C(4355)
	daeStringMap():/*A,*/map(typename map::key_compare(),*this){}
};
template<class T, int N=64, class map=
std::multimap<daeString,T,daeStringLessFunctor,
daeStringAllocator<std::pair<const daeString,T>,N>>>
struct daeStringMultiMap:map::allocator_type::Alligator,map
{
	COLLADA_SUPPRESS_C(4355)
	daeStringMultiMap():/*A,*/map(typename map::key_compare(),*this){}
};
template<class T, int N=64, class map=
std::map<daeString,T,std::less<daeString>,
daeStringAllocator<std::pair<const daeString,T>,N>>>
struct daeStringRefMap:map::allocator_type::Alligator,map
{
	COLLADA_SUPPRESS_C(4355)
	daeStringRefMap():/*A,*/map(typename map::key_compare(),*this){}
};
template<class T, int N=64, class map= 
std::multimap<daeString,T,std::less<daeString>,
daeStringAllocator<std::pair<const daeString,T>,N>>> 
struct daeStringRefMultiMap:map::allocator_type::Alligator,map
{
	COLLADA_SUPPRESS_C(4355)
	daeStringRefMultiMap():/*A,*/map(typename map::key_compare(),*this){}
};
#endif //COLLADA_DOM_MAP

#if defined(COLLADA_DOM_SET)\
 || defined(COLLADA_DOM_UNORDERED_SET)
template<class DBaseString=daeDBaseString, class A=daeStringAlligator<DBaseString>>
/**
 * Previously "daeStringTable."
 * The @c daeSmallStringTable is a simple string table class to hold a list of strings
 * without a lot of allocations.
 * 
 * Post-2.5 this class is tailored to @c daeDBaseString and @c daeStringRef. It's made
 * available so that implementations of @c daeDatabase can reuse it. It's been renamed
 * to highlight that it's intended to be used with small-strings only. How small is up
 * to the user/client.
 *
 * It's hardcoded to set the small strings' ref-counter to 2. This is so they will not
 * be released, unless the database implements a garbage-collector. Note, there's some
 * ambiguity here, as it could set the counters to 1, expecting callers will increment 
 * it. 2 is used, since it doesn't really matter. Garbage-collectors just need to know
 * that the baseline value is 2. The system-pool doesn't increment, so it saves cycles.
 *
 * Furthermore, the table appends the strings directly after the # fragment characters.
 * This means the user must be on the lookout for strings that begin with #, adjusting
 * the returned strings accordingly. This is a scheme to reuse same-document URLs that
 * refer to fragments by their "id" attributes. Whether this is a good idea or not, it
 * can always be disabled.
 */
class daeSmallStringTable : A
{
COLLADA_(public) //CONSTRUCTOR
	/**
	 * Constructor which specifies the initial buffer size, and maximum string size.
	 * @param pool is used to set @c daeDBaseString::_pool. 
	 * @param stringDBufferSize The size of the buffer to create for string allocation.
	 * The table cannot hold larger strings, including @c daeDBaseString::sizeofHeader().
	 * THIS IS AN IMPLEMENTATION DETAIL. THIS SHOULD BE MANY TIMES THE TARGET STRING SIZE.
	 */
	daeSmallStringTable(size_t pool, size_t stringDBufferSize=1024/*1024*1024*/)
	:A(stringDBufferSize)
	,_pool(pool)
	#ifdef _DEBUG
	,_small(stringDBufferSize)
	#endif
	{}

COLLADA_(public) //INTERFACE
	/**LEGACY
	 * Allocates a string from the table.
	 * @param string @c daeString to copy into the table.
	 * @return Returns an allocated string.
	 */
	inline daeString allocString(daeString string, size_t length)
	{
		#ifndef COLLADA_DOM_UNORDERED_SET
		if(string[length]!='\0') return _allocString2(string,length);
		#endif
		std::pair<_it,bool> ins = _set.insert(daeHashString(string,length));
		if(ins.second) 
		(daeString&)*ins.first = _assign(string,length); 
		return *ins.first;
	}
	#ifndef COLLADA_DOM_UNORDERED_SET
	/**
	 * This is to support truncated inputs, where @c std::set is used
	 * with @c daeStringLessFunctor. It copies the string, adding the
	 * 0 terminator, that is not required by @c daeStringEqualFunctor.
	 */
	inline daeString _allocString2(daeString string, size_t length)
	{
		assert(string[length]!='\0');
		daeArray<daeStringCP,260> tmp; tmp.grow(length+1);
		((daeString)memcpy(tmp.data(),string,length*sizeof(daeStringCP)))[length] = '\0';
		return allocString(tmp.data(),length);
	}
	#endif

	/**LEGACY-SUPPORT
	 * Previously "clear."
	 * Clears the storage. Sort of.
	 */
	inline void reset(size_t stringDBufferSize=1024)
	{
		this->~daeSmallStringTable(); daeSmallStringTable(_pool,stringDBufferSize);
	}

COLLADA_(private) //UTILITIES

	COLLADA_NOINLINE
	/** 
	 * Sets up a new @c daeDBaseString object. 
	 */
	inline daeString _assign(daeString str, size_t len)
	{
		assert(len<_small);
		if(*A::_nexT+len>=A::_end) A::_reserve();
		DBaseString &out = *new(A::_nexT)DBaseString(_pool,2,str,len);		
		A::_nexT = (DBaseString*)out.next_pointer_boundary(); 
		return out+1;
	}

COLLADA_(private) //DATA-MEMBERS

	size_t _pool;
	daeStringSet _set;
	#ifdef _DEBUG 
	/** Max size of the table's strings. */
	size_t _small;
	#else
	enum{ _small=INT_MAX };
	#endif
	typedef daeStringSet::iterator _it;
};
#endif //COLLADA_DOM_SET || COLLADA_DOM_UNORDERED_SET

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_STRING_REF_H__
/*C1071*/
