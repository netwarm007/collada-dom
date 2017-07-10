/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef __COLLADA_DOM__DAE_PLATONIC_H__
#define __COLLADA_DOM__DAE_PLATONIC_H__

#include "daeStringRef.h"

COLLADA_(namespace)
{//-.
//<-'

/**WORKAROUND
 * This is to get around GCC/C++ explicit specialization
 * strictures.
 */
struct daePlatonic_base
{
	enum Op //__Thunk__v1__method2 operations
	{
		/** __COLLADA__atomize */
		atomize,
		/** __COLLADA__locate */
		locate,
	};

COLLADA_(protected)
	/**
	 * To be continued...
	 */
	template<class S> struct _unwrap;
};
template<class S>
struct daePlatonic_base::_unwrap
{
	typedef S type,  *ptr; static S *s(ptr p){ return  p; }
};
template<class S>
struct daePlatonic_base::_unwrap<S*>
{
	typedef S type, **ptr; static S *s(ptr p){ return *p; }
};
template<class S>
struct daePlatonic_base::_unwrap<daeSmartRef<S>>
{
	typedef S type, **ptr; static S *s(ptr p){ return *p; }
};
template<class S>
struct daePlatonic_base::_unwrap<daeSmartRef<const S>> //???
{
	typedef S type, **ptr; static S *s(ptr p){ return *p; }
};
template<class T>
/**
 * This class bridges @c daeAllocThunk and @c daeFeature.
 * It's a marriage of convenience.
 * @see daeAllocThunk::__Thunk__v1__method2().
 *
 * @remarks This class is for internal use. Although it's
 * to be studied to see how @c __COLLADA__atomize is done.
 * @c daePlatonic is named after this header, and not the
 * other way around. Its prominence is due to its @c enum.
 */
class daePlatonic : public daePlatonic_base
{
	friend class daeFeature;
	/** Friend all so MSVC can compile sizeof. */
	template<class,int> friend class daeAlloc;

COLLADA_(public)

	const daeAllocThunk *thunk;
	union{ void *feature; typename _unwrap<T>::ptr pointer; }; 

	/** 
	 * Takes the args from @c __Thunk__v1__method2(). 
	 * It's easier to work this way. It can be eliminated.
	 * @param f can be @c nullptr because of @c _feature_test().
	 */
	daePlatonic(void *f, const daeAllocThunk *t)
	:thunk(t),feature(f){}
	/**
	 * NEW: This makes it easy to perform feature tests, at
	 * the expense of checking @c nullptr. No big deal really.
	 */
	bool _feature_test(){ return nullptr==feature; }
	/**	 
	 * This was setup to provide services to @c daeFeature.
	 * It was the constructor, but it's not always required.
	 */
	int _setup_pointer_and_get_counter_for_feature_operation()
	{			
		if(_feature_test()) return 0;

		if(0!=thunk->_offset) //Indicates an array feature thunk.
		{
			//A custom-offset is needed, so this is an array.
			pointer = (typename _unwrap<T>::ptr)((daeArray<T>*)feature)->data();
			return (int)((daeArray<T>*)feature)->getCount();
		}
		return 1; //Indicates a standalone T.		
	}
	
	template<class TT, TT> class PtoM{};

	/** 
	 * @return Returns @c DAE_ERR_NOT_IMPLEMENTED where
	 * @a ENUM is a no-op. 
	 * The thunk itself can be passed to do a test only.
	 * The @c daeFeature flags save the result of tests.
	 * @note Pass a @c nullptr to perform feature tests.
	 * The ambiguity between arrays and non-arrays made
	 * feature tests too much to set up/easy to mess up.
	 *
	 * Classes must "typedef void __COLLADA__atomize" 
	 * or define a "void __COLLADA__atomize()" method.
	 */
	daeError maybe_atomize()
	{
		//_atomize_recursive adds support for multidimensional
		//arrays. 
		//return _atomize<_unwrap<T>::type>(nullptr);
		return _atomize_recursive((T*)nullptr); 
	}
	template<typename S> daeError _atomize(...) //permitted by non-classes
	{
		//MSVC had let the "using" keyword be used to inherit __COLLADA_atomize
		//but GCC wanted to a full wrapper, which is too much work.
		//This allows inheritance without any intervention.
		//daeCTC<daeArrayAware<S>::is_plain>();
		if(!daeArrayAware<S>::is_plain)
		_atomize<S>(daeFig<daeArrayAware<S>::is_plain>()); return DAE_ERR_NOT_IMPLEMENTED;
	}
	template<typename S> daeError _atomize(typename S::__COLLADA__atomize *voidptr)
	{
		voidptr = (class undefined*)nullptr; return DAE_ERR_NOT_IMPLEMENTED;
	}		
	//This change lets __COLLADA__atomize be inherited and still be detectable.
	//template<typename S> daeError _atomize(PtoM<void(S::*)(),&S::__COLLADA__atomize>*)
	template<typename S> daeError _atomize(daeFig<0>)
	{
		int counter = _setup_pointer_and_get_counter_for_feature_operation();
		for(S*s;counter-->0;)
		if((s=_unwrap<T>::s(pointer+counter))!=nullptr) s->__COLLADA__atomize(); return DAE_OK;
	}
	template<class SS, int N> daeError _atomize_recursive(daeArray<SS,N>*)
	{	
		//Note. a temporary thunk is used both on the off chance (it should
		//not be possible) that the _offset is not set, and to give compilers
		//enough to be sure it's safe to eliminate DAE_ERR_NOT_IMPLEMENTED code.
		daeAlloc<SS,0> tmp; ((daeAllocThunk*)tmp)->_offset = !0;
		if(!tmp._feature_test(atomize)) return DAE_ERR_NOT_IMPLEMENTED;
		int counter = _setup_pointer_and_get_counter_for_feature_operation();		
		while(counter-->0) tmp.__Thunk__v1__method2(pointer+counter,atomize); return DAE_OK;
	}	
	/**
	 * This was set up to implement list_of_hex_binary as a 2-D char-array.
	 * Probably hexBinary won't ultimately be implemented that way, but it's
	 * still possible to have multi-dimensional arrays. If the underlying type
	 * doesn't require @c __COLLADA__atomize() then an N-D array doesn't have to
	 * be atomized.
	 * @remarks @c daeArray can't implement a branching @c __COLLADA__atomize() if
	 * it doesn't return a @c daeError code. It's better/safer to implement it here.
	 */
	daeError _atomize_recursive(...){ return _atomize<typename _unwrap<T>::type>(nullptr); }

	/** 
	 * Facilitates @c maybe<locate>().
	 * @c locate does cross-module virtual-method-table 
	 * ownership via @c daeArrayAware<T>::place() vis-a-vis __COLLADA__locate().
	 *
	 * Class T must define a "void __COLLADA__locate(const T&)"
	 * or "void __COLLADA__locate(const T::__COLLADA__Object*, const T&)" method.
	 */
	daeError maybe_locate(va_list va)
	{
		return _locate<T>(va,nullptr); 
	}
	template<typename S> daeError _locate(...){ return DAE_ERR_NOT_IMPLEMENTED; }	
	template<typename S> daeError _locate(va_list va, PtoM<void(S::*)(const S&),&S::__COLLADA__locate>*)
	{
		//Reminder: This cannot be extracted inside the __COLLADA__locate args list.
		const T &cp = *va_arg(va,const T*);
		if(!_feature_test()) ((T*)feature)->__COLLADA__locate(cp); return DAE_OK;
	}		
	template<typename S> daeError _locate(va_list va, 
	PtoM<void(S::*)(typename S::__COLLADA__Object*, const S&),&S::__COLLADA__locate>*)
	{
		//Reminder: These cannot be extracted inside the __COLLADA__locate args list.
		typename T::__COLLADA__Object *obj = va_arg(va,typename S::__COLLADA__Object*);
		const T &cp = *va_arg(va,const T*);
		if(!_feature_test()) ((T*)feature)->__COLLADA__locate(obj,cp); return DAE_OK;
	}		
};

/**
 * COLLADA C++ Object member RTTI class.
 *
 * @c daeFeature is conceptualized a feature of a @c daeModel.
 * A  model is a Platonic object; features describe the model.
 *
 * @remarks This class was added to generalize the meta model
 * to 2.5's generic object-model. There are 2 very real goals.
 * 1) Memory ownership semantics.
 * 2) Ref-counter conflation, and cycles: circular references.
 * Secondary goals are:
 * - Runtime inspection.
 * - Module identification.
 * - Structural annotation.
 */
class daeFeature
{	
COLLADA_(private) 

	friend class daeModel;
	friend class XS::Schema;
	friend class daeMetaObject;	
	friend class daeProcessShare_base;

	daeOffset _offset;
			 
	daeAllocThunk *_localthunk;
	daeTypewriter *_typewriter;
	/**ALIGNED
	 */
	struct Flags
	{
	unsigned 
	feature_1:1,
	subobject:1,
	atomizing:1,
	unionized:1,
	new_thunk:1;		
	operator bool()const{ return (unsigned&)*this!=0; }
	}_flags; 
	unsigned _RESERVED_; //Padding on 64 bit systems.
	
	//This should be added sometime.
	//It should be a name, followed by whitespace, and words.
	//daeClientString _name_explanation;

COLLADA_(public) //OPERATORS

	//It's becoming standard to access a daeTypewriter this way. 
	inline operator daeTypewriter*()const{ return _typewriter; }
	inline operator daeTypewriter&()const{ return *_typewriter; }
	inline daeTypewriter *operator->()const{ return _typewriter; }	

COLLADA_(public) //MODEL SETUP
	/**OBSOLETE?
	 * Prefer @c daeModel::addFeature().
	 * (This is provided for completeness.)
	 * Sets up the @c sizeof(char) offset from the top of the
	 * object's @c this pointer.
	 */
	inline daeFeature &setOffset(daeOffset os){ _offset = os; return *this; }

	/**
	 * Sets up the process-share local thunk.
	 * This must be set up, even if the feature is not an array.
	 * And if @c this feature is an array, then
	 * @c lt->_offset must be nonzero. Don't be intimidated by its _.
	 * (There just doesn't seem to be a better way to set up thunks.)
	 */
	inline daeFeature &setAllocThunk(const daeAllocThunk *lt)
	{
		//Call deleteAllocThunk() to prevent this assert from triggering.
		assert(_localthunk==nullptr||_localthunk==lt);
		_flags.atomizing = lt->_feature_test(daePlatonic<>::atomize)?1:0;
		_localthunk = const_cast<daeAllocThunk*>(lt); return *this; 
	}
	/**HELPER @c daeAlloc<T,0> protects its thunk aggressively. */
	inline daeFeature &setAllocThunk_offset(daeAllocThunk *lt)
	{
		lt->_offset = _offset; return setAllocThunk(lt);
	}
	static daeAlloc<daeStringCP,0> thunk;
	/** Does @c daeAlloc::deleteThis() if @c getFlags().new_thunk. */
	inline daeFeature &deleteAllocThunk()
	{
		if(1==_flags.new_thunk)
		_localthunk->__Thunk__v1__method(0); _localthunk = nullptr; return *this;
	}

	/**
	 * Sets up the @c daeTypewriter. It doesn't have to work, but
	 * it should not be @c nullptr for release builds, and should
	 * generate an error if it doesn't work.
	 */
	inline daeFeature &setTypewriter(daeTypewriter *tw){ _typewriter = tw; return *this; }

COLLADA_(public) //CONST-ONLY ACCESSORS
	/**
	 * Gets the data member @c char offset.
	 */
	inline daeOffset getOffset()const{ return _offset; }

	/**
	 * Gets the various flags bitfield object.
	 */
	inline const Flags &getFlags()const{ return _flags; }

	/**
	 * Typewriters are not strictly required, as
	 * thunks are. If absent, consider the feature
	 * opaque. The owning object knows what it's for.	  
	 */
	inline bool isVariant()const{ return nullptr==_typewriter; }
	/**	 
	 * @see operator->(). THIS CAN BE @c nullptr. 
	 */
	inline daeTypewriter *getTypewriter()const{ return _typewriter; }

	/**
	 * Gets the allocation-unit thunk for this feature.
	 * All features are basically required to have a thunk.
	 * If @c _offset is an "atom" @c getAllocThunk()._offset==0.
	 * If @c _offset is an array @c getAllocThunk()._offset==_offset.
	 */
	inline const daeAllocThunk &getAllocThunk()const{ return *_localthunk; }

	/**
	 * This can be used to iterate over features (from last to first) and
	 * can even be used to locate the model, although it's not recommended.
	 */
	inline bool isFirstFeature()const{ return 1==_flags.feature_1; }	

	/**
	 * Tells if this feature can be a @c daeArray.
	 * Note: "can" means it can be a "variant" field. But if so,
	 * -it cannot house sub-objects or trigger @c __COLLADA__atomize().
	 */
	inline bool isArrayCompatible()const{ return 0!=_localthunk->_offset; }	
																	
#ifdef BUILDING_COLLADA_DOM

COLLADA_(public) //INVISIBLE (protecting "obj")
	/**
	 * Access this feature WRT @a obj.
	 */
	inline daeOpaque getWRT(DAEP::Object *obj)const
	{
		return daeOpaque(obj)[_offset]; 
	}	
	/**CONST-FORM
	 * Access this feature WRT @a obj.
	 */
	inline const daeOpaque getWRT(const DAEP::Object *obj)const
	{
		return daeOpaque(obj)[_offset]; 
	}

	/**
	 * Access subobject feature WRT @a obj.
	 */
	inline daeObject &getKnownObjectWRT(DAEP::Object *obj)const
	{
		assert(1==_flags.subobject); return getWRT(obj); 
	}
	/**CONST-FORM
	 * Access subobject feature WRT @a obj.
	 */
	inline const daeObject &getKnownObjectWRT(const DAEP::Object *obj)const
	{
		assert(1==_flags.subobject); return getWRT(obj); 
	}

	/**
	 * Corresponds to __COLLADA__atomize().
	 */
	inline void atomizeWRT(DAEP::Object *obj)const
	{
		if(_flags.atomizing!=0)
		_localthunk->__Thunk__v1__method2(&getWRT(obj),daePlatonic<>::atomize);
	}

	template<class LAZY>
	/**
	 * Gets the subordinated-flag's state.
	 * @param obj is ignored by release-builds.
	 * @return Returns @c true if this feature is a sub-object.
	 * That is, an object embedded directly in @a obj object.	 
	 */
	inline bool isEmbeddedObjectWRT(const LAZY *obj)const
	{
		const LAZY &so = getWRT(obj);
		assert(0==_flags.subobject||so.__DAEP__Object__refs<-1000);
		return 1==_flags.subobject;
	}

	template<class LAZY>
	/**RECURSIVE, SUBOPTIMAL
	 * @return Returns @c true if [sub]subordinates have refs.
	 */
	inline bool hasEmbeddedRefsWRT(const LAZY *obj)const
	{
		const LAZY &so = getKnownObjectWRT(obj);
		assert(so.__DAEP__Object__refs<-1000);
		if(so.__DAEP__Object__refs>-so.__DAEP__Object__refs_embedding_threshold)
		return true;
		const COLLADA_INCOMPLETE(LAZY) daeModel &model = so.__DAEP__Object__v1__model();		
		return model.hasEmbeddedRefsWRT(&so);
	}

#endif //BUILDING_COLLADA_DOM
};

#include "../LINKAGE.HPP" //#define LINKAGE

extern void daeMetaElement_self_destruct(daeMetaObject*);
/**
 * COLLADA C++ Object metadata class. The element-object metadata class is
 * based on this class: @c daeMetaObject.
 *
 * This class MUST BE a @c DAEP::Make pointer-pointer. It fulfills this by
 * a @c daeProcessShare* being its first member, and being standard-layout.
 */
class daeMetaObject
{
	friend class daeModel;
	friend class XS::Schema;	
	friend class daeProcessShare;
	friend class daeProcessShare_base;

COLLADA_(protected) //VISIBLE DATA-MEMBERS

	friend class daeDocument;
	union //COLLADA__VIZDEBUG
	{
	/**	 
	 * This implements @c DAEP::Make. This class is a
	 * "pointer-pointer" to a DAEP Make, which these are.
	 *
	 * @note This must be the first member of this class.
	 */	
	daeProcessShare *_processShare; const XS::Schema *_schema;
	};

	/**
	 * This is the type-name for this object. For XML-like elements
	 * this is the global or local type-name from the schema, which
	 * is sometimes, but not always, the name used by the element's
	 * tag.
	 */
	daeName _name;

	/**
	 * This is the @c sizeof size of this metadata's object class. 
	 *
	 * @note The objects themselves can be variable-length, and or
	 * have some of their data emebedded inside of them, and after
	 * all of this, databases may store data before or after their
	 * @c this pointer.
	 */
	size_t _sizeof;

	/**
	 * Negative value, marking the heighest @c daeFeatureID.
	 * @c -_rendFeatureID is equal to their number.
	 */
	daeFeatureID _finalFeatureID;

	/**SCHEDULED FOR REMOVAL?
	 * "(char*)this-_sizeof" is normally the model, except for how
	 * @c domAny::_meta works. 
	 */
	const daeModel *_domAny_safe_model;

	/**
	 * @c daeProcessShare uses this to delete its models. It holds
	 * the first pointer in a linked-list. If this is not @c nullptr
	 * and hince the end, then it is a subsequent pointer in the list.
	 * @see @c daeProcessShare::~daeProcessShare().
	 */
	daeMetaObject *_deleteList;
	/**
	 * @c _delete() calls @c _self_destruct() if it's not @c nullptr.
	 * The "daeMeta-" classes don't have virtual-destructors, as they
	 * are embedded inside of the "model" and it would just complicate
	 * things.
	 * This is @c daeMetaElement_self_destruct(). Although the design is
	 * such that it can be open-ended.
	 */
	void (*_destructor_ptr)(daeMetaObject *self_destruct);
	/**RECURSIVE (TRIVIALLY-SO)
	 * @c daeProcessShare calls @c _delete() for the first meta-object
	 * in its list. It recursively deletes every object in a linked-list.
	 * The "_self_delete" scenario is not considered. It's a singly-linked
	 * list.
	 */
	inline void _destruct()
	{	
		//Here daeMetaElement_self_destruct() is fast-tracked.
		#ifdef BUILDING_COLLADA_DOM		
		if(_destructor_ptr==daeMetaElement_self_destruct)
		daeMetaElement_self_destruct(this);
		else
		#endif //BUILDING_COLLADA_DOM
		if(_destructor_ptr!=nullptr) _destructor_ptr(this);
	}

COLLADA_(protected) //INACCESSIBLE
	/**
	 * Disabled Destructor
	 * @c _self_destruct() should be used instead.
	 */
	~daeMetaObject(){ assert(_domAny_safe_model!=nullptr); }

COLLADA_(public) //PUBLIC OPERATORS

	COLLADA_DOM_OBJECT_OPERATORS(daeMetaObject)

	/** @return Returns @c true if @a cmp is @c this metadata. */
	inline bool operator==(const daeMetaObject &cmp)const{ return this==&cmp; }
	/** @return Returns @c false if @a cmp is @c this metadata. */
	inline bool operator!=(const daeMetaObject &cmp)const{ return this!=&cmp; }

COLLADA_(public) //PUBLIC ACCESSORS
	/**
	 * 2.5: holds schema namespace and "atomic" types.
	 */
	inline daeProcessShare &getProcessShare()const{ return *_processShare; }
	
	/**
	 * Gets at the @c daeModel.
	 */
	inline daeModel &getModel()
	{
		//This would be unsafe as long as domAny::_meta exists.
		//return (daeModel&)daeOpaque(this)[-daeOffset(_sizeof)]; 
		return (daeModel&)*_domAny_safe_model; 		
	}
	/**CONST-FORM
	 * Gets at the @c daeModel (as DAEP Model. See remarks.)
	 * @remarks This returns @c DAEP::Model because @c daeModel
	 * cannot convert to a @c DAEP::Model since they are the same
	 * type (DAEP Model is const-agnostic.) The reverse is not true.
	 * It doesn't work to do "getModel().x." But it's rarely useful to
	 * do that. (dae(getModel()).x is one option.)
	 */
	inline DAEP::Model &getModel()const
	{
		//This would be unsafe as long as domAny::_meta exists.
		//return (DAEP::Model&)daeOpaque(this)[-daeOffset(_sizeof)]; 
		return (DAEP::Model&)*_domAny_safe_model;
	}
	
	/**
	 * Gets the name of this element type.
	 * @return Returns the name of this element type.
	 */
	inline const daePseudonym &getName()const{ return _name; }

	/**
	 * Gets the size in chars of each instance of this object type.
	 * Used for factory element creation.
	 * @return Returns the number of chars for each C++ object instance.
	 */
	inline size_t getSize()const{ return _sizeof; }
		
	/**EXPERIMENTAL
	 * Gets the negated @c daeFeature count for @c getModel().
	 */
	inline daeFeatureID rbeginFeatureID()const{ return _finalFeatureID; }

	/**EXPERIMENTAL
	 * Gets the @c daeFeature count for @c getModel().
	 * This negates the internal value.
	 * @see getFinalFeatureID(). 
	 */
	inline size_t getFeatureCount()const{ return -_finalFeatureID; }
	
	/**EXPERIMENTAL
	 * Tells if there are sub-objects present. 
	 */
	NOALIAS_LINKAGE bool hasObjectsEmbedded()const
	SNIPPET( return 0!=_subobjectsMask; )
	
	/**EXPERIMENTAL
	 * Tells if at least one feature defines @c __COLLLADA__atomize().
	 * (Most should not. This avoids looping over the features to see if so.)
	 */
	NOALIAS_LINKAGE bool hasFeaturesWithAtomize()const
	SNIPPET( return 0!=_features_atomize; )
	
#ifdef BUILDING_COLLADA_DOM

COLLADA_(protected)

	friend class daeModel;
	/**
	 * This iterates over the features to combine the information from
	 * @c addFeature() into the gestalt members of this data structure.
	 */
	void _feature_complete(daeFeatureID);
	/** Implements _feature_complete(). */
	inline void _feature_complete()const
	{
		const_cast<daeMetaObject*>(this)->_feature_complete(_finalFeatureID); 
	}

COLLADA_(protected) //INVISIBLE	

	/**ALIGNED
	 * Bitmask corresponding to @c daeFeature::_flags.subobject
	 * for the first 31 features. This establishes a firm limit.
	 */
	int _subobjectsMask;
	/**ALIGNED, BIT-FIELD
	 * Bitwise flags/fields reserved for use by elements.
	 */
	unsigned _features_atomize:1;
	/**ALIGNED, BIT-FIELD-CONTINUED
	 * Signals @c daeFeature::deleteAllocThunk() is required.
	 * This is a messy detail. Internally @c _destructor_ptr is
	 * set if not @c nullptr. Otherwise this flag is all there is.
	 * NOTE: THE FLAG IS ONLY VISIBLE IF THE LIBRARY ISN'T IMPORTED.
	 */
	unsigned _deleteAllocThunk:1;

#endif //BUILDING_COLLADA_DOM
};

#include "../LINKAGE.HPP" //#undef LINKAGE

//---.
}//<-'

#endif //__COLLADA_DOM__DAE_PLATONIC_H__
/*C1071*/
